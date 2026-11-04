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
    if (owner && type != rt::midi::RtMidiError::DEBUG_WARNING)
        owner->reportMidiBackendError(QString::fromLatin1("MIDI Backend Error"), QString::fromStdString(errorText));
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

namespace
{
QString stablePortNameKey(const QString &portName)
{
#ifdef Q_OS_LINUX
    // ALSA's display name ends with a transient numeric "client:port" address.
    const int addressSeparator = portName.lastIndexOf(' ');
    if (addressSeparator > 0)
    {
        const QString address = portName.mid(addressSeparator + 1);
        const int colon = address.indexOf(':');
        if (colon > 0)
        {
            bool clientOk = false;
            bool portOk = false;
            address.left(colon).toUInt(&clientOk);
            address.mid(colon + 1).toUInt(&portOk);
            if (clientOk && portOk)
                return portName.left(addressSeparator);
        }
    }
#endif
    return portName;
}

bool decodePortSelectionKey(const QString &selection, QString &exactName, QString &stableName)
{
    const int separator = selection.indexOf(':');
    bool lengthOk = false;
    const int exactLength = separator > 0 ? selection.left(separator).toInt(&lengthOk) : -1;
    if (lengthOk && exactLength >= 0 && separator + 1 + exactLength <= selection.size())
    {
        exactName = selection.mid(separator + 1, exactLength);
        stableName = selection.mid(separator + 1 + exactLength);
        if (stableName.isEmpty())
            stableName = stablePortNameKey(exactName);
        return true;
    }

    exactName = selection;
    bool legacyIndex = false;
    selection.toUInt(&legacyIndex, 10);
    stableName = legacyIndex ? QString() : stablePortNameKey(selection);
    return false;
}

template <typename MidiBackend>
bool resolveMidiPort(MidiBackend *backend, const QString &selectedPort,
                     unsigned int &portIndex, midiIO *owner,
                     const QString &direction)
{
    if (selectedPort.isEmpty())
    {
        owner->reportMidiBackendError(QObject::tr("MIDI Port Error"),
            QObject::tr("No MIDI %1 port has been selected.").arg(direction));
        return false;
    }

    QString exactName;
    QString stableName;
    const bool hasStructuredKey = decodePortSelectionKey(selectedPort, exactName, stableName);
    if (!hasStructuredKey)
    {
        bool legacyIndex = false;
        selectedPort.toUInt(&legacyIndex, 10);
        if (legacyIndex)
        {
            owner->reportMidiBackendError(QObject::tr("MIDI Port Error"),
                QObject::tr("The saved MIDI selection is an old device index. Please select the port again."));
            return false;
        }
    }

    const unsigned int portCount = backend->getPortCount();
    unsigned int exactMatches = 0;
    unsigned int stableMatches = 0;
    unsigned int stablePortIndex = 0;
    for (unsigned int i = 0; i < portCount; ++i)
    {
        const QString currentName = QString::fromStdString(backend->getPortName(i));
        if (currentName == exactName)
        {
            portIndex = i;
            ++exactMatches;
        }
        if (!stableName.isEmpty() && stablePortNameKey(currentName) == stableName)
        {
            stablePortIndex = i;
            ++stableMatches;
        }
    }

    if (exactMatches == 1)
        return true;
    if (exactMatches > 1)
    {
        owner->reportMidiBackendError(QObject::tr("MIDI Port Error"),
            QObject::tr("The selected MIDI %1 port is ambiguous. Please select it again: %2")
                .arg(direction, selectedPort));
        return false;
    }
    if (stableMatches == 1)
    {
        portIndex = stablePortIndex;
        return true;
    }

    const QString message = stableMatches == 0
        ? QObject::tr("The selected MIDI %1 port is no longer available: %2")
              .arg(direction, selectedPort)
        : QObject::tr("The selected MIDI %1 port is ambiguous. Please select it again: %2")
              .arg(direction, selectedPort);
    owner->reportMidiBackendError(QObject::tr("MIDI Port Error"), message);
    return false;
}
}

QString midiIO::portSelectionKey(const QString &portName)
{
    const QString stableName = stablePortNameKey(portName);
    return QString::number(portName.size()) + ":" + portName + stableName;
}

bool midiIO::portSelectionMatches(const QString &savedSelection, const QString &currentSelection)
{
    QString savedExact;
    QString savedStable;
    QString currentExact;
    QString currentStable;
    const bool savedHasStructuredKey = decodePortSelectionKey(savedSelection, savedExact, savedStable);
    if (!savedHasStructuredKey)
    {
        bool legacyIndex = false;
        savedSelection.toUInt(&legacyIndex, 10);
        if (legacyIndex)
            return false;
    }
    decodePortSelectionKey(currentSelection, currentExact, currentStable);
    return savedExact == currentExact ||
           (!savedStable.isEmpty() && savedStable == currentStable);
}

