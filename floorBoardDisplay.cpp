/****************************************************************************
**
** Copyright (C) 2007~2014 Colin Willcocks.
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

#include <QChar>
#include "floorBoardDisplay.h"
#include "Preferences.h"
#include "MidiTable.h"
#include "SysxIO.h"
#include "midiIO.h"
#include "renameWidget.h"
#include "customRenameWidget.h"
#include "customComboBox.h"
#include "globalVariables.h"



// Platform-dependent sleep routines.
#ifdef Q_OS_WIN
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds )
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif


floorBoardDisplay::floorBoardDisplay(QWidget *parent, QPoint pos)
    : QWidget(parent)
{
        this->pos = pos;
        this->timer = new QTimer(this);
        this->patchLoadError = false;
        this->blinkCount = 0;

        int patchDisplayRowOffset = 5;
        int editButtonRowOffset = 60;
        int tempRowOffset = 580;
        int bottomOffset = 635;
        this->patchNumDisplay = new customDisplay(QRect(10, patchDisplayRowOffset, 50, 34), this);
        this->patchNumDisplay->setLabelPosition(true);
        this->patchNumDisplay->setMainObjectName("bankMain");
        this->patchNumDisplay->setSubObjectName("bankSub");
        this->patchNumDisplay->setWhatsThis(tr("Patch Number Display.<br>displays the currently selected patch<br>and patch write memory location."));
        this->patchDisplay = new customDisplay(QRect(70, patchDisplayRowOffset, 150, 34), this);
        this->patchDisplay->setMainObjectName("nameMain");
        this->patchDisplay->setSubObjectName("nameSub");
        this->valueDisplay = new customDisplay(QRect(230, patchDisplayRowOffset, 150, 34), this);
        this->valueDisplay->setMainObjectName("valueMain");
        this->valueDisplay->setSubObjectName("valueSub");

        this->temp1Display = new customLabelDisplay(QRect(55, tempRowOffset+20, 170, 18), this);
        this->temp1Display->setLabelPosition(true);
        this->temp1Display->setMainObjectName("nameMain");
        this->temp1Display->setMainText(tr("Empty"), Qt::AlignCenter);
        this->temp1Display->setWhatsThis(tr("Name of the currently stored patch in the clipboard."));
        this->temp2Display = new customLabelDisplay(QRect(255, tempRowOffset+20, 170, 18), this);
        this->temp2Display->setLabelPosition(true);
        this->temp2Display->setMainObjectName("nameMain");
        this->temp2Display->setMainText(tr("Empty"), Qt::AlignCenter);
        this->temp2Display->setWhatsThis(tr("Name of the currently stored patch in the clipboard."));
        this->temp3Display = new customLabelDisplay(QRect(455, tempRowOffset+20, 170, 18), this);
        this->temp3Display->setLabelPosition(true);
        this->temp3Display->setMainObjectName("nameMain");
        this->temp3Display->setMainText(tr("Empty"), Qt::AlignCenter);
        this->temp3Display->setWhatsThis(tr("Name of the currently stored patch in the clipboard."));
        this->temp4Display = new customLabelDisplay(QRect(655, tempRowOffset+20, 170, 18), this);
        this->temp4Display->setLabelPosition(true);
        this->temp4Display->setMainObjectName("nameMain");
        this->temp4Display->setMainText(tr("Empty"), Qt::AlignCenter);
        this->temp4Display->setWhatsThis(tr("Name of the currently stored patch in the clipboard."));
        this->temp5Display = new customLabelDisplay(QRect(855, tempRowOffset+20, 170, 18), this);
        this->temp5Display->setLabelPosition(true);
        this->temp5Display->setMainObjectName("nameMain");
        this->temp5Display->setMainText(tr("Empty"), Qt::AlignCenter);
        this->temp5Display->setWhatsThis(tr("Name of the currently stored patch in the clipboard."));

        Preferences *preferences = Preferences::Instance();
        QString version = preferences->getPreferences("General", "Application", "version");
        this->patchDisplay->setMainText(deviceType + tr(" Fx FloorBoard"));
        this->patchDisplay->setSubText(tr("version"), version);

  initPatch = new initPatchListMenu(QRect(390, patchDisplayRowOffset+19, 168, 15), this);
  initPatch->setWhatsThis(tr("Clicking on this will load a patch from a predefined selection.<br>patches place in the Init Patches folder will appear in this list at the start of the next session."));
  renameWidget *nameEdit = new renameWidget(this);
  nameEdit->setGeometry(70, patchDisplayRowOffset, 150, 34);
  nameEdit->setWhatsThis(tr("Clicking on this will open<br>a text dialog window<br>allowing user text input."));
  customRenameWidget *userDialog = new customRenameWidget(this, "0E", "00", "00", "Structure", "20");
  userDialog->setGeometry(780, patchDisplayRowOffset+5, 320, 25);
  userDialog->setWhatsThis(tr("Clicking on this will open<br>a text dialog window<br>allowing user text input."));
  customRenameWidget *patchDialog = new customRenameWidget(this, "0D", "00", "00", "Structure", "80");
  patchDialog->setGeometry(50, bottomOffset, 980, 25);
  patchDialog->setWhatsThis(tr("Clicking on this will open<br>a text dialog window<br>allowing user text input."));
  this->output = new customControlListMenu(this, "00", "00", "11", "right");
  output->setGeometry(580, patchDisplayRowOffset, 180, 30);
  output->setWhatsThis(tr("This is the Patch Mode setting of Output Select<br>this is only active if the SYSTEM setting<br>is set to Patch Mode."));
  this->catagory = new customControlListMenu(this, "00", "00", "10", "right");
  catagory->setGeometry(580, patchDisplayRowOffset+19, 300, 30);

  this->connectButton = new customButton(tr("Connect"), false, QPoint(390, patchDisplayRowOffset), this, ":/images/greenledbutton.png");
  this->connectButton->setWhatsThis(tr("Connect Button<br>used to establish a continuous midi connection<br>when lit green, the connection is valid"));
  this->writeButton = new customButton(tr("Write/Sync"), false, QPoint(479, patchDisplayRowOffset), this, ":/images/ledbutton.png");
  this->writeButton->setWhatsThis(tr("Write/Sync Button<br>if the patch number displays [temp buffer]<br>the current patch is sent to the GT temporary memory only<br>or else the patch will be written to the displayed patch memory location."));
  this->system_midi_Button = new customPanelButton(tr("Midi"), false, QPoint(1000, editButtonRowOffset+35), this, ":/images/switch.png");
  this->system_midi_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->system_Button = new customPanelButton(tr("System"), false, QPoint(1000, editButtonRowOffset), this, ":/images/switch.png");
  this->system_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));

  this->ch_mode_Button = new customPanelButton(tr("CH MODE"), false, QPoint(40, editButtonRowOffset), this, ":/images/switch.png");
  this->ch_mode_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->channel_A_Button = new customPanelButton(tr("CH-A"), false, QPoint(90, editButtonRowOffset), this, ":/images/switch.png");
  this->channel_A_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->channel_B_Button = new customPanelButton(tr("CH-B"), false, QPoint(90, editButtonRowOffset+35), this, ":/images/switch.png");
  this->channel_B_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->preamp1_Button = new customPanelButton(tr("PREAMP-A"), false, QPoint(160, editButtonRowOffset), this, ":/images/switch.png");
  this->preamp1_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->preamp2_Button = new customPanelButton(tr("PREAMP-B"), false, QPoint(160, editButtonRowOffset+35), this, ":/images/switch.png");
  this->preamp2_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->solo1_Button = new customPanelButton(tr("SOLO-A"), false, QPoint(210, editButtonRowOffset), this, ":/images/solo_switch.png");
  this->solo1_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->solo2_Button = new customPanelButton(tr("SOLO-B"), false, QPoint(210, editButtonRowOffset+35), this, ":/images/solo_switch.png");
  this->solo2_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->compressor_Button = new customPanelButton(tr("Compressor"), false, QPoint(290,editButtonRowOffset), this, ":/images/switch.png");
  this->compressor_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->distortion_Button = new customPanelButton(tr("Distortion"), false, QPoint(290,editButtonRowOffset+35), this, ":/images/switch.png");
  this->distortion_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->ns1_Button = new customPanelButton(tr("NS-1"), false, QPoint(350, editButtonRowOffset), this, ":/images/switch.png");
  this->ns1_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->ns2_Button = new customPanelButton(tr("NS-2"), false, QPoint(350, editButtonRowOffset+35), this, ":/images/switch.png");
  this->ns2_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->fx1_Button = new customPanelButton(tr("FX-1"), false, QPoint(410, editButtonRowOffset), this, ":/images/switch.png");
  this->fx1_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->fx2_Button = new customPanelButton(tr("FX-2"), false, QPoint(410, editButtonRowOffset+35), this, ":/images/switch.png");
  this->fx2_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->fx1_autoriff_Button = new customPanelButton(tr("FX1 AutoRiff"), false, QPoint(470, editButtonRowOffset), this, ":/images/switch.png");
  this->fx1_autoriff_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->fx2_autoriff_Button = new customPanelButton(tr("FX2 AutoRiff"), false, QPoint(470, editButtonRowOffset+35), this, ":/images/switch.png");
  this->fx2_autoriff_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->sendreturn_Button = new customPanelButton(tr("Send/Return"), false, QPoint(530, editButtonRowOffset+35), this, ":/images/switch.png");
  this->sendreturn_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->eq_Button = new customPanelButton(tr("Equalizer"), false, QPoint(590, editButtonRowOffset), this, ":/images/switch.png");
  this->eq_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->chorus_Button = new customPanelButton(tr("Chorus"), false, QPoint(590, editButtonRowOffset+35), this, ":/images/switch.png");
  this->chorus_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->reverb_Button = new customPanelButton(tr("Reverb"), false, QPoint(650, editButtonRowOffset), this, ":/images/switch.png");
  this->reverb_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->delay_Button = new customPanelButton(tr("Delay"), false, QPoint(650, editButtonRowOffset+35), this, ":/images/switch.png");
  this->delay_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->pedal_Button = new customPanelButton(tr("Pedal/FX"), false, QPoint(710, editButtonRowOffset), this, ":/images/switch.png");
  this->pedal_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->master_Button = new customPanelButton(tr("Master"), false, QPoint(710, editButtonRowOffset+35), this, ":/images/switch.png");
  this->master_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));

  this->assign1_Button = new customPanelButton(tr("1"), false, QPoint(795, editButtonRowOffset), this, ":/images/switch.png");
  this->assign1_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->assign2_Button = new customPanelButton(tr("2"), false, QPoint(840, editButtonRowOffset), this, ":/images/switch.png");
  this->assign2_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->assign3_Button = new customPanelButton(tr("3"), false, QPoint(885, editButtonRowOffset), this, ":/images/switch.png");
  this->assign3_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->assign4_Button = new customPanelButton(tr("4"), false, QPoint(930, editButtonRowOffset), this, ":/images/switch.png");
  this->assign4_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->assign5_Button = new customPanelButton(tr("5"), false, QPoint(795, editButtonRowOffset+35), this, ":/images/switch.png");
  this->assign5_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->assign6_Button = new customPanelButton(tr("6"), false, QPoint(840, editButtonRowOffset+35), this, ":/images/switch.png");
  this->assign6_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->assign7_Button = new customPanelButton(tr("7"), false, QPoint(885, editButtonRowOffset+35), this, ":/images/switch.png");
  this->assign7_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));
  this->assign8_Button = new customPanelButton(tr("8"), false, QPoint(930, editButtonRowOffset+35), this, ":/images/switch.png");
  this->assign8_Button->setWhatsThis(tr("Deep editing of the selected effect<br>pressing this button will open an edit page<br>allowing detailed setting of this effects parameters."));

  this->temp1_copy_Button = new customButton(tr("Patch-1 Copy"), false, QPoint(55, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp1_copy_Button->setWhatsThis(tr("Copy current patch to clipboard<br>pressing this button will save the current patch to a clipboard<br>the clipboard is saved to file and is re-loaded<br>on the next session startup."));
  this->temp1_paste_Button = new customButton(tr("Patch-1 Paste"), false, QPoint(145, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp1_paste_Button->setWhatsThis(tr("Paste current patch from clipboard<br>pressing this button will load the current patch to a clipboard<br>the clipboard is re-loaded<br>from the previous session copy."));
  this->temp2_copy_Button = new customButton(tr("Patch-2 Copy"), false, QPoint(255, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp2_copy_Button->setWhatsThis(tr("Copy current patch to clipboard<br>pressing this button will save the current patch to a clipboard<br>the clipboard is saved to file and is re-loaded<br>on the next session startup."));
  this->temp2_paste_Button = new customButton(tr("Patch-2 Paste"), false, QPoint(345, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp2_paste_Button->setWhatsThis(tr("Paste current patch from clipboard<br>pressing this button will load the current patch to a clipboard<br>the clipboard is re-loaded<br>from the previous session copy."));
  this->temp3_copy_Button = new customButton(tr("Patch-3 Copy"), false, QPoint(455, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp3_copy_Button->setWhatsThis(tr("Copy current patch to clipboard<br>pressing this button will save the current patch to a clipboard<br>the clipboard is saved to file and is re-loaded<br>on the next session startup."));
  this->temp3_paste_Button = new customButton(tr("Patch-3 Paste"), false, QPoint(545, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp3_paste_Button->setWhatsThis(tr("Paste current patch from clipboard<br>pressing this button will load the current patch to a clipboard<br>the clipboard is re-loaded<br>from the previous session copy."));
  this->temp4_copy_Button = new customButton(tr("Patch-4 Copy"), false, QPoint(655, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp4_copy_Button->setWhatsThis(tr("Copy current patch to clipboard<br>pressing this button will save the current patch to a clipboard<br>the clipboard is saved to file and is re-loaded<br>on the next session startup."));
  this->temp4_paste_Button = new customButton(tr("Patch-4 Paste"), false, QPoint(745, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp4_paste_Button->setWhatsThis(tr("Paste current patch from clipboard<br>pressing this button will load the current patch to a clipboard<br>the clipboard is re-loaded<br>from the previous session copy."));
  this->temp5_copy_Button = new customButton(tr("Patch-5 Copy"), false, QPoint(855, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp5_copy_Button->setWhatsThis(tr("Copy current patch to clipboard<br>pressing this button will save the current patch to a clipboard<br>the clipboard is saved to file and is re-loaded<br>on the next session startup."));
  this->temp5_paste_Button = new customButton(tr("Patch-5 Paste"), false, QPoint(945, tempRowOffset), this, ":/images/pushbutton.png");
  this->temp5_paste_Button->setWhatsThis(tr("Paste current patch from clipboard<br>pressing this button will load the current patch to a clipboard<br>the clipboard is re-loaded<br>from the previous session copy."));

        SysxIO *sysxIO = SysxIO::Instance();
        QObject::connect(this, SIGNAL(setStatusSymbol(int)), sysxIO, SIGNAL(setStatusSymbol(int)));
        QObject::connect(this, SIGNAL(setStatusProgress(int)), sysxIO, SIGNAL(setStatusProgress(int)));
        QObject::connect(this, SIGNAL(setStatusMessage(QString)), sysxIO, SIGNAL(setStatusMessage(QString)));

        QObject::connect(sysxIO, SIGNAL(notConnectedSignal()), this, SLOT(notConnected()));
        QObject::connect(this, SIGNAL(notConnectedSignal()), this, SLOT(notConnected()));

        QObject::connect(this->parent(), SIGNAL(updateSignal()), this, SLOT(updateDisplay()));
        QObject::connect(this, SIGNAL(updateSignal()), this->parent(), SIGNAL(updateSignal()));

        QObject::connect(this->connectButton, SIGNAL(valueChanged(bool)), this, SLOT(connectSignal(bool)));
        QObject::connect(this->writeButton, SIGNAL(valueChanged(bool)), this, SLOT(writeSignal(bool)));

        QObject::connect(this->ch_mode_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(ch_mode_buttonSignal(bool)));
        QObject::connect(this->channel_A_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(ch_mode_buttonSignal(bool)));
        QObject::connect(this->channel_B_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(ch_mode_buttonSignal(bool)));
        QObject::connect(this->preamp1_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(preamp1_buttonSignal(bool)));
        QObject::connect(this->preamp2_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(preamp2_buttonSignal(bool)));
        QObject::connect(this->solo1_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(preamp1_buttonSignal(bool)));
        QObject::connect(this->solo2_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(preamp2_buttonSignal(bool)));
        QObject::connect(this->distortion_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(distortion_buttonSignal(bool)));
        QObject::connect(this->compressor_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(compressor_buttonSignal(bool)));
        QObject::connect(this->ns1_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(ns1_buttonSignal(bool)));
        QObject::connect(this->ns2_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(ns2_buttonSignal(bool)));
        QObject::connect(this->fx1_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(fx1_buttonSignal(bool)));
        QObject::connect(this->fx2_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(fx2_buttonSignal(bool)));
        QObject::connect(this->fx1_autoriff_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(fx1_autoriff_buttonSignal(bool)));
        QObject::connect(this->fx2_autoriff_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(fx2_autoriff_buttonSignal(bool)));
        QObject::connect(this->reverb_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(reverb_buttonSignal(bool)));
        QObject::connect(this->delay_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(delay_buttonSignal(bool)));
        QObject::connect(this->chorus_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(chorus_buttonSignal(bool)));
        QObject::connect(this->sendreturn_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(sendreturn_buttonSignal(bool)));
        QObject::connect(this->eq_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(eq_buttonSignal(bool)));
        QObject::connect(this->pedal_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(pedal_buttonSignal(bool)));
        QObject::connect(this->master_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(master_buttonSignal(bool)));
    QObject::connect(this->system_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(system_buttonSignal(bool)));
    QObject::connect(this->system_midi_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(midi_buttonSignal(bool)));
    QObject::connect(this->assign1_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(assign1_buttonSignal(bool)));
    QObject::connect(this->assign2_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(assign2_buttonSignal(bool)));
    QObject::connect(this->assign3_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(assign3_buttonSignal(bool)));
    QObject::connect(this->assign4_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(assign4_buttonSignal(bool)));
    QObject::connect(this->assign5_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(assign5_buttonSignal(bool)));
    QObject::connect(this->assign6_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(assign6_buttonSignal(bool)));
    QObject::connect(this->assign7_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(assign7_buttonSignal(bool)));
    QObject::connect(this->assign8_Button, SIGNAL(valueChanged(bool)), this->parent(), SIGNAL(assign8_buttonSignal(bool)));
    
    QObject::connect(this->parent(), SIGNAL(ch_mode_statusSignal(bool)), this->ch_mode_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(channel_A_statusSignal(bool)), this->channel_A_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(channel_B_statusSignal(bool)), this->channel_B_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(preamp1_statusSignal(bool)), this->preamp1_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(preamp2_statusSignal(bool)), this->preamp2_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(solo1_statusSignal(bool)), this->solo1_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(solo2_statusSignal(bool)), this->solo2_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(distortion_statusSignal(bool)), this->distortion_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(compressor_statusSignal(bool)), this->compressor_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(ns1_statusSignal(bool)), this->ns1_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(ns2_statusSignal(bool)), this->ns2_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(fx1_statusSignal(bool)), this->fx1_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(fx2_statusSignal(bool)), this->fx2_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(reverb_statusSignal(bool)), this->reverb_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(delay_statusSignal(bool)), this->delay_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(chorus_statusSignal(bool)), this->chorus_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(eq_statusSignal(bool)), this->eq_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(pedal_statusSignal(bool)), this->pedal_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(sendreturn_statusSignal(bool)), this->sendreturn_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(master_statusSignal(bool)), this->master_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(system_statusSignal(bool)), this->system_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(system_midi_statusSignal(bool)), this->system_midi_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(assign1_statusSignal(bool)), this->assign1_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(assign2_statusSignal(bool)), this->assign2_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(assign3_statusSignal(bool)), this->assign3_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(assign4_statusSignal(bool)), this->assign4_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(assign5_statusSignal(bool)), this->assign5_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(assign6_statusSignal(bool)), this->assign6_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(assign7_statusSignal(bool)), this->assign7_Button, SLOT(setValue(bool)));
    QObject::connect(this->parent(), SIGNAL(assign8_statusSignal(bool)), this->assign8_Button, SLOT(setValue(bool)));

        QObject::connect(this->temp1_copy_Button, SIGNAL(valueChanged(bool)),  this, SLOT(temp1_copy(bool)));
        QObject::connect(this->temp1_paste_Button, SIGNAL(valueChanged(bool)), this, SLOT(temp1_paste(bool)));
        QObject::connect(this->temp2_copy_Button, SIGNAL(valueChanged(bool)),  this, SLOT(temp2_copy(bool)));
        QObject::connect(this->temp2_paste_Button, SIGNAL(valueChanged(bool)), this, SLOT(temp2_paste(bool)));
        QObject::connect(this->temp3_copy_Button, SIGNAL(valueChanged(bool)),  this, SLOT(temp3_copy(bool)));
        QObject::connect(this->temp3_paste_Button, SIGNAL(valueChanged(bool)), this, SLOT(temp3_paste(bool)));
        QObject::connect(this->temp4_copy_Button, SIGNAL(valueChanged(bool)),  this, SLOT(temp4_copy(bool)));
        QObject::connect(this->temp4_paste_Button, SIGNAL(valueChanged(bool)), this, SLOT(temp4_paste(bool)));
        QObject::connect(this->temp5_copy_Button, SIGNAL(valueChanged(bool)),  this, SLOT(temp5_copy(bool)));
        QObject::connect(this->temp5_paste_Button, SIGNAL(valueChanged(bool)), this, SLOT(temp5_paste(bool)));


        set_temp();

        QString midiIn = preferences->getPreferences("Midi", "MidiIn", "device");
        QString midiOut = preferences->getPreferences("Midi", "MidiOut", "device");
        if(midiIn!="" && midiOut!="")
        {autoconnect(); } else {
        QMessageBox *msgBox = new QMessageBox();
        msgBox->isTopLevel();
        msgBox->setWindowTitle(deviceType + tr(" FloorBoard"));
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setTextFormat(Qt::RichText);
        QString msgText;
        msgText.append("<font size='+1'><b>");
        msgText.append(tr("Midi device not selected in Preferences Menu."));
        msgText.append("<b></font><br>");
        msgText.append(tr("Select the GT-10 or a midi device from the Preferences USB/Midi menu and press the 'Connect' button"));
        msgBox->setText(msgText);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();};
        }

QPoint floorBoardDisplay::getPos()
{
        return this->pos;
}

void floorBoardDisplay::setPos(QPoint newPos)
{
        this->move(newPos);
        this->pos = newPos;
}

void floorBoardDisplay::setValueDisplay(QString fxName, QString valueName, QString value)
{
        this->valueDisplay->setMainText(fxName);
        this->valueDisplay->setSubText(valueName, value);
}

void floorBoardDisplay::setPatchDisplay(QString patchName)
{
        SysxIO *sysxIO = SysxIO::Instance();
        if(sysxIO->getFileName() != ":default.syx") // Keep the initial version display if the default.syx is loaded at startup.
        {
                QString fileName = sysxIO->getFileName();
                this->patchDisplay->setMainText(patchName);
                this->patchDisplay->setSubText(fileName.section('/', -1, -1));
                this->patchName = patchName;
        };
        if(sysxIO->getFileName() == tr("init patch") || sysxIO->getFileName() == ":default.syx")
        {
                sysxIO->setFileName("");
                this->patchName = tr("Empty");
        }
        else
        {
                if(sysxIO->getFileName() == deviceType + tr(" patch"))
                {
                        sysxIO->setFileName("");
                        if(this->patchLoadError)
                        {
                                QMessageBox *msgBox = new QMessageBox();
                                msgBox->setWindowTitle(deviceType + tr(" Fx FloorBoard"));
                                msgBox->setIcon(QMessageBox::Warning);
                                msgBox->setTextFormat(Qt::RichText);
                                QString msgText;
                                msgText.append("<font size='+1'><b>");
                                msgText.append(tr("Error while changing banks."));
                                msgText.append("<b></font><br>");
                                msgText.append(tr("An incorrect patch has been loaded. Please try to load the patch again."));
                                msgBox->setText(msgText);
                                msgBox->setStandardButtons(QMessageBox::Ok);
                                msgBox->exec();

                                sysxIO->setBank(0);
                                sysxIO->setPatch(0);
                        };
                };
    this->initPatch->setIndex(0);
        };
}

void floorBoardDisplay::setPatchNumDisplay(int bank, int patch)
{
        if(bank > 0)
        {
                        if(bank <= bankTotalUser)
                {
                 this->patchNumDisplay->resetAllColor();
     this->patchNumDisplay->setSubText(tr("User"));
                }
                else if (bank > bankTotalUser && bank <= bankTotalAll)
                {
                        this->patchNumDisplay->setAllColor(QColor(255,0,0));
      this->patchNumDisplay->setSubText(tr("Preset"));
                }
        else if (bank == 101)
                {
     this->patchNumDisplay->resetAllColor();
     this->patchNumDisplay->setSubText(tr("Quick"));
    }
    else
    {
     this->patchNumDisplay->setAllColor(QColor(255,0,0));
     this->patchNumDisplay->setSubText(tr("Quick"));
    };
                QString str;
                if(bank < 51)
    {
      str.append("U");
      if(bank < 10) {	str.append("0"); };
                  str.append(QString::number(bank, 10));
    }
      else if (bank > 50 && bank < 101)
                { str.append("P");
                if(bank < 60) {	str.append("0"); };
                str.append(QString::number(bank-50, 10));
          }
    else if (bank ==101)
    {
     str.append("U");
    }
    else
    {
     str.append("P");
    };
                str.append(":");
                str.append(QString::number(patch, 10));
          this->patchNumDisplay->setMainText(str, Qt::AlignCenter);
  }
        else
        {
                //this->patchNumDisplay->clearAll();
                this->patchNumDisplay->setSubText(tr("Temp"));
                QString str = tr("Buffer");
                this->patchNumDisplay->setMainText(str, Qt::AlignCenter);
        };
};

void floorBoardDisplay::updateDisplay()
{
        SysxIO *sysxIO = SysxIO::Instance();
        QString area = "Structure";
        QList<QString> nameArray = sysxIO->getFileSource(area, nameAddress, "00");
        QString name;
        for(int i=sysxDataOffset;i<(sysxDataOffset+nameLength);i++ )
                {

                QString hexStr = nameArray.at(i);
                bool ok;
                name.append( (char)(hexStr.toInt(&ok, 16)) );
    };

        QString patchName = name.trimmed();
        sysxIO->setCurrentPatchName(patchName);
        if(sysxIO->getRequestName().trimmed() != patchName.trimmed())
        {
                this->patchLoadError = true;
        }
        else
        {
                this->patchLoadError = false;
        };


        this->setPatchDisplay(patchName);
        if(sysxIO->isDevice())
        {
                int bank = sysxIO->getBank();
                int patch = sysxIO->getPatch();
                this->setPatchNumDisplay(bank, patch);
        }
        else
        {
                this->patchNumDisplay->clearAll();
        };
        this->valueDisplay->clearAll();

        if(sysxIO->isDevice() )  // comment out from here
        {
                if(sysxIO->getBank() > 50)
                {
                        this->writeButton->setBlink(false);
                        this->writeButton->setValue(true);
                }
                else
                {
                        this->writeButton->setBlink(false);
                        this->writeButton->setValue(true);
                };
                int bank = sysxIO->getBank();
                int patch = sysxIO->getPatch();
                this->setPatchNumDisplay(bank, patch);
        }
        else if(sysxIO->getBank() != 0)
        {
                if(sysxIO->isConnected())
                {
                        this->writeButton->setBlink(false);
                        this->writeButton->setValue(false);
                };
                int bank = sysxIO->getBank();
                int patch = sysxIO->getPatch();
                this->setPatchNumDisplay(bank, patch);
        }
        else
        {
                patchNumDisplay->clearAll();
                this->writeButton->setBlink(false);   
                this->writeButton->setValue(false);
        };  
        int index = sysxIO->getSourceValue("Structure", "00", "00", "11");
        this->output->controlListComboBox->setCurrentIndex(index);
        index = sysxIO->getSourceValue("Structure", "00", "00", "10");
        this->catagory->controlListComboBox->setCurrentIndex(index);
    };

void floorBoardDisplay::autoconnect()
{
        QString replyMsg;
        SysxIO *sysxIO = SysxIO::Instance();
        //this->connectButtonActive = value;
        if(!sysxIO->isConnected() && sysxIO->deviceReady())
        {
                emit setStatusSymbol(2);
                emit setStatusMessage(tr("Connecting"));

                this->connectButton->setBlink(true);
                sysxIO->setDeviceReady(false); // Reserve the device for interaction.

                QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)));
                QObject::connect(sysxIO, SIGNAL(sysxReply(QString)),
                        this, SLOT(autoConnectionResult(QString)));

                sysxIO->sendSysx(idRequestString); // GT-10 Identity Request.
        }
        else
        {
                emit notConnected();
                sysxIO->setNoError(true);		// Reset the error status (else we could never retry :) ).
        };
}

void floorBoardDisplay::autoConnectionResult(QString sysxMsg)
{
        SysxIO *sysxIO = SysxIO::Instance();
        QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)),
                        this, SLOT(autoConnectionResult(QString)));

        sysxIO->setDeviceReady(true); // Free the device after finishing interaction.
 if(sysxIO->noError())
        {
                if(sysxMsg.contains(idReplyPatern))
                {
                        this->connectButton->setBlink(false);
                        this->connectButton->setValue(true);
                        sysxIO->setConnected(true);
                        emit connectedSignal();

                        if(sysxIO->getBank() != 0)
                        {
                                this->writeButton->setBlink(false);
                                this->writeButton->setValue(false);
                        };
                }else
    {
     this->connectButton->setBlink(false);
                 this->connectButton->setValue(false);
                 sysxIO->setConnected(false);
    };
  }
  else
  {
   notConnected();
   sysxIO->setNoError(true);		// Reset the error status (else we could never retry :) ).
  };
}

void floorBoardDisplay::set_temp()
{

  SysxIO *sysxIO = SysxIO::Instance();
  QByteArray data;

  QFile file1("temp-1.syx");   // Read the sysx file .
  if (file1.open(QIODevice::ReadOnly))
        {	data = file1.readAll(); };

        QString sysxBuffer;
        for(int i=0;i<data.size();i++)
        {
                unsigned char byte = (char)data[i];
                unsigned int n = (int)byte;
                QString hex = QString::number(n, 16).toUpper();     // convert QByteArray to QString
                if (hex.length() < 2) hex.prepend("0");
                sysxBuffer.append(hex);
  };
        if( data.size() == fullPatchSize)
        {
         QString patchText;
         unsigned char r;
         unsigned int a = sysxNameOffset; // locate patch text start position from the start of the file
         for (int b=0;b<nameLength;b++)
           {
             r = (char)data[a+b];
             patchText.append(r);         // extract the text characters from the current patch name.
           };
   this->temp1Display->setMainText(patchText, Qt::AlignCenter);
   sysxIO->temp1_sysxMsg = sysxBuffer;
  };

  data.clear();
  QFile file2("temp-2.syx");   // Read the sysx file .
  if (file2.open(QIODevice::ReadOnly))
        {	data = file2.readAll(); };

        sysxBuffer.clear();
        for(int i=0;i<data.size();i++)
        {
                unsigned char byte = (char)data[i];
                unsigned int n = (int)byte;
                QString hex = QString::number(n, 16).toUpper();     // convert QByteArray to QString
                if (hex.length() < 2) hex.prepend("0");
                sysxBuffer.append(hex);
  };
        if( data.size() == fullPatchSize)
        {
         QString patchText;
         unsigned char r;
         unsigned int a = sysxNameOffset; // locate patch text start position from the start of the file
         for (int b=0;b<nameLength;b++)
           {
             r = (char)data[a+b];
             patchText.append(r);         // extract the text characters from the current patch name.
           };
   this->temp2Display->setMainText(patchText, Qt::AlignCenter);
   sysxIO->temp2_sysxMsg = sysxBuffer;
  };

        data.clear();
        QFile file3("temp-3.syx");   // Read the default GT-3 sysx file so we don't start empty handed.
  if (file3.open(QIODevice::ReadOnly))
        {	data = file3.readAll(); };


        sysxBuffer.clear();
        for(int i=0;i<data.size();i++)
        {
                unsigned char byte = (char)data[i];
                unsigned int n = (int)byte;
                QString hex = QString::number(n, 16).toUpper();     // convert QByteArray to QString
                if (hex.length() < 2) hex.prepend("0");
                sysxBuffer.append(hex);
  };
        if( data.size() == fullPatchSize)
        {
         QString patchText;
         unsigned char r;
         unsigned int a = sysxNameOffset; // locate patch text start position from the start of the file
         for (int b=0;b<nameLength;b++)
           {
             r = (char)data[a+b];
             patchText.append(r);         // extract the text characters from the current patch name.
           };
   this->temp3Display->setMainText(patchText, Qt::AlignCenter);
   sysxIO->temp3_sysxMsg = sysxBuffer;
  };

        data.clear();
        QFile file4("temp-4.syx");   // Read the sysx file .
  if (file4.open(QIODevice::ReadOnly))
        {	data = file4.readAll(); };

        sysxBuffer.clear();
        for(int i=0;i<data.size();i++)
        {
                unsigned char byte = (char)data[i];
                unsigned int n = (int)byte;
                QString hex = QString::number(n, 16).toUpper();     // convert QByteArray to QString
                if (hex.length() < 2) hex.prepend("0");
                sysxBuffer.append(hex);
  };
        if( data.size() == fullPatchSize)

        {
         QString patchText;
         unsigned char r;
         unsigned int a = sysxNameOffset; // locate patch text start position from the start of the file
         for (int b=0;b<nameLength;b++)
           {
             r = (char)data[a+b];
             patchText.append(r);         // extract the text characters from the current patch name.
           };
   this->temp4Display->setMainText(patchText, Qt::AlignCenter);
   sysxIO->temp4_sysxMsg = sysxBuffer;
  };

  data.clear();
        QFile file5("temp-5.syx");   // Read the sysx file .
  if (file5.open(QIODevice::ReadOnly))
        {	data = file5.readAll(); };

        sysxBuffer.clear();
        for(int i=0;i<data.size();i++)
        {
                unsigned char byte = (char)data[i];
                unsigned int n = (int)byte;
                QString hex = QString::number(n, 16).toUpper();     // convert QByteArray to QString
                if (hex.length() < 2) hex.prepend("0");
                sysxBuffer.append(hex);
  };
        if( data.size() == fullPatchSize)

        {
         QString patchText;
         unsigned char r;
         unsigned int a = sysxNameOffset; // locate patch text start position from the start of the file
         for (int b=0;b<nameLength;b++)
           {
             r = (char)data[a+b];
             patchText.append(r);         // extract the text characters from the current patch name.
           };
   this->temp5Display->setMainText(patchText, Qt::AlignCenter);
   sysxIO->temp5_sysxMsg = sysxBuffer;
  };
};

void floorBoardDisplay::temp1_copy(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();

  QString sysxMsg;
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        //QList<QString> patchAddress = sysxIO->getFileSource().address;
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();

        for(int i=0;i<patchData.size();++i)
        {
                QList<QString> data = patchData.at(i);
                for(int x=0;x<data.size();++x)
                {
                        QString hex;
                        if(x == sysxAddressOffset)
                        { hex = addr1; }
                        else if(x == sysxAddressOffset + 1)
                        {	hex = addr2; }
                        else
                        {	hex = data.at(x);	};
                        if (hex.length() < 2) hex.prepend("0");
                        sysxMsg.append(hex);
                };
        };
        if( sysxMsg.size()/2 == fullPatchSize)
        {
        this->patchName = sysxIO->getCurrentPatchName();
  this->temp1Display->setMainText(patchName, Qt::AlignCenter);
  sysxIO->temp1_sysxMsg = sysxMsg;
  save_temp("temp-1.syx", sysxMsg);
  } else {
  QApplication::beep();
  QString size = QString::number(sysxMsg.size()/2, 10);
  sysxIO->emitStatusdBugMessage(tr("in-consistant patch data detected ") + size + tr("bytes: re-save or re-load file to correct"));
  };
};

void floorBoardDisplay::temp1_paste(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();
  this->temp1_sysxMsg = sysxIO->temp1_sysxMsg;
  if (!temp1_sysxMsg.isEmpty() && sysxIO->deviceReady() )
  {
  SysxIO *sysxIO = SysxIO::Instance();
  sysxIO->setFileSource("patch", temp1_sysxMsg);
  emit updateSignal();
        sysxIO->writeToBuffer();
        }
  else
  {
   QApplication::beep();
   sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
  };
};

void floorBoardDisplay::temp2_copy(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();

  QString sysxMsg;
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        //QList<QString> patchAddress = sysxIO->getFileSource().address;
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();

        for(int i=0;i<patchData.size();++i)
        {
                QList<QString> data = patchData.at(i);
                for(int x=0;x<data.size();++x)
                {
                        QString hex;
                        if(x == sysxAddressOffset)
                        { hex = addr1; }
                        else if(x == sysxAddressOffset + 1)
                        {	hex = addr2; }
                        else
                        {	hex = data.at(x);	};
                        if (hex.length() < 2) hex.prepend("0");
                        sysxMsg.append(hex);
                };
        };
        if( sysxMsg.size()/2 == fullPatchSize)
        {
        this->patchName = sysxIO->getCurrentPatchName();
  this->temp2Display->setMainText(patchName, Qt::AlignCenter);
  sysxIO->temp2_sysxMsg = sysxMsg;
  save_temp("temp-2.syx", sysxMsg);
  } else {
  QApplication::beep();
  QString size = QString::number(sysxMsg.size()/2, 10);
  sysxIO->emitStatusdBugMessage(tr("in-consistant patch data detected ") + size + tr("bytes: re-save or re-load file to correct"));
  };
};

void floorBoardDisplay::temp2_paste(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();
  this->temp2_sysxMsg = sysxIO->temp2_sysxMsg;
  if (!temp2_sysxMsg.isEmpty() && sysxIO->deviceReady() )
  {
  SysxIO *sysxIO = SysxIO::Instance();
  sysxIO->setFileSource("patch", temp2_sysxMsg);
  emit updateSignal();
        sysxIO->writeToBuffer();
        }else
  {
   QApplication::beep();
   sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
  };
};

void floorBoardDisplay::temp3_copy(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();

  QString sysxMsg;
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        //QList<QString> patchAddress = sysxIO->getFileSource().address;
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();

        for(int i=0;i<patchData.size();++i)
        {
                QList<QString> data = patchData.at(i);
                for(int x=0;x<data.size();++x)
                {
                        QString hex;
                        if(x == sysxAddressOffset)
                        { hex = addr1; }
                        else if(x == sysxAddressOffset + 1)
                        {	hex = addr2; }
                        else
                        {	hex = data.at(x);	};
                        if (hex.length() < 2) hex.prepend("0");
                        sysxMsg.append(hex);
                };
        };
        if( sysxMsg.size()/2 == fullPatchSize)
        {
        this->patchName = sysxIO->getCurrentPatchName();
  this->temp3Display->setMainText(patchName, Qt::AlignCenter);
  sysxIO->temp3_sysxMsg = sysxMsg;
  save_temp("temp-3.syx", sysxMsg);
  } else {
  QApplication::beep();
  QString size = QString::number(sysxMsg.size()/2, 10);
  sysxIO->emitStatusdBugMessage(tr("in-consistant patch data detected ") + size + tr("bytes: re-save or re-load file to correct"));
  };
};

void floorBoardDisplay::temp3_paste(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();
  this->temp3_sysxMsg = sysxIO->temp3_sysxMsg;
  if (!temp3_sysxMsg.isEmpty() && sysxIO->deviceReady() )
  {
  SysxIO *sysxIO = SysxIO::Instance();
  sysxIO->setFileSource("patch", temp3_sysxMsg);
  emit updateSignal();
        sysxIO->writeToBuffer();
        }else
  {
   QApplication::beep();
   sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
  };
};

void floorBoardDisplay::temp4_copy(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();

  QString sysxMsg;
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        //QList<QString> patchAddress = sysxIO->getFileSource().address;
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();

        for(int i=0;i<patchData.size();++i)
        {
                QList<QString> data = patchData.at(i);
                for(int x=0;x<data.size();++x)
                {
                        QString hex;
                        if(x == sysxAddressOffset)
                        { hex = addr1; }
                        else if(x == sysxAddressOffset + 1)
                        {	hex = addr2; }
                        else
                        {	hex = data.at(x);	};
                        if (hex.length() < 2) hex.prepend("0");
                        sysxMsg.append(hex);
                };
        };
        if( sysxMsg.size()/2 == fullPatchSize)
        {
        this->patchName = sysxIO->getCurrentPatchName();
  this->temp4Display->setMainText(patchName, Qt::AlignCenter);
  sysxIO->temp4_sysxMsg = sysxMsg;
  save_temp("temp-4.syx", sysxMsg);
  } else {
  QApplication::beep();
  QString size = QString::number(sysxMsg.size()/2, 10);
  sysxIO->emitStatusdBugMessage(tr("in-consistant patch data detected ") + size + tr("bytes: re-save or re-load file to correct"));
  };
};

void floorBoardDisplay::temp4_paste(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();
  this->temp4_sysxMsg = sysxIO->temp4_sysxMsg;
  if (!temp4_sysxMsg.isEmpty() && sysxIO->deviceReady() )
  {
  SysxIO *sysxIO = SysxIO::Instance();
  sysxIO->setFileSource("patch", temp4_sysxMsg);
  emit updateSignal();
        sysxIO->writeToBuffer();
        }else
  {
   QApplication::beep();
   sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
  };
};

void floorBoardDisplay::temp5_copy(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();

  QString sysxMsg;
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        //QList<QString> patchAddress = sysxIO->getFileSource().address;
        QString addr1 = tempBulkWrite;  // temp address
        QString addr2 = QString::number(0, 16).toUpper();

        for(int i=0;i<patchData.size();++i)
        {
                QList<QString> data = patchData.at(i);
                for(int x=0;x<data.size();++x)
                {
                        QString hex;
                        if(x == sysxAddressOffset)
                        { hex = addr1; }
                        else if(x == sysxAddressOffset + 1)
                        {	hex = addr2; }
                        else
                        {	hex = data.at(x);	};
                        if (hex.length() < 2) hex.prepend("0");
                        sysxMsg.append(hex);
                };
        };
        if( sysxMsg.size()/2 == fullPatchSize)
        {
        this->patchName = sysxIO->getCurrentPatchName();
  this->temp5Display->setMainText(patchName, Qt::AlignCenter);
  sysxIO->temp5_sysxMsg = sysxMsg;
  save_temp("temp-5.syx", sysxMsg);
  } else {
  QApplication::beep();
  QString size = QString::number(sysxMsg.size()/2, 10);
  sysxIO->emitStatusdBugMessage(tr("in-consistant patch data detected ") + size + tr("bytes: re-save or re-load file to correct"));
  };
};

void floorBoardDisplay::temp5_paste(bool value)
{
  SysxIO *sysxIO = SysxIO::Instance();
  this->temp5_sysxMsg = sysxIO->temp5_sysxMsg;
  if (!temp5_sysxMsg.isEmpty() && sysxIO->deviceReady() )
  {
  SysxIO *sysxIO = SysxIO::Instance();
  sysxIO->setFileSource("patch", temp5_sysxMsg);
  emit updateSignal();
        sysxIO->writeToBuffer();
        }else
  {
   QApplication::beep();
   sysxIO->emitStatusdBugMessage(tr("patch must be copied to clipboard first"));
  };
};

void floorBoardDisplay::save_temp(QString fileName, QString sysxMsg)
 {
   QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
        {
                QByteArray out;
                unsigned int count=0;
                        QString data = sysxMsg;
                        int x = data.size()/2;
                        for (int a=0;a<x;++a)
                        {
                          QString str = data.at(a*2);
                                str.append(data.at((a*2)+1));
                                bool ok;
                                unsigned int n = str.toInt(&ok, 16);
                                out[count] = (char)n;
                                count++;
                        };
                file.write(out);
        };
 };

void floorBoardDisplay::connectSignal(bool value)
{
        QString replyMsg;
        SysxIO *sysxIO = SysxIO::Instance();
        this->connectButtonActive = value;
        if(connectButtonActive == true && sysxIO->deviceReady())
        {
                emit setStatusSymbol(2);
                emit setStatusMessage(tr("Connecting"));

                this->connectButton->setBlink(true);
                sysxIO->setDeviceReady(false); // Reserve the device for interaction.

                QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)));
                QObject::connect(sysxIO, SIGNAL(sysxReply(QString)),
                        this, SLOT(connectionResult(QString)));

                sysxIO->sendSysx(idRequestString); // GT10 Identity Request.
        }
        else
        {
                emit notConnected();
                sysxIO->setNoError(true);		// Reset the error status (else we could never retry :) ).
        };
}

void floorBoardDisplay::connectionResult(QString sysxMsg)
{
        SysxIO *sysxIO = SysxIO::Instance();
        QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)),
                        this, SLOT(connectionResult(QString)));

        sysxIO->setDeviceReady(true); // Free the device after finishing interaction.

                 //DeBugGING OUTPUT
        Preferences *preferences = Preferences::Instance(); // Load the preferences.
        if(preferences->getPreferences("Midi", "DBug", "bool")=="true")
        {
                        this->connectButton->setBlink(false);
                        this->connectButton->setValue(true);
                        sysxIO->setConnected(true);
                        emit connectedSignal();
                        emit setStatusMessage(tr("Ready"));

                        if(sysxIO->getBank() != 0)
                        {
                                this->writeButton->setBlink(true);
                                this->writeButton->setValue(false);
                        };
        }

        else if(sysxIO->noError())
        {
                if(sysxMsg.contains(idReplyPatern) && connectButtonActive == true)
                {
                        this->connectButton->setBlink(false);
                        this->connectButton->setValue(true);
                        sysxIO->setConnected(true);
                        emit connectedSignal();

                        if(sysxIO->getBank() != 0)
                        {
                                this->writeButton->setBlink(true);
                                this->writeButton->setValue(false);
                        };
                }
                else if(!sysxMsg.isEmpty())
                {
                        this->connectButton->setBlink(false);
                        this->connectButton->setValue(false);
                        sysxIO->setConnected(false);

                        QMessageBox *msgBox = new QMessageBox();
                        msgBox->setWindowTitle(deviceType + tr(" Fx FloorBoard connection Error !!"));
                        msgBox->setIcon(QMessageBox::Warning);
                        msgBox->setTextFormat(Qt::RichText);
                        QString msgText;
                        msgText.append("<font size='+1'><b>");
                        msgText.append(tr("The device connected is not a Boss ") + deviceType + tr(" Effects Processor."));
                        if (sysxMsg.contains(idRequestString))
                        {msgText.append(tr("<br>Midi loopback detected, ensure midi device 'thru' is switched off.")); };
                        msgText.append("<b></font>");
                        msgBox->setText(msgText);
                        msgBox->setStandardButtons(QMessageBox::Ok);
                        msgBox->exec();

                        notConnected();

                        emit setStatusSymbol(0);
                        emit setStatusProgress(0);
                        emit setStatusMessage(tr("Not connected"));
                }
                else
                {
                        this->connectButton->setBlink(false);
                        this->connectButton->setValue(false);
                        sysxIO->setConnected(false);

                        QMessageBox *msgBox = new QMessageBox();
                        msgBox->setWindowTitle(deviceType + tr(" Fx FloorBoard connection Error !!"));
                        msgBox->setIcon(QMessageBox::Warning);
                        msgBox->setTextFormat(Qt::RichText);
                        QString msgText;
                        msgText.append("<font size='+1'><b>");
                        msgText.append(tr("The Boss ") + deviceType + tr(" Effects Processor was not found."));
                        msgText.append(tr("<br><br>Ensure correct midi device is selected in Menu, "));
                        msgText.append(tr("<br>Boss drivers are installed and the GT-10 is switched on,"));
                        msgText.append(tr("<br>and GT-10 USB driver mode is set to advanced"));
                        msgText.append("<b></font><br>");
                        msgBox->setText(msgText);
                        msgBox->setStandardButtons(QMessageBox::Ok);
                        msgBox->exec();

                        notConnected();

                        emit setStatusSymbol(0);
                        emit setStatusProgress(0);
                        emit setStatusMessage(tr("Not connected"));
                };
        }
        else
        {
                notConnected();
                sysxIO->setNoError(true);		// Reset the error status (else we could never retry :) ).
        };
}

void floorBoardDisplay::writeSignal(bool)
{
        SysxIO *sysxIO = SysxIO::Instance();
        if(sysxIO->isConnected() && sysxIO->deviceReady()) /* Check if we are connected and if the device is free. */
        {
         this->writeButton->setBlink(true);

                if(sysxIO->getBank() == 0) // Check if a bank is sellected.
                {
                        sysxIO->setDeviceReady(false);			// Reserve the device for interaction.
                        writeToBuffer();
                }
                else // Bank is sellected.
                {
                        sysxIO->setDeviceReady(false);			// Reserve the device for interaction.
                        {
                                if((sysxIO->getBank() > bankTotalUser && sysxIO->getBank() <= bankTotalAll) || (sysxIO->getBank() == 105)) // Preset banks are NOT writable so we check.
                                {
                                        QMessageBox *msgBox = new QMessageBox();
                                        msgBox->setWindowTitle(deviceType + tr(" Fx FloorBoard"));
                                        msgBox->setIcon(QMessageBox::Warning);
                                        msgBox->setTextFormat(Qt::RichText);
                                        QString msgText;
                                        msgText.append("<font size='+1'><b>");
                                        msgText.append(tr("You can't write to the preset banks."));
                                        msgText.append("<b></font><br>");
                                        msgText.append(tr("Please select a user bank to write this patch to and try again."));
                                        msgBox->setText(msgText);
                                        msgBox->setStandardButtons(QMessageBox::Ok);
                                        msgBox->exec();
                                        this->writeButton->setBlink(false); // Allready sync with the buffer so no blinking
                                        this->writeButton->setValue(true);	// and so we will also leave the write button active.
                                        sysxIO->setDeviceReady(true);
                                }
                                else /* User bank so we can write to it after confirmation to overwrite stored data. */
                                {
                                  QString bankNum;
                                  QString patchNum;
                                  int bank = sysxIO->getBank();
                                  int patch = sysxIO->getPatch();
                                  bankNum.append(QString::number(bank, 10));
                                  if (bankNum.size() < 2){ bankNum.prepend("0"); };
                                  bankNum.prepend(" U");
                                  patchNum.append(QString::number(patch, 10));

                                        QMessageBox *msgBox = new QMessageBox();
                                        msgBox->setWindowTitle(deviceType + tr(" Fx FloorBoard"));
                                        msgBox->setIcon(QMessageBox::Warning);
                                        msgBox->setTextFormat(Qt::RichText);
                                        QString msgText;
                                        msgText.append("<font size='+1'><b>");
                                        msgText.append(tr("You have chosen to write the patch permanently into ") + deviceType + (" memory."));
                                        msgText.append("<b></font><br>");
                                        msgText.append(tr("This will overwrite the patch currently stored at patch location<br>"));
                                        msgText.append("<font size='+2'><b>");
                                        if (bank == 101){ msgText.append(("Quick Effects ") + patchNum); }
                                        else { msgText.append(bankNum +(":") +patchNum	); };
                                        msgText.append("<b></font><br>");
          msgText.append(tr (" and can't be undone. "));
                                        msgBox->setInformativeText(tr("Are you sure you want to continue?"));
                                        msgBox->setText(msgText);
                                        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);

                                        if(msgBox->exec() == QMessageBox::Yes)
                                        {	/* Accepted to overwrite data. So now we have to set destination address by replacing the default (buffer). */

                                                writeToMemory();
                                                sysxIO->setSyncStatus(true);
                                        }
                                        else if(sysxIO->isConnected())
                                        {
                                                sysxIO->setDeviceReady(true);
                                                this->writeButton->setBlink(false);
                                                //this->writeButton->setValue(true);
                                        };
                                };
                        };
                };
        }
};

