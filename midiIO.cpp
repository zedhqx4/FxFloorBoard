/****************************************************************************
**
** Copyright (C) 2007~2010 Colin Willcocks.
** Copyright (C) 2005~2007 Uco Mesdag.
** All rights reserved.
** This file is part of "GT-10 Fx FloorBoard".
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
****************************************************************************/

#include <QSysInfo>
#include <QMutexLocker>
#include "midiIO.h"
#include "SysxIO.h"
#include "globalVariables.h"
#ifdef Q_OS_LINUX
#include "third_party/rtmidi-6.1.0/RtMidi.h"
typedef rt::midi::RtMidiIn MidiBackendIn;
typedef rt::midi::RtMidiOut MidiBackendOut;
typedef rt::midi::RtMidiError MidiBackendError;
#else
#include "RtMidi.h"
#include "RtError.h"
typedef RtMidiIn MidiBackendIn;
typedef RtMidiOut MidiBackendOut;
typedef RtError MidiBackendError;
#endif

#ifdef Q_OS_LINUX
static void midiBackendErrorCallback(rt::midi::RtMidiError::Type type, const std::string &errorText, void *userData)
{
    Q_UNUSED(type);
    midiIO *owner = static_cast<midiIO *>(userData);
    if (owner)
        owner->reportMidiBackendError(QString::fromStdString(errorText));
}
#endif

static MidiBackendIn *createMidiIn(const std::string &clientName, midiIO *owner)
{
#ifdef Q_OS_LINUX
    MidiBackendIn *midiin = new rt::midi::RtMidiIn(rt::midi::RtMidi::LINUX_ALSA, clientName);
    midiin->setErrorCallback(&midiBackendErrorCallback, owner);
    return midiin;
#else
    Q_UNUSED(owner);
    return new MidiBackendIn(clientName);
#endif
}

static MidiBackendOut *createMidiOut(const std::string &clientName, midiIO *owner)
{
#ifdef Q_OS_LINUX
    MidiBackendOut *midiout = new rt::midi::RtMidiOut(rt::midi::RtMidi::LINUX_ALSA, clientName);
    midiout->setErrorCallback(&midiBackendErrorCallback, owner);
    return midiout;
#else
    Q_UNUSED(owner);
    return new MidiBackendOut(clientName);
#endif
}

#include "Preferences.h"
#include "MidiTable.h"


// Platform-dependent sleep routines.
#ifdef Q_OS_WIN
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds )
#else // Unix variants & Mac
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif

midiIO::midiIO()
{
        this->midi = false;
        this->msgType = "name";
        this->loopCount = 0; // Set this to false until required;
#ifdef Q_OS_LINUX
        this->midiBackendErrorOccurred.store(false);
#endif
        /* Connect signals */
        SysxIO *sysxIO = SysxIO::Instance();
        QObject::connect(this, SIGNAL(setStatusSymbol(int)), sysxIO, SIGNAL(setStatusSymbol(int)));
        QObject::connect(this, SIGNAL(setStatusProgress(int)), sysxIO, SIGNAL(setStatusProgress(int)));
        QObject::connect(this, SIGNAL(setStatusMessage(QString)), sysxIO, SIGNAL(setStatusMessage(QString)));
        QObject::connect(this, SIGNAL(setStatusdBugMessage(QString)), sysxIO, SIGNAL(setStatusdBugMessage(QString)));
        QObject::connect(this, SIGNAL(errorSignal(QString, QString)), sysxIO, SLOT(errorSignal(QString, QString)));
        QObject::connect(this, SIGNAL(replyMsg(QString)),	sysxIO, SLOT(receiveSysx(QString)));
        QObject::connect(this, SIGNAL(midiFinished()), sysxIO, SLOT(finishedSending()));
};
/*********************** queryMidiOutDevices() *****************************
 * Retrieves all MIDI Out devices installed on your system and stores them
 * as a QList of QStrings and device id's.
 *************************************************************************/