namespace
{
std::mutex persistentMidiInputMutex;
std::mutex persistentMidiOutputMutex;
std::unique_ptr<MidiBackendIn> persistentMidiInput;
std::unique_ptr<MidiBackendOut> persistentMidiOutput;
QString persistentMidiInputSelection;
QString persistentMidiOutputSelection;

#ifdef Q_OS_LINUX
template <typename MidiBackend>
void setMidiErrorOwner(MidiBackend *backend, midiIO *owner)
{
    backend->setErrorCallback(owner ? &midiBackendErrorCallback : 0, owner);
}
#else
template <typename MidiBackend>
void setMidiErrorOwner(MidiBackend *, midiIO *) {}
#endif

bool ensurePersistentOutput(const QString &selection, midiIO *owner)
{
    if (!persistentMidiOutput)
        persistentMidiOutput.reset(createMidiOut("FxFloorBoard", owner));

    setMidiErrorOwner(persistentMidiOutput.get(), owner);

    if (persistentMidiOutput->isPortOpen() && persistentMidiOutputSelection != selection)
        persistentMidiOutput->closePort();

    if (!persistentMidiOutput->isPortOpen())
    {
        unsigned int portIndex = 0;
        if (!resolveMidiPort(persistentMidiOutput.get(), selection, portIndex, owner, QObject::tr("output")))
            return false;

        persistentMidiOutput->openPort(portIndex, "FxFloorBoard");
        if (!persistentMidiOutput->isPortOpen())
        {
            owner->reportMidiBackendError(QObject::tr("MIDI Output Error"),
                QObject::tr("The selected MIDI output port could not be opened."));
            return false;
        }
        persistentMidiOutputSelection = selection;
    }
    return true;
}

bool ensurePersistentInput(const QString &selection, midiIO *owner)
{
    if (!persistentMidiInput)
        persistentMidiInput.reset(createMidiIn("FxFloorBoard", owner));

    setMidiErrorOwner(persistentMidiInput.get(), owner);

    if (persistentMidiInput->isPortOpen() && persistentMidiInputSelection != selection)
        persistentMidiInput->closePort();

    if (!persistentMidiInput->isPortOpen())
    {
        unsigned int portIndex = 0;
        if (!resolveMidiPort(persistentMidiInput.get(), selection, portIndex, owner, QObject::tr("input")))
            return false;

#ifdef Q_OS_LINUX
        // GT-10 patch and system replies exceed RtMidi's 1 KiB default input buffer.
        persistentMidiInput->setBufferSize(4096, 4);
#endif
        persistentMidiInput->ignoreTypes(false, true, true);
        persistentMidiInput->openPort(portIndex, "FxFloorBoard");
        if (!persistentMidiInput->isPortOpen())
        {
            owner->reportMidiBackendError(QObject::tr("MIDI Input Error"),
                QObject::tr("The selected MIDI input port could not be opened."));
            return false;
        }
        persistentMidiInputSelection = selection;
    }
    return true;
}

void clearMidiErrorOwner(MidiBackendIn *backend)
{
    if (backend)
        setMidiErrorOwner(backend, 0);
}

void clearMidiErrorOwner(MidiBackendOut *backend)
{
    if (backend)
        setMidiErrorOwner(backend, 0);
}
}

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
    reportMidiBackendError(tr("Midi Output Error"), tr("port error"));
    goto cleanup; };
  try { outPorts = midiout->getPortCount(); }
  catch (MidiBackendError &error) {
    error.printMessage();
    reportMidiBackendError(tr("Midi Output Error"), tr("enumeration error"));
    goto cleanup;
  };
  for ( unsigned int i=0; i<outPorts; i++ ) {
    try {
      portName = midiout->getPortName(i);
        }
    catch (MidiBackendError &error) {
      error.printMessage();
      reportMidiBackendError(tr("Midi Output Error"), tr("data error"));
      goto cleanup; };
this->midiOutDevices.append(QString::fromStdString(portName));
  };
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
    reportMidiBackendError(tr("Midi Input Error"), tr("port error"));
    goto cleanup; };
  try { inPorts = midiin->getPortCount(); }
  catch (MidiBackendError &error) {
    error.printMessage();
    reportMidiBackendError(tr("Midi Input Error"), tr("enumeration error"));
    goto cleanup;
  };
  for ( unsigned int i=0; i<inPorts; i++ ) {
     try { portName = midiin->getPortName(i); }
     catch (MidiBackendError &error) {
        error.printMessage();
        reportMidiBackendError(tr("Midi Input Error"), tr("data error"));
        goto cleanup; };
this->midiInDevices.append(QString::fromStdString(portName));
  };
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
void midiIO::sendSyxMsg(QString sysxOutMsg, const QString &midiOutPort)
{
    std::lock_guard<std::mutex> sessionLock(persistentMidiOutputMutex);
    try
    {
        if (!ensurePersistentOutput(midiOutPort, this))
            goto cleanup;

        if ((sysxOutMsg.size() % 2) != 0)
        {
            reportMidiBackendError(tr("Syx Output Error"), tr("invalid hexadecimal message length"));
            goto cleanup;
        }

        {
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
                    reportMidiBackendError(tr("Syx Output Error"), tr("invalid hexadecimal byte in SysEx message"));
                    goto cleanup;
                }
                if (message.empty() && value != 0xF0)
                {
                    reportMidiBackendError(tr("Syx Output Error"), tr("SysEx frame does not start with F0"));
                    goto cleanup;
                }
                message.push_back(static_cast<unsigned char>(value));
                ++bytesSinceLastFrame;
                emit setStatusProgress((i + 1) * 100 / (totalBytes > 0 ? totalBytes : 1));
                if (value == 0xF7)
                {
                    persistentMidiOutput->sendMessage(&message);
                    SLEEP((bytesSinceLastFrame * 1000 + 3124) / 3125);
                    message.clear();
                    bytesSinceLastFrame = 0;
                }
            }
            if (!message.empty())
                reportMidiBackendError(tr("Syx Output Error"), tr("unterminated SysEx frame (missing F7)"));
        }
    }
    catch (MidiBackendError &error)
    {
        error.printMessage();
        reportMidiBackendError(tr("Syx Output Error"), QString::fromStdString(error.getMessage()));
    }