void floorBoardDisplay::writeToBuffer()
{
        SysxIO *sysxIO = SysxIO::Instance();
        QMessageBox *msgBox = new QMessageBox();
    msgBox->setWindowTitle(deviceType + tr(" FloorBoard"));
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setTextFormat(Qt::RichText);
    QString msgText;
    msgText.append("<font size='+1'><b>");
    msgText.append(tr("You have not chosen a User Patch address to Write the patch to"));
    msgText.append("<b></font><br>");
    msgText.append(tr("This will only update the GT-10 Temporary Buffer memory<br>"));
    msgText.append(tr (" with the editor patch. "));
    msgBox->setInformativeText(tr("Select the required destination patch <br>by a single-click on the left panel Patch-Tree"));
    msgBox->setText(msgText);
    msgBox->setStandardButtons(QMessageBox::Close);
    msgBox->exec();

    sysxIO->writeToBuffer();
    sysxIO->setSyncStatus(true);
        this->writeButton->setBlink(false);	// Sync so we stop blinking the button
        this->writeButton->setValue(false);	// and activate the write button.
};

void floorBoardDisplay::writeToMemory()
{
        SysxIO *sysxIO = SysxIO::Instance();

        QString sysxMsg; bool ok;
        QList< QList<QString> > patchData = sysxIO->getFileSource().hex; // Get the loaded patch data.
        QList<QString> patchAddress = sysxIO->getFileSource().address;

        emit setStatusSymbol(2);
        emit setStatusMessage(tr("Writing to Patch"));

        int bank = sysxIO->getBank();
        int patch = sysxIO->getPatch();
        QString addr1;
        QString addr2;
        if (bank < 101)
        {
         int patchOffset = (((bank - 1 ) * patchPerBank) + patch) - 1;
         int memmorySize = QString("7F").toInt(&ok, 16) + 1;
         int emptyAddresses = (memmorySize) - ((bankTotalUser * patchPerBank) - (memmorySize));
         if(bank > bankTotalUser) patchOffset += emptyAddresses; //System patches start at a new memmory range.
     int addrMaxSize = QString("80").toInt(&ok, 16);
     int n = (int)(patchOffset / addrMaxSize);

        addr1 = QString::number(16 + n, 16).toUpper();
        addr2 = QString::number(patchOffset - (addrMaxSize * n), 16).toUpper();
         }
         else
         {
    addr1 = "30";
    addr2 = QString::number(patch - 1, 16).toUpper();
   };
   if (addr1.length() < 2) addr1.prepend("0");
         if (addr2.length() < 2) addr2.prepend("0");
        for(int i=0;i<patchData.size();++i)
        {
                QList<QString> data = patchData.at(i);
                for(int x=0;x<data.size();++x)
                {
                        QString hex;
                        if(x == sysxAddressOffset)
                        {
                                hex = addr1;
                        }
                        else if(x == sysxAddressOffset + 1)
                        {
                                hex = addr2;
                        }
                        else
                        {
                                hex = data.at(x);
                        };
                        if (hex.length() < 2) hex.prepend("0");
                        sysxMsg.append(hex);
                };
        };
        sysxIO->setSyncStatus(true);		// Still in sync
        this->writeButton->setBlink(false); // so no blinking here either...
        this->writeButton->setValue(false);	// ... and still the button will be active also ...

        QObject::connect(sysxIO, SIGNAL(sysxReply(QString)), this, SLOT(resetDevice(QString))); // Connect the result signal to a slot that will reset the device after sending.
        sysxIO->sendSysx(sysxMsg);								// Send the data.
        };