void midiIO::queryMidiOutDevices()
{
    this->midiOutDevices.clear();
         MidiBackendOut *midiout = 0;
         std::string portName;
         unsigned int outPorts;
         const std::string clientName = "FxFloorBoard";
  try { midiout = createMidiOut(clientName, this); }   /* MidiBackendOut constructor */
  catch (MidiBackendError &error) {
    error.printMessage();
    emit errorSignal(tr("Midi Output Error"), tr("port error"));
    goto cleanup; };
  try { outPorts = midiout->getPortCount(); }
  catch (MidiBackendError &error) {
    error.printMessage();
    emit errorSignal(tr("Midi Output Error"), tr("enumeration error"));
    this->midiOutDevices.push_back(tr("no midi device available"));
    goto cleanup;
  };
  for ( unsigned int i=0; i<outPorts; i++ ) {
    try {
      portName = midiout->getPortName(i);
        }
    catch (MidiBackendError &error) {
      error.printMessage();
      emit errorSignal(tr("Midi Output Error"), tr("data error"));
      goto cleanup; };
#ifdef Q_OS_WIN
        /* if we are running msdos based windows, use device numbers instead, UNICODE not supported*/
    if(QSysInfo::WindowsVersion <= QSysInfo::WV_Me)
        {
                char dev = char(i+49); // point to character set numerals
                QString outstring = tr("un-named midi device: ");
                outstring.append(dev);
                this->midiOutDevices.append(outstring);
        }
        else { this->midiOutDevices.append(QString::fromStdString(portName)); };
#else
        this->midiOutDevices.append(QString::fromStdString(portName));
#endif
  };
  if (outPorts < 1) { this->midiOutDevices.push_back(tr("no midi device available")); };
 /* Clean up */
 cleanup:
  delete midiout;
};

QList<QString> midiIO::getMidiOutDevices()
{
        queryMidiOutDevices();
        return this->midiOutDevices;
};

/*********************** queryMidiInDevices() ******************************
 * Retrieves all MIDI In devices installed on your system and stores them
 * as a QList of QStrings and device id's.
 *************************************************************************/
void midiIO::queryMidiInDevices()
{
    this->midiInDevices.clear();
         MidiBackendIn *midiin = 0;
         std::string portName;
         unsigned int inPorts;
         const std::string clientName = "FxFloorBoard";
  try { midiin = createMidiIn(clientName, this); }    /* MidiBackendIn constructor */
  catch (MidiBackendError &error) {
    error.printMessage();
    emit errorSignal(tr("Midi Input Error"), tr("port error"));
    goto cleanup; };
  try { inPorts = midiin->getPortCount(); }
  catch (MidiBackendError &error) {
    error.printMessage();
    emit errorSignal(tr("Midi Input Error"), tr("enumeration error"));
    this->midiInDevices.push_back(tr("no midi device available"));
    goto cleanup;
  };
  for ( unsigned int i=0; i<inPorts; i++ ) {
     try { portName = midiin->getPortName(i); }
     catch (MidiBackendError &error) {
        error.printMessage();
        emit errorSignal(tr("Midi Input Error"), tr("data error"));
        goto cleanup; };
#ifdef Q_OS_WIN
  if(QSysInfo::WindowsVersion <= QSysInfo::WV_Me)
        {
                char dev = char(i+49); // point to character set numerals
                QString instring = tr("un-named midi device: ");
                instring.append(dev);
                this->midiInDevices.append(instring);
        }
  else { this->midiInDevices.append(QString::fromStdString(portName)); };
#else
        this->midiInDevices.append(QString::fromStdString(portName));
#endif
  };
  if (inPorts < 1)
        { this->midiInDevices.push_back(tr("no midi device available")); };
 // Clean up
 cleanup:
  delete midiin;
};

QList<QString> midiIO::getMidiInDevices()
{
        queryMidiInDevices();
        return this->midiInDevices;
};

/*********************** sendMsg() **********************************
 * Prepares the sysx message before sending on the MIDI Out device. It
 * converts the message from a QString to a std::vector and opens, sends
 * and closes the MIDI device.
 *************************************************************************/
