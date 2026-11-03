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

#include "stompbox_eq.h"

stompbox_eq::stompbox_eq(QWidget *parent)
    : stompBox(parent)
{
    /* EQ */
    setImage(":/images/eq.png");
    setLSB("01", "00");
    setSlider1("01", "00", "72");
    setSlider2("01", "00", "75");
    setSlider3("01", "00", "78");
    setSlider4("01", "00", "79");
    setSlider5("01", "00", "7B");
    setButton("01", "00", "70");
    editDetails()->patchPos(530, 24, "01", "70");
    setEditPages();
};

void stompbox_eq::updateSignal()
{
    updateSlider1("01", "00", "72");
    updateSlider2("01", "00", "75");
    updateSlider3("01", "00", "78");
    updateSlider4("01", "00", "79");
    updateSlider5("01", "00", "7B");
    updateButton("01", "00", "70");
};

void stompbox_eq::setEditPages()
{
    editDetails()->page()->newGroupBox("Effect", Qt::AlignTop | Qt::AlignHCenter);
    editDetails()->page()->addSwitch(0, 0, 1, 1, "01", "00", "70", "middle", Qt::AlignCenter);
    editDetails()->page()->addParaEQ(0, 1, 2, 4, "01", "00", "71");
    editDetails()->page()->addGroupBox(0, 0, 2, 4);
    editDetails()->addPage();
};