void floorBoardDisplay::patchChangeFailed()
{
        SysxIO *sysxIO = SysxIO::Instance();
        sysxIO->setBank(sysxIO->getLoadedBank());
        sysxIO->setPatch(sysxIO->getLoadedPatch());
        setPatchNumDisplay(sysxIO->getLoadedBank(), sysxIO->getLoadedPatch());
}

void floorBoardDisplay::resetDevice(QString replyMsg)
{
      SysxIO *sysxIO = SysxIO::Instance();
      QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)),	this, SLOT(resetDevice(QString)));

        if(sysxIO->getBank() != sysxIO->getLoadedBank() || sysxIO->getPatch() != sysxIO->getLoadedPatch())
          {
            sysxIO->setLoadedBank(sysxIO->getBank());
            sysxIO->setLoadedPatch(sysxIO->getPatch());
          };

    emit setStatusProgress(33); // time wasting sinusidal statusbar progress
    SLEEP(50);
    emit setStatusProgress(66);
    SLEEP(50);
    emit setStatusProgress(100);
    SLEEP(50);
    emit setStatusProgress(75);
    SLEEP(50);
    emit setStatusProgress(42);
    SLEEP(50);
    emit setStatusProgress(25);
    SLEEP(50);
    sysxIO->setDeviceReady(true);	// Free the device after finishing interaction.
    emit connectedSignal();			// Emit this signal to tell we are still connected and to update the patch names in case they have changed.
    sysxIO->setDeviceReady(true);
};