void midiIO::sendSyxMsg(QString sysxOutMsg, int midiOutPort)
{
    MidiBackendOut *midiMsgOut = 0;
    bool portOpened = false;
    const std::string clientName = "FxFloorBoard";
    try
    {
        midiMsgOut = createMidiOut(clientName, this);
        if (midiMsgOut->getPortCount() < 1)
        {
            emit errorSignal(tr("Syx Output Error"), tr("no MIDI output device available"));
            goto cleanup;
        }
        if ((sysxOutMsg.size() % 2) != 0)
        {
            emit errorSignal(tr("Syx Output Error"), tr("invalid hexadecimal message length"));
            goto cleanup;
        }
        midiMsgOut->openPort(static_cast<unsigned int>(midiOutPort), clientName);
        portOpened = true;

        std::vector<unsigned char> message;
        message.reserve(256);
        int bytesSinceLastFrame = 0;
        const int totalBytes = sysxOutMsg.size() / 2;
        for (int i = 0; i < totalBytes; ++i)
        {
            bool ok = false;
            const unsigned int value = sysxOutMsg.mid(i * 2, 2).toUInt(&ok, 16);
            if (!ok || value > 0xFF)
            {
                emit errorSignal(tr("Syx Output Error"), tr("invalid hexadecimal byte in SysEx message"));
                goto cleanup;
            }
            if (message.empty() && value != 0xF0)
            {
                emit errorSignal(tr("Syx Output Error"), tr("SysEx frame does not start with F0"));
                goto cleanup;
            }
            message.push_back(static_cast<unsigned char>(value));
            ++bytesSinceLastFrame;
            emit setStatusProgress((i + 1) * 100 / (totalBytes > 0 ? totalBytes : 1));
            if (value == 0xF7)
            {
                midiMsgOut->sendMessage(&message);
                SLEEP((bytesSinceLastFrame * 1000 + 3124) / 3125); // Drain at DIN MIDI's 3125-byte/s rate.
                message.clear();
                bytesSinceLastFrame = 0;
            }
        }
        if (!message.empty())
            emit errorSignal(tr("Syx Output Error"), tr("unterminated SysEx frame (missing F7)"));
    }
    catch (MidiBackendError &error)
    {
        error.printMessage();
        emit errorSignal(tr("Syx Output Error"), QString::fromStdString(error.getMessage()));
    }
cleanup:
    if (midiMsgOut)
    {
        if (portOpened)
        {
            try { midiMsgOut->closePort(); }
            catch (MidiBackendError &error) { error.printMessage(); }
        }
        delete midiMsgOut;
    }
};
/*********** send midi messages *******************************/
void midiIO::sendMidiMsg(QString sysxOutMsg, int midiOutPort)
{
    MidiBackendOut *midiMsgOut = 0;
    bool portOpened = false;
    const std::string clientName = "FxFloorBoard";
    try
    {
        midiMsgOut = createMidiOut(clientName, this);
        if (midiMsgOut->getPortCount() < 1)
        {
            emit errorSignal(tr("Midi Output Error"), tr("no MIDI output device available"));
            goto cleanup;
        }
        if ((sysxOutMsg.size() % 2) != 0)
        {
            emit errorSignal(tr("Midi Output Error"), tr("invalid hexadecimal message length"));
            goto cleanup;
        }
        midiMsgOut->openPort(static_cast<unsigned int>(midiOutPort), clientName);
        portOpened = true;
        std::vector<unsigned char> message;
        message.reserve(static_cast<size_t>(sysxOutMsg.size() / 2));
        for (int i = 0; i < sysxOutMsg.size(); i += 2)
        {
            bool ok = false;
            const unsigned int value = sysxOutMsg.mid(i, 2).toUInt(&ok, 16);
            if (!ok || value > 0xFF)
            {
                emit errorSignal(tr("Midi Output Error"), tr("invalid hexadecimal byte in MIDI message"));
                goto cleanup;
            }
            message.push_back(static_cast<unsigned char>(value));
        }
        if (!message.empty())
            midiMsgOut->sendMessage(&message);
    }
    catch (MidiBackendError &error)
    {
        error.printMessage();
        emit errorSignal(tr("Midi Output Error"), QString::fromStdString(error.getMessage()));
    }
cleanup:
    if (midiMsgOut)
    {
        if (portOpened)
        {
            try { midiMsgOut->closePort(); }
            catch (MidiBackendError &error) { error.printMessage(); }
        }
        delete midiMsgOut;
    }
};
/*********************** receiveMsg() **********************************
 * Prepares the sysx message after receiving on the MIDI In device. It
 *  and opens, receives and closes the MIDI device then converts the
 *  message from a std::vector to a QString.
 *************************************************************************/
void midicallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{
    Q_UNUSED(deltatime);
    midiIO *midi = static_cast<midiIO *>(userData);
    if (!midi || !message || message->size() < 2)
        return;
    if (message->front() != 0xF0 || message->back() != 0xF7)
        return;
    QString rxData;
    for (std::vector<unsigned char>::const_iterator it = message->begin(); it != message->end(); ++it)
    {
        QString hex = QString::number(static_cast<unsigned int>(*it), 16).toUpper();
        if (hex.length() < 2) hex.prepend("0");
        rxData.append(hex);
    }
    midi->callbackMsg(rxData);
};

void midiIO::reportMidiBackendError(QString errorMessage)
{
#ifdef Q_OS_LINUX
    this->midiBackendErrorOccurred.store(true);
#endif
    emit errorSignal(tr("MIDI Backend Error"), errorMessage);
};

void midiIO::callbackMsg(QString rxData)
{
    // Accept only the reply family that matches this request. This prevents
    // unrelated MIDI SysEx traffic from satisfying the byte-count wait.
    if (msgType == "identity")
    {
        if (rxData != idReplyPatern)
            return;
    }
    else
    {
        if (!rxData.startsWith("F0410000002F12"))
            return;

        // Name and patch replies must match the exact requested memory address.
        // System dumps are multi-frame transfers and legitimately advance addresses.
        if (msgType == "name" || msgType == "patch")
        {
            const QString requestedAddress = sysxOutMsg.mid(sysxAddressOffset * 2, 8);
            const QString replyAddress = rxData.mid(sysxAddressOffset * 2, 8);
            if (requestedAddress.size() != 8 || replyAddress != requestedAddress)
                return;
        }
    }

    QMutexLocker locker(&sysxBufferMutex);
    sysxBuffer.append(rxData);
};
void midiIO::receiveMsg(int midiInPort)
{
    count = 0;
    emit setStatusSymbol(3);
#ifdef Q_OS_WIN
    int timeoutFactor = 1;
#else
    int timeoutFactor = 3;
#endif
    if (msgType == "patch") { loopCount = timeoutFactor * 200; count = patchReplySize; }
    else if (msgType == "system") { loopCount = timeoutFactor * 400; count = systemSize; }
    else if (msgType == "name") { loopCount = timeoutFactor * 20; count = 29; }
    else if (msgType == "identity") { loopCount = timeoutFactor * 100; count = 15; }
    else { loopCount = timeoutFactor * 10; count = 0; }

    MidiBackendIn *midiin = 0;
    bool callbackInstalled = false;
    bool portOpened = false;
    const std::string clientName = "FxFloorBoard";
    try
    {
        midiin = createMidiIn(clientName, this);
        if (midiin->getPortCount() < 1) goto cleanup;
#ifdef Q_OS_LINUX
        // GT-10 patch and system replies exceed RtMidi's 1 KiB default input buffer.
        // Set the ALSA decoder buffer before opening the input port.
        midiin->setBufferSize(4096, 4);
#endif
        midiin->ignoreTypes(false, true, true);
        midiin->setCallback(&midicallback, this);
        callbackInstalled = true;
        midiin->openPort(static_cast<unsigned int>(midiInPort), clientName);
        portOpened = true;
        sendSyxMsg(sysxOutMsg, midiOutPort);
        int elapsed = 0;
        unsigned int progress = 1;
        unsigned int timeProgress = 1;
        int receivedBytes = 0;
        while (elapsed < loopCount && receivedBytes < count)
        {
#ifdef Q_OS_LINUX
            if (this->midiBackendErrorOccurred.load())
                break;
#endif
            SLEEP(5);
            {
                QMutexLocker locker(&sysxBufferMutex);
                receivedBytes = sysxBuffer.size() / 2;
            }
            progress = count > 0 ? static_cast<unsigned int>((receivedBytes * 50) / count) : 0;
            timeProgress = static_cast<unsigned int>((elapsed * 200) / loopCount);
            if (progress < timeProgress) progress = timeProgress;
            emit setStatusProgress(static_cast<int>(progress));
            ++elapsed;
        }
    }
    catch (MidiBackendError &error)
    {
        error.printMessage();
        emit errorSignal(tr("Midi Input Error"), QString::fromStdString(error.getMessage()));
    }
cleanup:
    emit setStatusProgress(100);
    if (midiin)
    {
        if (callbackInstalled) midiin->cancelCallback();
        if (portOpened) midiin->closePort();
    }
    {
        QMutexLocker locker(&sysxBufferMutex);
        this->sysxInMsg = this->sysxBuffer;
    }
#ifdef Q_OS_LINUX
    if (this->midiBackendErrorOccurred.load())
        this->sysxInMsg.clear();
#endif
    delete midiin;
};
/**************************** run() **************************************
 * New QThread that processes the sysex or midi message and handles if yes
 * or no it has to start receiving a reply on the MIDI In device midiIn. If
 * so midiCallback() will handle the receive of the incomming sysx message.
 *************************************************************************/