cleanup:
    clearMidiErrorOwner(persistentMidiOutput.get());
};
/*********** send midi messages *******************************/

void midiIO::sendMidiMsg(QString sysxOutMsg, const QString &midiOutPort)
{
    std::lock_guard<std::mutex> sessionLock(persistentMidiOutputMutex);
    try
    {
        if (!ensurePersistentOutput(midiOutPort, this))
            goto cleanup;

        if ((sysxOutMsg.size() % 2) != 0)
        {
            reportMidiBackendError(tr("Midi Output Error"), tr("invalid hexadecimal message length"));
            goto cleanup;
        }

        std::vector<unsigned char> message;
        message.reserve(static_cast<size_t>(sysxOutMsg.size() / 2));
        for (int i = 0; i < sysxOutMsg.size(); i += 2)
        {
            bool ok = false;
            const unsigned int value = sysxOutMsg.mid(i, 2).toUInt(&ok, 16);
            if (!ok || value > 0xFF)
            {
                reportMidiBackendError(tr("Midi Output Error"), tr("invalid hexadecimal byte in MIDI message"));
                goto cleanup;
            }
            message.push_back(static_cast<unsigned char>(value));
        }
        if (!message.empty())
            persistentMidiOutput->sendMessage(&message);
    }
    catch (MidiBackendError &error)
    {
        error.printMessage();
        reportMidiBackendError(tr("Midi Output Error"), QString::fromStdString(error.getMessage()));
    }
cleanup:
    clearMidiErrorOwner(persistentMidiOutput.get());
};
/*********************** receiveMsg() **********************************

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

void midiIO::reportMidiBackendError(QString windowTitle, QString errorMessage)
{
#ifdef Q_OS_LINUX
    this->midiBackendErrorOccurred.store(true);
#endif
    emit errorSignal(windowTitle, errorMessage);
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
void midiIO::receiveMsg(const QString &midiInPort)
{
    std::lock_guard<std::mutex> sessionLock(persistentMidiInputMutex);
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

    bool callbackInstalled = false;
    try
    {
        if (!ensurePersistentInput(midiInPort, this))
            goto cleanup;

        persistentMidiInput->setCallback(&midicallback, this);
        callbackInstalled = true;
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
        reportMidiBackendError(tr("Midi Input Error"), QString::fromStdString(error.getMessage()));
    }
cleanup:
    emit setStatusProgress(100);
    if (callbackInstalled && persistentMidiInput)
    {
        try { persistentMidiInput->cancelCallback(); }
        catch (MidiBackendError &error)
        {
            error.printMessage();
            reportMidiBackendError(tr("Midi Input Error"), tr("could not cancel MIDI callback cleanly"));
        }
    }
    clearMidiErrorOwner(persistentMidiInput.get());

    {
        QMutexLocker locker(&sysxBufferMutex);
        this->sysxInMsg = this->sysxBuffer;
    }
#ifdef Q_OS_LINUX
    if (this->midiBackendErrorOccurred.load())
        this->sysxInMsg.clear();
#endif
};
/**************************** run() **************************************
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
void midiIO::sendSysxMsg(QString sysxOutMsg, const QString &midiOutPort, const QString &midiInPort)
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
void midiIO::sendMidi(QString midiMsg, const QString &midiOutPort)
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

