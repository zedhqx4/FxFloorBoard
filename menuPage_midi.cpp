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

#include "menuPage_midi.h"

menuPage_midi::menuPage_midi(QWidget *parent)
    : menuPage(parent)
{
  //setImage(":/images/midi_pushbutton.png");
  setLSB("0B", "00");
	setEditPages();
};

void menuPage_midi::updateSignal()
{

};

void menuPage_midi::setEditPages()
{
               
	editDetails()->page()->newGroupBox("SYSTEM MIDI.  Caution: Changes are automatically written to the GT-10.");
	editDetails()->page()->addComboBox(0, 0, 1, 1, "02", "00", "00", "System"); // omni mode
	editDetails()->page()->addComboBox(0, 1, 1, 1, "02", "00", "01", "System"); // rx channel
	editDetails()->page()->addComboBox(0, 2, 1, 1, "02", "00", "02", "System"); // tx channel
	editDetails()->page()->addComboBox(0, 3, 1, 1, "02", "00", "04", "System"); // sync clock
	editDetails()->page()->addComboBox(0, 4, 1, 1, "02", "00", "05", "System"); // pc out
	
	
	editDetails()->page()->addComboBox(1, 0, 1, 1, "02", "00", "06", "System"); // exp1 out
	editDetails()->page()->addComboBox(1, 1, 1, 1, "02", "00", "07", "System"); // exp sw out
	editDetails()->page()->addComboBox(1, 2, 1, 1, "02", "00", "08", "System"); // ctl 1 out
	editDetails()->page()->addComboBox(1, 3, 1, 1, "02", "00", "09", "System"); // ctl 2 out
	editDetails()->page()->addComboBox(2, 0, 1, 1, "02", "00", "0A", "System"); // exp2 out
	editDetails()->page()->addComboBox(2, 1, 1, 1, "02", "00", "0B", "System"); // ctl 3 out
	editDetails()->page()->addComboBox(2, 2, 1, 1, "02", "00", "0C", "System"); // ctl 4 out
	editDetails()->page()->addComboBox(2, 3, 1, 1, "02", "00", "0D", "System"); // map select
	
	editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->addPage();
                          
 
};