void midiIO::run()
{
  int repeat = 0;
        if(midi && midiMsg.size() > 1)	// Check if we are going to send sysx or midi data & have an actual midi message to send.
        {
                if (midiMsg.size() <= 6)		// if the midi message is <= 3 words
                        {
                        sysxOutMsg = midiMsg;   // use the same sending routine as sysx messages.
                        sendMidiMsg(sysxOutMsg, midiOutPort);
                  }
                else
                        {
                     int z=0;
                           sysxOutMsg = midiMsg.mid(z,2);
                                 sysxOutMsg.append(midiMsg.mid(z+2,2));  // skip every second byte
                                 sysxOutMsg.append(midiMsg.mid(z+4,2));  // skip every second byte
                                 sendMidiMsg(sysxOutMsg, midiOutPort);
                                 z=(z+6);								// advance to the next midi message in the string
                                 sysxOutMsg = midiMsg.mid(z,2);
                                 sysxOutMsg.append(midiMsg.mid(z+2,2));  // skip every second byte
                                 sysxOutMsg.append(midiMsg.mid(z+4,2));  // skip every second byte
                                 sendMidiMsg(sysxOutMsg, midiOutPort);
                                 z=(z+6);								// advance to the next midi message in the string
                                 sysxOutMsg = midiMsg.mid(z,2);
                                 sysxOutMsg.append(midiMsg.mid(z+2,2));  // skip every second byte
                                 sysxOutMsg.append(midiMsg.mid(z+4,2));  // skip every second byte
                                 sendMidiMsg(sysxOutMsg, midiOutPort);
                         };
                emit setStatusSymbol(2);
                emit setStatusProgress(33); // time wasting sinusidal statusbar progress animation
                SLEEP(40);
                emit setStatusProgress(66);
                SLEEP(100);
                emit setStatusProgress(100);
                SLEEP(100);
                emit setStatusProgress(75);
                SLEEP(100);
                emit setStatusProgress(42);
                SLEEP(150);
                emit setStatusProgress(0);
                emit midiFinished(); // We are finished so we send a signal to free the device.
        }
        else   // if not a midi message, then it must be a sysx message
        {
        RECEIVE:
                {
                    QMutexLocker locker(&sysxBufferMutex);
                    this->sysxBuffer.clear();
                }
                this->sysxInMsg.clear();
                const bool expectsReply =
                    (this->sysxOutMsg.mid(sysxAddressOffset * 2 - 2, 2) != "12");
                if (expectsReply)
                {
                    emit setStatusSymbol(3);
                    receiveMsg(midiInPort);

                    int receivedBytes = this->sysxInMsg.size() / 2;
                    Preferences *preferences = Preferences::Instance();
                    bool mayRetry =
                        preferences->getPreferences("Midi", "DBug", "bool") != "true";
#ifdef Q_OS_LINUX
                    mayRetry = mayRetry && !this->midiBackendErrorOccurred.load();
#endif
                    if (receivedBytes != count && repeat < 3 && mayRetry)
                    {
                        emit setStatusdBugMessage(tr("re-trying data request"));
                        ++repeat;
                        goto RECEIVE;
                    }

                    if (count > 0 && receivedBytes != count)
                    {
                        // Never forward a partial or unrelated response to the patch parser.
                        this->sysxInMsg.clear();
                        emit errorSignal(tr("Midi Input Error"), tr("incomplete GT-10 SysEx response"));
                    }
                    emit midiFinished();
                }
                else
                {
                    emit setStatusSymbol(2);
                    emit setStatusMessage(tr("Sending"));
                    sendSyxMsg(sysxOutMsg, midiOutPort);
                    Preferences *preferences = Preferences::Instance();
                    bool ok = false;
                    int minWait = preferences->getPreferences("Midi", "Delay", "set").toInt(&ok, 10);
                    if (!ok || minWait <= 0)
                        minWait = 10;
                    emit setStatusProgress(33);
                    SLEEP((100 / minWait) * 2);
                    emit setStatusProgress(75);
                    SLEEP((100 / minWait) * 3);
                    emit setStatusProgress(100);
                    SLEEP((100 / minWait) * 3);
                    emit setStatusProgress(66);
                    SLEEP((100 / minWait) * 2);
                    emit midiFinished();
                }
                emit replyMsg(this->sysxInMsg);
                emit setStatusSymbol(1);
                emit setStatusProgress(0);
                
                };
};

