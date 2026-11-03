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

#include "menuPage_autoriff.h"

menuPage_autoriff::menuPage_autoriff(QWidget *parent)
    : menuPage(parent)
{ 
  //setImage(":/images/autoriff_pushbutton.png");
  setLSB("0B", "00");
  setEditPages();
};

void menuPage_autoriff::updateSignal()
{

};                                 

void menuPage_autoriff::setEditPages()
{
       // editDetails()->page()->newGroupBox("FX1 AutoRiff User Scales");
        editDetails()->page()->addComboBox(0, 0, 1, 1, "03", "00", "5C");
        editDetails()->page()->addComboBox(0, 1, 1, 1, "03", "00", "5D");
        editDetails()->page()->addComboBox(0, 2, 1, 1, "03", "00", "5E");
        editDetails()->page()->addComboBox(0, 3, 1, 1, "03", "00", "5F");
        editDetails()->page()->addComboBox(0, 4, 1, 1, "03", "00", "60");
        editDetails()->page()->addComboBox(0, 5, 1, 1, "03", "00", "61");
        editDetails()->page()->addComboBox(0, 6, 1, 1, "03", "00", "62");
        editDetails()->page()->addComboBox(0, 7, 1, 1, "03", "00", "63");
        editDetails()->page()->addComboBox(0, 8, 1, 1, "03", "00", "64");
        editDetails()->page()->addComboBox(0, 9, 1, 1, "03", "00", "65");
        editDetails()->page()->addComboBox(0, 10, 1, 1, "03", "00", "66");
        editDetails()->page()->addComboBox(0, 11, 1, 1, "03", "00", "67");
        editDetails()->page()->addComboBox(0, 12, 1, 1, "03", "00", "68");
        editDetails()->page()->addComboBox(0, 13, 1, 1, "03", "00", "69");
        editDetails()->page()->addComboBox(0, 14, 1, 1, "03", "00", "6A");
        editDetails()->page()->addComboBox(0, 15, 1, 1, "03", "00", "6B");
        editDetails()->page()->addComboBox(1, 0, 1, 1, "03", "00", "6C");
	editDetails()->page()->addComboBox(1, 1, 1, 1, "03", "00", "6D");	
	editDetails()->page()->addComboBox(1, 2, 1, 1, "03", "00", "6E");	
	editDetails()->page()->addComboBox(1, 3, 1, 1, "03", "00", "6F");	
	editDetails()->page()->addComboBox(1, 4, 1, 1, "03", "00", "70");	
	editDetails()->page()->addComboBox(1, 5, 1, 1, "03", "00", "71");	
	editDetails()->page()->addComboBox(1, 6, 1, 1, "03", "00", "72");	
	editDetails()->page()->addComboBox(1, 7, 1, 1, "03", "00", "73");	
	editDetails()->page()->addComboBox(1, 8, 1, 1, "03", "00", "74");	
	editDetails()->page()->addComboBox(1, 9, 1, 1, "03", "00", "75");	
	editDetails()->page()->addComboBox(1, 10, 1, 1, "03", "00", "76");	
	editDetails()->page()->addComboBox(1, 11, 1, 1, "03", "00", "77");	
        editDetails()->page()->addComboBox(1, 12, 1, 1, "03", "00", "78");
        editDetails()->page()->addComboBox(1, 13, 1, 1, "03", "00", "79");
        editDetails()->page()->addComboBox(1, 14, 1, 1, "03", "00", "7A");
        editDetails()->page()->addComboBox(1, 15, 1, 1, "03", "00", "7B");
  editDetails()->page()->addComboBox(2, 0, 1, 1, "03", "00", "7C");
	editDetails()->page()->addComboBox(2, 1, 1, 1, "03", "00", "7D");	
	editDetails()->page()->addComboBox(2, 2, 1, 1, "03", "00", "7E");	
	editDetails()->page()->addComboBox(2, 3, 1, 1, "03", "00", "7F");	
	editDetails()->page()->addComboBox(2, 4, 1, 1, "04", "00", "00");	
	editDetails()->page()->addComboBox(2, 5, 1, 1, "04", "00", "01");	
	editDetails()->page()->addComboBox(2, 6, 1, 1, "04", "00", "02");	
	editDetails()->page()->addComboBox(2, 7, 1, 1, "04", "00", "03");	
	editDetails()->page()->addComboBox(2, 8, 1, 1, "04", "00", "04");	
	editDetails()->page()->addComboBox(2, 9, 1, 1, "04", "00", "05");	
	editDetails()->page()->addComboBox(2, 10, 1, 1, "04", "00", "06");	
	editDetails()->page()->addComboBox(2, 11, 1, 1, "04", "00", "07");	
	editDetails()->page()->addComboBox(2, 12, 1, 1, "04", "00", "08");	
	editDetails()->page()->addComboBox(2, 13, 1, 1, "04", "00", "09");	
	editDetails()->page()->addComboBox(2, 14, 1, 1, "04", "00", "0A");	
	editDetails()->page()->addComboBox(2, 15, 1, 1, "04", "00", "0B");
  editDetails()->page()->addComboBox(3, 0, 1, 1, "04", "00", "0C");	
	editDetails()->page()->addComboBox(3, 1, 1, 1, "04", "00", "0D");	
	editDetails()->page()->addComboBox(3, 2, 1, 1, "04", "00", "0E");	
	editDetails()->page()->addComboBox(3, 3, 1, 1, "04", "00", "0F");	
	editDetails()->page()->addComboBox(3, 4, 1, 1, "04", "00", "10");	
	editDetails()->page()->addComboBox(3, 5, 1, 1, "04", "00", "11");	
	editDetails()->page()->addComboBox(3, 6, 1, 1, "04", "00", "12");	
	editDetails()->page()->addComboBox(3, 7, 1, 1, "04", "00", "13");	
	editDetails()->page()->addComboBox(3, 8, 1, 1, "04", "00", "14");	
	editDetails()->page()->addComboBox(3, 9, 1, 1, "04", "00", "15");	
	editDetails()->page()->addComboBox(3, 10, 1, 1, "04", "00", "16");	
	editDetails()->page()->addComboBox(3, 11, 1, 1, "04", "00", "17");	
	editDetails()->page()->addComboBox(3, 12, 1, 1, "04", "00", "18");	
	editDetails()->page()->addComboBox(3, 13, 1, 1, "04", "00", "19");	
	editDetails()->page()->addComboBox(3, 14, 1, 1, "04", "00", "1A");	
	editDetails()->page()->addComboBox(3, 15, 1, 1, "04", "00", "1B");
  editDetails()->page()->addComboBox(4, 0, 1, 1, "04", "00", "1C");	
	editDetails()->page()->addComboBox(4, 1, 1, 1, "04", "00", "1D");	
	editDetails()->page()->addComboBox(4, 2, 1, 1, "04", "00", "1E");	
	editDetails()->page()->addComboBox(4, 3, 1, 1, "04", "00", "1F");	
	editDetails()->page()->addComboBox(4, 4, 1, 1, "04", "00", "20");	
	editDetails()->page()->addComboBox(4, 5, 1, 1, "04", "00", "21");	
	editDetails()->page()->addComboBox(4, 6, 1, 1, "04", "00", "22");	
	editDetails()->page()->addComboBox(4, 7, 1, 1, "04", "00", "23");	
	editDetails()->page()->addComboBox(4, 8, 1, 1, "04", "00", "24");	
	editDetails()->page()->addComboBox(4, 9, 1, 1, "04", "00", "25");	
	editDetails()->page()->addComboBox(4, 10, 1, 1, "04", "00", "26");	
	editDetails()->page()->addComboBox(4, 11, 1, 1, "04", "00", "27");	
	editDetails()->page()->addComboBox(4, 12, 1, 1, "04", "00", "28");	
	editDetails()->page()->addComboBox(4, 13, 1, 1, "04", "00", "29");	
	editDetails()->page()->addComboBox(4, 14, 1, 1, "04", "00", "2A");	
	editDetails()->page()->addComboBox(4, 15, 1, 1, "04", "00", "2B");
  editDetails()->page()->addComboBox(5, 0, 1, 1, "04", "00", "2C");	
	editDetails()->page()->addComboBox(5, 1, 1, 1, "04", "00", "2D");	
	editDetails()->page()->addComboBox(5, 2, 1, 1, "04", "00", "2E");	
	editDetails()->page()->addComboBox(5, 3, 1, 1, "04", "00", "2F");	
	editDetails()->page()->addComboBox(5, 4, 1, 1, "04", "00", "30");	
	editDetails()->page()->addComboBox(5, 5, 1, 1, "04", "00", "31");	
	editDetails()->page()->addComboBox(5, 6, 1, 1, "04", "00", "32");	
	editDetails()->page()->addComboBox(5, 7, 1, 1, "04", "00", "33");	
	editDetails()->page()->addComboBox(5, 8, 1, 1, "04", "00", "34");	
	editDetails()->page()->addComboBox(5, 9, 1, 1, "04", "00", "35");	
	editDetails()->page()->addComboBox(5, 10, 1, 1, "04", "00", "36");	
	editDetails()->page()->addComboBox(5, 11, 1, 1, "04", "00", "37");	
	editDetails()->page()->addComboBox(5, 12, 1, 1, "04", "00", "38");	
	editDetails()->page()->addComboBox(5, 13, 1, 1, "04", "00", "39");	
	editDetails()->page()->addComboBox(5, 14, 1, 1, "04", "00", "3A");	
	editDetails()->page()->addComboBox(5, 15, 1, 1, "04", "00", "3B");
  editDetails()->page()->addComboBox(6, 0, 1, 1, "04", "00", "3C");	
	editDetails()->page()->addComboBox(6, 1, 1, 1, "04", "00", "3D");	
	editDetails()->page()->addComboBox(6, 2, 1, 1, "04", "00", "3E");	
	editDetails()->page()->addComboBox(6, 3, 1, 1, "04", "00", "3F");	
	editDetails()->page()->addComboBox(6, 4, 1, 1, "04", "00", "40");	
	editDetails()->page()->addComboBox(6, 5, 1, 1, "04", "00", "41");	
	editDetails()->page()->addComboBox(6, 6, 1, 1, "04", "00", "42");	
	editDetails()->page()->addComboBox(6, 7, 1, 1, "04", "00", "43");	
	editDetails()->page()->addComboBox(6, 8, 1, 1, "04", "00", "44");	
	editDetails()->page()->addComboBox(6, 9, 1, 1, "04", "00", "45");	
	editDetails()->page()->addComboBox(6, 10, 1, 1, "04", "00", "46");	
	editDetails()->page()->addComboBox(6, 11, 1, 1, "04", "00", "47");	
	editDetails()->page()->addComboBox(6, 12, 1, 1, "04", "00", "48");	
	editDetails()->page()->addComboBox(6, 13, 1, 1, "04", "00", "49");	
	editDetails()->page()->addComboBox(6, 14, 1, 1, "04", "00", "4A");	
	editDetails()->page()->addComboBox(6, 15, 1, 1, "04", "00", "4B");
  editDetails()->page()->addComboBox(7, 0, 1, 1, "04", "00", "4C");	
	editDetails()->page()->addComboBox(7, 1, 1, 1, "04", "00", "4D");	
	editDetails()->page()->addComboBox(7, 2, 1, 1, "04", "00", "4E");	
	editDetails()->page()->addComboBox(7, 3, 1, 1, "04", "00", "4F");	
	editDetails()->page()->addComboBox(7, 4, 1, 1, "04", "00", "50");	
	editDetails()->page()->addComboBox(7, 5, 1, 1, "04", "00", "51");	
	editDetails()->page()->addComboBox(7, 6, 1, 1, "04", "00", "52");	
	editDetails()->page()->addComboBox(7, 7, 1, 1, "04", "00", "53");	
	editDetails()->page()->addComboBox(7, 8, 1, 1, "04", "00", "54");	
	editDetails()->page()->addComboBox(7, 9, 1, 1, "04", "00", "55");	
	editDetails()->page()->addComboBox(7, 10, 1, 1, "04", "00", "56");	
	editDetails()->page()->addComboBox(7, 11, 1, 1, "04", "00", "57");	
	editDetails()->page()->addComboBox(7, 12, 1, 1, "04", "00", "58");	
	editDetails()->page()->addComboBox(7, 13, 1, 1, "04", "00", "59");	
	editDetails()->page()->addComboBox(7, 14, 1, 1, "04", "00", "5A");	
	editDetails()->page()->addComboBox(7, 15, 1, 1, "04", "00", "5B");	
  editDetails()->page()->addComboBox(8, 0, 1, 1, "04", "00", "5C");	
	editDetails()->page()->addComboBox(8, 1, 1, 1, "04", "00", "5D");	
	editDetails()->page()->addComboBox(8, 2, 1, 1, "04", "00", "5E");	
	editDetails()->page()->addComboBox(8, 3, 1, 1, "04", "00", "5F");	
	editDetails()->page()->addComboBox(8, 4, 1, 1, "04", "00", "60");	
	editDetails()->page()->addComboBox(8, 5, 1, 1, "04", "00", "61");	
	editDetails()->page()->addComboBox(8, 6, 1, 1, "04", "00", "62");	
	editDetails()->page()->addComboBox(8, 7, 1, 1, "04", "00", "63");	
	editDetails()->page()->addComboBox(8, 8, 1, 1, "04", "00", "64");	
	editDetails()->page()->addComboBox(8, 9, 1, 1, "04", "00", "65");	
	editDetails()->page()->addComboBox(8, 10, 1, 1, "04", "00", "66");	
	editDetails()->page()->addComboBox(8, 11, 1, 1, "04", "00", "67");	
	editDetails()->page()->addComboBox(8, 12, 1, 1, "04", "00", "68");	
	editDetails()->page()->addComboBox(8, 13, 1, 1, "04", "00", "69");	
	editDetails()->page()->addComboBox(8, 14, 1, 1, "04", "00", "6A");	
	editDetails()->page()->addComboBox(8, 15, 1, 1, "04", "00", "6B");	
  editDetails()->page()->addComboBox(9, 0, 1, 1, "04", "00", "6C");	
	editDetails()->page()->addComboBox(9, 1, 1, 1, "04", "00", "6D");	
	editDetails()->page()->addComboBox(9, 2, 1, 1, "04", "00", "6E");	
	editDetails()->page()->addComboBox(9, 3, 1, 1, "04", "00", "6F");	
	editDetails()->page()->addComboBox(9, 4, 1, 1, "04", "00", "70");	
	editDetails()->page()->addComboBox(9, 5, 1, 1, "04", "00", "71");	
	editDetails()->page()->addComboBox(9, 6, 1, 1, "04", "00", "72");	
	editDetails()->page()->addComboBox(9, 7, 1, 1, "04", "00", "73");	
	editDetails()->page()->addComboBox(9, 8, 1, 1, "04", "00", "74");	
	editDetails()->page()->addComboBox(9, 9, 1, 1, "04", "00", "75");	
	editDetails()->page()->addComboBox(9, 10, 1, 1, "04", "00", "76");	
	editDetails()->page()->addComboBox(9, 11, 1, 1, "04", "00", "77");	
	editDetails()->page()->addComboBox(9, 12, 1, 1, "04", "00", "78");	
	editDetails()->page()->addComboBox(9, 13, 1, 1, "04", "00", "79");	
	editDetails()->page()->addComboBox(9, 14, 1, 1, "04", "00", "7A");	
	editDetails()->page()->addComboBox(9, 15, 1, 1, "04", "00", "7B");	
  editDetails()->page()->addComboBox(10, 0, 1, 1, "04", "00", "7C");	
	editDetails()->page()->addComboBox(10, 1, 1, 1, "04", "00", "7D");	
	editDetails()->page()->addComboBox(10, 2, 1, 1, "04", "00", "7E");	
	editDetails()->page()->addComboBox(10, 3, 1, 1, "04", "00", "7F");	
	editDetails()->page()->addComboBox(10, 4, 1, 1, "05", "00", "00");	
	editDetails()->page()->addComboBox(10, 5, 1, 1, "05", "00", "01");	
	editDetails()->page()->addComboBox(10, 6, 1, 1, "05", "00", "02");	
	editDetails()->page()->addComboBox(10, 7, 1, 1, "05", "00", "03");	
	editDetails()->page()->addComboBox(10, 8, 1, 1, "05", "00", "04");	
	editDetails()->page()->addComboBox(10, 9, 1, 1, "05", "00", "05");	
	editDetails()->page()->addComboBox(10, 10, 1, 1, "05", "00", "06");	
	editDetails()->page()->addComboBox(10, 11, 1, 1, "05", "00", "07");	
	editDetails()->page()->addComboBox(10, 12, 1, 1, "05", "00", "08");	
	editDetails()->page()->addComboBox(10, 13, 1, 1, "05", "00", "09");	
	editDetails()->page()->addComboBox(10, 14, 1, 1, "05", "00", "0A");	
	editDetails()->page()->addComboBox(10, 15, 1, 1, "05", "00", "0B");	
  editDetails()->page()->addComboBox(11, 0, 1, 1, "05", "00", "0C");	
	editDetails()->page()->addComboBox(11, 1, 1, 1, "05", "00", "0D");	
	editDetails()->page()->addComboBox(11, 2, 1, 1, "05", "00", "0E");	
	editDetails()->page()->addComboBox(11, 3, 1, 1, "05", "00", "0F");	
	editDetails()->page()->addComboBox(11, 4, 1, 1, "05", "00", "10");	
	editDetails()->page()->addComboBox(11, 5, 1, 1, "05", "00", "11");	
	editDetails()->page()->addComboBox(11, 6, 1, 1, "05", "00", "12");	
	editDetails()->page()->addComboBox(11, 7, 1, 1, "05", "00", "13");	
	editDetails()->page()->addComboBox(11, 8, 1, 1, "05", "00", "14");	
	editDetails()->page()->addComboBox(11, 9, 1, 1, "05", "00", "15");	
	editDetails()->page()->addComboBox(11, 10, 1, 1, "05", "00", "16");	
	editDetails()->page()->addComboBox(11, 11, 1, 1, "05", "00", "17");	
	editDetails()->page()->addComboBox(11, 12, 1, 1, "05", "00", "18");	
	editDetails()->page()->addComboBox(11, 13, 1, 1, "05", "00", "19");	
	editDetails()->page()->addComboBox(11, 14, 1, 1, "05", "00", "1A");	
  editDetails()->page()->addComboBox(11, 15, 1, 1, "05", "00", "1B");	
  //editDetails()->page()->addGroupBox(0, 0, 1, 1);
	editDetails()->addPage();

};
