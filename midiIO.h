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

#ifndef MIDIIO_H
#define MIDIIO_H
#include <vector>
#include <QThread>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMutex>
#ifdef Q_OS_LINUX
#include <atomic>
#endif

class midiIO: public QThread
{
	Q_OBJECT

public:
	midiIO();
	void run();
	void sendSysxMsg(QString sysxOutMsg, const QString &midiOutPort, const QString &midiInPort);
	void sendMidi(QString midiMsg, const QString &midiOutPort);
	void callbackMsg(QString rxData);
	void reportMidiBackendError(QString windowTitle, QString errorMessage);
	QList<QString> getMidiOutDevices();
	QList<QString> getMidiInDevices();
	static QString portSelectionKey(const QString &portName);
	static bool portSelectionMatches(const QString &savedSelection, const QString &currentSelection);
		
signals:
	void errorSignal(QString windowTitle, QString errorMsg);
	void replyMsg(QString sysxInMsg);
	void midiFinished();
	void started();
	void setStatusSymbol(int value);
	void setStatusProgress(int value);
  void setStatusMessage(QString message);
  void setStatusdBugMessage(QString dBug);
		
private:
	void queryMidiInDevices();
	void queryMidiOutDevices();
	void sendSyxMsg(QString sysxOutMsg, const QString &midiOutPort);
	void sendMidiMsg(QString sysxOutMsg, const QString &midiOutPort);
	void receiveMsg(const QString &midiInPort);
	QList<QString> midiOutDevices;
	QList<QString> midiInDevices;
	
	QString msgType;
	int loopCount;
	QString sysxBuffer;
	QMutex sysxBufferMutex;
#ifdef Q_OS_LINUX
	std::atomic<bool> midiBackendErrorOccurred;
#endif
	QString midiOutPort;
	QString midiInPort;
	QString sysxOutMsg;
	QString sysxInMsg;
	QString midiMsg;
	int dataSize;
	int h;
	QString reBuild;
	QString hex;
	bool midi;
	int count;
};

#endif // MIDIIO_H