/*********************** sendSysxMsg() ***********************************
 * Starts a new thread that handles the send and receive of sysex messages.
 *************************************************************************/
void midiIO::sendSysxMsg(QString sysxOutMsg, int midiOutPort, int midiInPort)
{
  QString reBuild;
  QString sysxEOF;
  QString hex;
  int msgLength = sysxOutMsg.length()/2;
  for(int i=0;i<msgLength*2;++i)
  {
        hex.append(sysxOutMsg.mid(i*2, 2));
        sysxEOF = sysxOutMsg.mid((i*2)+4, 2);
  if (sysxEOF == "F7")
    {
        int dataSize = 0; bool ok;
          for(int h=checksumOffset;h<hex.size()-1;++h)
          { dataSize += hex.mid(h*2, 2).toInt(&ok, 16); };
                QString base = "80";                       // checksum calculate.
          unsigned int sum = dataSize % base.toInt(&ok, 16);
        if(sum!=0) { sum = base.toInt(&ok, 16) - sum; };
          QString checksum = QString::number(sum, 16).toUpper();
           if(checksum.length()<2) {checksum.prepend("0");};
        hex.append(checksum);
        hex.append("F7");
        if (!hex.contains("F0410000001B12")) // remove txt portions of the message from address 00 0D 00 00 onwards
         {reBuild.append(hex); };
                hex.clear();
                sysxEOF.clear();
                i=i+2;
    };
  };

  if (sysxOutMsg == idRequestString)
   {reBuild = sysxOutMsg;  msgType = "identity";  emit setStatusdBugMessage(tr("identity request")); };  // identity request not require checksum
        this->sysxOutMsg = reBuild.simplified().toUpper();

  if((sysxOutMsg.size() == (sysxDataOffset*2 + 12)) && (sysxOutMsg.mid(sysxOutMsg.size()-12, 8) == patchRequestDataSize) && (sysxOutMsg.mid((sysxAddressOffset*2-2), 2) == "11") )
   { msgType = "patch"; emit setStatusdBugMessage(tr("patch request")); };

  if((sysxOutMsg.size() == 34) && (sysxOutMsg.contains("00000010")) && (sysxOutMsg.mid((sysxAddressOffset*2-2), 2) == "11"))
   { msgType = "name";  emit setStatusdBugMessage(tr("name request")); };

  if (this->sysxOutMsg == systemRequest )
   { msgType = "system"; emit setStatusdBugMessage(tr("system request")); };

        this->midiOutPort = midiOutPort;
        this->midiInPort = midiInPort;
        this->midi = false;
        Preferences *preferences = Preferences::Instance();// Load the preferences.
        QString midiOut = preferences->getPreferences("Midi", "MidiOut", "device");
  if(midiOut!="")
{
    // Defer destruction until QThread::run() has actually returned.
    QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    start();
}
else
{
    emit setStatusSymbol(0);
    emit setStatusMessage(tr("no midi device set"));
    emit replyMsg("");
    deleteLater();
};

};

/*********************** sendMidi() **********************************
 * Starts a new thread that handles the sending of the midi messages.
 **********************************************************************/
void midiIO::sendMidi(QString midiMsg, int midiOutPort)
{
        this->midiOutPort = midiOutPort;
        this->midiMsg = midiMsg;
        this->midi = true;
  Preferences *preferences = Preferences::Instance();// Load the preferences.
        QString midiOut = preferences->getPreferences("Midi", "MidiOut", "device");
  if(midiOut!="")
{
    QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    start();
}
else
{
    emit setStatusSymbol(0);
    emit setStatusMessage(tr("no midi device set"));
    deleteLater();
};
};

