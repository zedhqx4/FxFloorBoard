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

#include "stompbox_od.h"

stompbox_od::stompbox_od(QWidget *parent)
    : stompBox(parent)
{
	/* OD/DS */
	setImage(":/images/od.png");
	setLSB("00", "00");
	setKnob1("00", "00", "72");
	setKnob2("00", "00", "75");
	setComboBox("00", "00", "71");
	setButton("00", "00", "70");
	editDetails()->patchPos(246, 14, "00", "70");
	setEditPages();
};

void stompbox_od::updateSignal()
{
	updateKnob1("00", "00", "72");
	updateKnob2("00", "00", "75");
	updateComboBox("00", "00", "71");
	updateButton("00", "00", "70");
};

void stompbox_od::setEditPages()
{
    editDetails()->page()->newGroupBox("Effect");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "00", "00", "70", "middle", Qt::AlignCenter);
	editDetails()->page()->newStackControl(0);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "00", "00", "71");
	editDetails()->page()->addStackControl();
	editDetails()->page()->addGroupBox(0, 0, 1, 1);

	editDetails()->page()->newGroupBox("Overdrive");
	editDetails()->page()->addKnob(0, 0, 1, 1, "00", "00", "72", "turbo");
	editDetails()->page()->addKnob(0, 1, 1, 1, "00", "00", "73");
	editDetails()->page()->addKnob(0, 2, 1, 1, "00", "00", "74");
	editDetails()->page()->addGroupBox(0, 1, 1, 1);

	editDetails()->page()->newGroupBox("Level");
	editDetails()->page()->addKnob(0, 0, 1, 1, "00", "00", "75");
	editDetails()->page()->addKnob(0, 1, 1, 1, "00", "00", "76");
	editDetails()->page()->addGroupBox(0, 2, 1, 1);
	
	editDetails()->page()->newGroupBox("Solo");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "00", "00", "77", "middle", Qt::AlignCenter);
	editDetails()->page()->addKnob(0, 1, 1, 1, "00", "00", "78");
	editDetails()->page()->addGroupBox(0, 3, 1, 1);
	
	editDetails()->page()->insertStackField(0, 1, 1, 2, 2);
	
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();
	editDetails()->page()->newStackField(0);editDetails()->page()->addStackField();


		editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox("Custom");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "00", "00", "79");
	editDetails()->page()->addKnob(0, 1, 1, 1, "00", "00", "7A");
	editDetails()->page()->addKnob(0, 2, 1, 1, "00", "00", "7B");
	editDetails()->page()->addKnob(0, 3, 1, 1, "00", "00", "7C");
	editDetails()->page()->addKnob(0, 4, 1, 1, "00", "00", "7D");
	editDetails()->page()->addGroupBox(1, 0, 1, 4);
		editDetails()->page()->addStackField();
  
	editDetails()->addPage();	
};