void floorBoardDisplay::patchSelectSignal(int bank, int patch)
{
        SysxIO *sysxIO = SysxIO::Instance();
        if(blinkCount == 0)
        {
                currentSyncStatus = sysxIO->getSyncStatus();
                sysxIO->setSyncStatus(false);
                writeButton->setBlink(false);
        };

        //if( sysxIO->getLoadedBank() != bank ||  sysxIO->getLoadedPatch() != patch)
        //{
                sysxIO->setBank(bank);
                sysxIO->setPatch(patch);

                if(blinkCount == 0)
                {
                        QObject::connect(timer, SIGNAL(timeout()), this, SLOT(blinkSellectedPatch()));
                        timer->start(sellectionBlinkInterval);
                }
                else
                {
                        blinkCount = 0;
                };
        //}
        //else
        //{
        //	blinkSellectedPatch(false);
        //};
}

void floorBoardDisplay::blinkSellectedPatch(bool active)
{
        SysxIO *sysxIO = SysxIO::Instance();
        int bank = sysxIO->getBank();
        int patch = sysxIO->getPatch();

        if(active && blinkCount <= (sellectionBlinks * 2) - 1)
        {
                if(blinkCount % 2 == 0)
                {
                        this->patchNumDisplay->clearAll();
                }
                else
                {
                        setPatchNumDisplay(bank, patch);
                };
                blinkCount++;
        }
        else
        {
                QObject::disconnect(timer, SIGNAL(timeout()), this, SLOT(blinkSellectedPatch()));
                timer->stop();
                blinkCount = 0;
                //sysxIO->setBank(sysxIO->getLoadedBank());
                //sysxIO->setPatch(sysxIO->getLoadedPatch());
                sysxIO->setSyncStatus(currentSyncStatus);
                if(currentSyncStatus || sysxIO->getLoadedBank() == 0)
                {
                        writeButton->setBlink(false);
                };
                setPatchNumDisplay(bank,patch);//(sysxIO->getLoadedBank(),  sysxIO->getLoadedPatch());
  };
        emit setStatusSymbol(1);
        emit setStatusMessage(tr("Ready"));
}

void floorBoardDisplay::patchLoadSignal(int bank, int patch)
{
        blinkSellectedPatch(false);

        SysxIO *sysxIO = SysxIO::Instance();
        sysxIO->setBank(bank);
        sysxIO->setPatch(patch);
}

void floorBoardDisplay::notConnected()
{
        this->connectButton->setBlink(false);
        this->connectButton->setValue(false);
        this->writeButton->setBlink(false);
        this->writeButton->setValue(false);

        SysxIO *sysxIO = SysxIO::Instance();
        sysxIO->setConnected(false);
        sysxIO->setSyncStatus(false);
        sysxIO->setDeviceReady(true);	// Free the device after finishing interaction.

        emit setStatusSymbol(0);
        emit setStatusProgress(0);
        emit setStatusMessage(tr("Not connected"));
}

void floorBoardDisplay::valueChanged(bool value, QString hex1, QString hex2, QString hex3)
{

};


