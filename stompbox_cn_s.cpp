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

#include "stompbox_cn_s.h"

stompbox_cn_s::stompbox_cn_s(QWidget *parent)
    : stompBox(parent)
{
	/* CHAIN SPLIT */
	this->setImage(":/images/cn_s.png");
	setLSB("01", "00");
	setComboBox("01", "00", "01", QRect(8, 78, 79, 13));
	//setComboBox2("01", "00", "02", QRect(8, 98, 79, 13));
	setSwitch("01", "00", "00");
	editDetails()->patchPos(304, 10, "01", "00"); 
	setEditPages();
};

void stompbox_cn_s::updateSignal()
{
	updateComboBox("01", "00", "01");
	//updateComboBox2("01", "00", "02");
	updateSwitch("01", "00", "00");
};

void stompbox_cn_s::setEditPages()
{
  editDetails()->page()->newGroupBox(tr("PreAmp Effect"));
	editDetails()->page()->addSwitch(0, 0, 1, 1, "01", "00", "00", "middle", Qt::AlignCenter);   // off/on effect
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	
  editDetails()->page()->newGroupBox(tr("Channel"));
	editDetails()->page()->newStackControl(0);
	editDetails()->page()->addComboBox(2, 0, 1, 1, "01", "00", "01");   //mode
	editDetails()->page()->addStackControl();
	editDetails()->page()->addGroupBox(1, 0, 1, 1);

	// TYPE SETTINGS 
	editDetails()->page()->insertStackField(0, 2, 0, 1, 1);

	editDetails()->page()->newStackField(0);
	editDetails()->page()->newGroupBox(tr("Channel select"));
	editDetails()->page()->addComboBox(1, 0, 1, 1, "01", "00", "02"); // channel select
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->addStackField();
	
	editDetails()->page()->newStackField(0, Qt::AlignCenter);
	editDetails()->page()->newGroupBox(tr("Channel Delay"));
	editDetails()->page()->addKnob(0, 0, 1, 1, "01", "00", "03");
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->addStackField();

	editDetails()->page()->newStackField(0, Qt::AlignCenter);
	editDetails()->page()->newGroupBox(tr("Channel Delay"));
	editDetails()->page()->addKnob(0, 0, 1, 1, "01", "00", "03");
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->addStackField();

	editDetails()->page()->newStackField(0, Qt::AlignCenter);
	editDetails()->page()->newGroupBox(tr("Dynamic switching"));
	editDetails()->page()->addKnob(0, 0, 1, 1, "01", "00", "04");
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->page()->addStackField();
    
	editDetails()->addPage();	
};
