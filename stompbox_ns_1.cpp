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

#include "stompbox_ns_1.h"

stompbox_ns_1::stompbox_ns_1(QWidget *parent)
    : stompBox(parent)
{
	/* NS_1 */
	setImage(":/images/ns_1.png");
	setLSB("0A", "00");
	setKnob1("0A", "00", "72");
	setKnob2("0A", "00", "73");
	setButton("0A", "00", "71");
	editDetails()->patchPos(2928, 8, "0A", "71");
	setEditPages();
};

void stompbox_ns_1::updateSignal()
{
  updateKnob1("0A", "00", "72");
	updateKnob2("0A", "00", "73");
	updateButton("0A", "00", "71");
};

void stompbox_ns_1::setEditPages()
{
  editDetails()->page()->newGroupBox("Effect");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "0A", "00", "71", "middle", Qt::AlignCenter);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "0A", "00", "74");
	editDetails()->page()->addGroupBox(0, 0, 1, 1);

	editDetails()->page()->newGroupBox("Noise Suppressor");
	editDetails()->page()->addKnob(0, 0, 1, 1, "0A", "00", "72");
	editDetails()->page()->addKnob(0, 1, 1, 1, "0A", "00", "73");
	editDetails()->page()->addGroupBox(0, 1, 1, 1);
	editDetails()->addPage();	
	 
};
