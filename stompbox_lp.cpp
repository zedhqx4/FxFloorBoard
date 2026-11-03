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

#include "stompbox_lp.h"

stompbox_lp::stompbox_lp(QWidget *parent)
    : stompBox(parent)
{
	/* LOOP */
	setImage(":/images/lp.png");
	setLSB("0A", "00");
	setKnob1("0A", "00", "7B");
	setKnob2("0A", "00", "7C");
	setComboBox("0A", "00", "7A");
	setButton("0A", "00", "79");
	editDetails()->patchPos(2944, 8, "0A", "79");
	setEditPages();
};

void stompbox_lp::updateSignal()
{
	updateKnob1("0A", "00", "7B");
	updateKnob2("0A", "00", "7C");
	updateComboBox("0A", "00", "7A");
	updateButton("0A", "00", "79");
};

void stompbox_lp::setEditPages()
{
  editDetails()->page()->newGroupBox("Effect");
	editDetails()->page()->addSwitch(0, 0, 1, 1, "0A", "00", "79", "middle", Qt::AlignCenter);
	editDetails()->page()->addComboBox(1, 0, 1, 1, "0A", "00", "7A");
	editDetails()->page()->addGroupBox(0, 0, 1, 1);

	editDetails()->page()->newGroupBox("Level");
	editDetails()->page()->addKnob(0, 0, 1, 1, "0A", "00", "7B");
	editDetails()->page()->addKnob(0, 1, 1, 1, "0A", "00", "7C");
	editDetails()->page()->addGroupBox(0, 1, 1, 1);

	editDetails()->addPage();	
};
