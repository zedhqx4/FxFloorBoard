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

#include <QLayout>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include "bankTreeList.h"
#include "Preferences.h"
#include "MidiTable.h"
#include "SysxIO.h"
#include "globalVariables.h"

bankTreeList::bankTreeList(QWidget *parent)
    : QWidget(parent)
{
        QFont font;
        font.setStretch(85);

        this->treeList = newTreeList();
        this->treeList->setObjectName("banklist");
  QObject::connect(treeList, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(setOpenItems(QTreeWidgetItem*)));

  QObject::connect(treeList, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(setClosedItems(QTreeWidgetItem*)));

  QObject::connect(treeList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(setItemClicked(QTreeWidgetItem*, int)));

  QObject::connect(treeList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(setItemDoubleClicked(QTreeWidgetItem*, int)));

        QObject::connect(this, SIGNAL(updateSignal()), this->parent(), SIGNAL(updateSignal()));

        QVBoxLayout *treeListLayout = new QVBoxLayout;
        treeListLayout->addWidget(treeList);
        treeListLayout->setMargin(0);
        treeListLayout->setSpacing(0);
        setLayout(treeListLayout);

        SysxIO *sysxIO = SysxIO::Instance();
        QObject::connect(this, SIGNAL(setStatusSymbol(int)), sysxIO, SIGNAL(setStatusSymbol(int)));

        QObject::connect(this, SIGNAL(setStatusProgress(int)), sysxIO, SIGNAL(setStatusProgress(int)));

        QObject::connect(this, SIGNAL(setStatusMessage(QString)), sysxIO, SIGNAL(setStatusMessage(QString)));

        QObject::connect(this, SIGNAL(notConnectedSignal()), sysxIO, SIGNAL(notConnectedSignal()));
};

void bankTreeList::updateSize(QRect newrect)
{
        this->setGeometry(newrect);
};

void bankTreeList::setClosedItems(QTreeWidgetItem *item)
{
        if(item->childCount() == patchPerBank)
        {
                int index = openPatchTreeItems.indexOf(item);
                openPatchTreeItems.removeAt(index);
        }
        else if(item->childCount() == 5)
        {
                int index = openBankTreeItems.indexOf(item);
                openBankTreeItems.removeAt(index);
                closeChildren(item);
        }
        else
        {
                closeChildren(item);
        };
};

void bankTreeList::closeChildren(QTreeWidgetItem *item)
{
        for(int i=0;i<item->childCount();++i)
        {
                for(int n=0;n<item->child(i)->childCount();++n)
                {
                        item->child(i)->child(n)->setExpanded(false);
                };
                item->child(i)->setExpanded(false);
        };
};

void bankTreeList::setOpenItems(QTreeWidgetItem *item)
{
        QString type = "root";
        if(item->childCount() == patchPerBank)
        {
                openPatchTreeItems.append(item);
                updateTree(item);
                type = "patch";
        }
        else if(item->childCount() == 5)
        {
                openBankTreeItems.append(item);
                type = "bank";
        };

        int c = openPatchTreeItems.size();
        int b = openBankTreeItems.size();
        int a = 0;
        if(treeList->isExpanded(treeList->model()->index(1, 0)) &&
                treeList->isExpanded(treeList->model()->index(2, 0)))
        {
                a = 2;
        }
        else if(treeList->isExpanded(treeList->model()->index(1, 0)) ||
                treeList->isExpanded(treeList->model()->index(2, 0)))
        {
                a = 1;
        };

        int userBankCount = 0;
        for(int i=0;i<openBankTreeItems.size();++i)
        {
                if(openBankTreeItems.at(i)->parent()->text(0).contains("User"))
                {
                        userBankCount++;
                };
        };


        if(type == "root")
        {
                if(a > 1)
                {
                        int maxExpandedItems = 1;
                        while(c > 3)
                        {
                                openPatchTreeItems.first()->setExpanded(false);
                                c = openPatchTreeItems.size();
                        };
                        switch(c)
                        {
                                case 0: maxExpandedItems = 3; break;
                                case 1: maxExpandedItems = 2; break;
                                case 2: maxExpandedItems = 1; break;
                                case 3: maxExpandedItems = 1; break;
                        };

                        while(b > maxExpandedItems)
                        {
                                openBankTreeItems.first()->setExpanded(false);;
                                b = openBankTreeItems.size();
                        };
                };
        }
        else if(type == "bank")
        {
                int maxExpandedItems = 1;
                if(a > 1)
                {
                        switch(c)
                        {
                                case 0: maxExpandedItems = 3; break;
                                case 1: maxExpandedItems = 2; break;
                                case 2: maxExpandedItems = 1; break;
                                case 3: maxExpandedItems = 1; break;
                        };
                }
                else
                {
                        if(treeList->isExpanded(treeList->model()->index(0, 0)))
                        {
                                switch(c)
                                {
                                        case 0: maxExpandedItems = 5; break;
                                        case 1: maxExpandedItems = 4; break;
                                        case 2: maxExpandedItems = 3; break;
                                        case 3: maxExpandedItems = 3; break;
                                        case 4: maxExpandedItems = 2; break;
                                        case 5: maxExpandedItems = 1; break;
                                };
                        }
                        else
                        {
                                switch(c)
                                {
                                        case 0: maxExpandedItems = 4; break;
                                        case 1: maxExpandedItems = 4; break;
                                        case 2: maxExpandedItems = 3; break;
                                        case 3: maxExpandedItems = 3; break;
                                        case 4: maxExpandedItems = 1; break;
                                        case 5: maxExpandedItems = 1; break;
                                };
                        };
                };

                if(maxExpandedItems == 1)
                {
                        openPatchTreeItems.first()->setExpanded(false);
                        if(treeList->isExpanded(treeList->model()->index(1, 0)))
                        {
                                openPatchTreeItems.first()->setExpanded(false);
                        }
                        maxExpandedItems = 2;
                };
                if(b > maxExpandedItems)
                {
                        openBankTreeItems.first()->setExpanded(false);;
                };
        }
        else if(type == "patch")
        {
                for(int i=0;i<openBankTreeItems.size();++i)
                {
                        bool allCollapsed = true;
                        for(int n=0;n<openBankTreeItems.at(i)->childCount();++n)
                        {
                                if(openBankTreeItems.at(i)->child(n)->isExpanded())
                                {
                                        allCollapsed = false;
                                };
                        };
                        if(allCollapsed)
                        {
                                openBankTreeItems.at(i)->setExpanded(false);
                                b = openBankTreeItems.size();
                                break;
                        };
                };

                int maxExpandedItems = 1;
                if(a > 1)
                {
                        switch(b)
                        {
                                case 1: maxExpandedItems = 3; break;
                                case 2: maxExpandedItems = 1; break;
                                case 3: maxExpandedItems = 0; break;
                        };
                }
                else
                {
                        if(treeList->isExpanded(treeList->model()->index(0, 0)))
                        {
                                switch(b)
                                {
                                        case 1: maxExpandedItems = 5; break;
                                        case 2: maxExpandedItems = 4; break;
                                        case 3: maxExpandedItems = 3; break;
                                        case 4: maxExpandedItems = 1; break;
                                };
                        }
                        else
                        {
                                switch(b)
                                {
                                        case 1: maxExpandedItems = 4; break;
                                        case 2: maxExpandedItems = 3; break;
                                        case 3: maxExpandedItems = 3; break;
                                        case 4: maxExpandedItems = 1; break;
                                };
                        };
                };

                if(maxExpandedItems == 0)
                {
                        openBankTreeItems.first()->setExpanded(false);
                        maxExpandedItems = 1;
                };
                if(c > maxExpandedItems)
                {
                        openPatchTreeItems.first()->setExpanded(false);
                };
        };
};

QTreeWidget* bankTreeList::newTreeList()
{
        QTreeWidget *newTreeList = new QTreeWidget();
        newTreeList->setColumnCount(1);
        newTreeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        newTreeList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        QStringList headers;
        headers << QObject::tr("Double-click tree item to load patch");
        newTreeList->setHeaderLabels(headers);

  QTreeWidgetItem *temp = new QTreeWidgetItem(newTreeList);
        temp->setText(0, "Temp");
        temp->setWhatsThis(0, tr("Temporary Buffer.<br>a single mouse click will set the Write/Sync button to send to the buffer only,<br>a double click will load the current GT patch data."));

        QTreeWidgetItem *user = new QTreeWidgetItem(newTreeList);
        user->setText(0, "User");
        user->setWhatsThis(0, tr("User Banks.<br>expand the Bank to view a section of Banks."));
        //user->setIcon(...);

    QList<QTreeWidgetItem *> userBankRanges;
    for (int a=1; a<=bankTotalUser; a++)
        {
                QTreeWidgetItem* bankRange = new QTreeWidgetItem; // don't pass a parent here!
                bankRange->setText(0, QString("Bank U").append(QString::number(a, 10)).append("-U").append(QString::number(a+4, 10)) );
                bankRange->setWhatsThis(0, tr("User Banks.<br>expand the Bank to view a section of Patch Banks"));

                for (int b=a; b<=(a+4); b++)
                                {
                        QTreeWidgetItem* bank = new QTreeWidgetItem(bankRange);
                        bank->setText(0, QString("Bank ").append(QString::number(b, 10)));
                        bank->setWhatsThis(0, tr("User Bank.<br>expand the Bank to view the Patches"));
                        //bank->setIcon(...);

                        for (int c=1; c<=4; c++)
                        {
                                QTreeWidgetItem* patch = new QTreeWidgetItem(bank);
                                patch->setText(0, QString("Patch ").append(QString::number(c, 10)));
                                patch->setWhatsThis(0, tr("User Patches.<br>a single mouse click will only change patch<br>a double mouse click will load the select patch from the GT."));
                        };
                };
                userBankRanges << bankRange;
                a += 4;
                };
        user->addChildren(userBankRanges);


        QTreeWidgetItem *preset = new QTreeWidgetItem(newTreeList);
        preset->setText(0, "Preset");
        preset->setWhatsThis(0, tr("Preset Banks.<br>expand the Bank to view a section of Banks."));

    QList<QTreeWidgetItem *> presetBankRanges;
    for (int a=(bankTotalUser+1); a<=bankTotalAll; a++)
        {
                QTreeWidgetItem* bankRange = new QTreeWidgetItem; // don't pass a parent here!
                bankRange->setText(0, QString("Bank P").append(QString::number(a-50, 10)).append("-P").append(QString::number(a-46, 10)) );
                bankRange->setWhatsThis(0, tr("Preset Banks.<br>expand the Bank to view a section of Patch Banks"));

                for (int b=a; b<=(a+4); b++)
                {
                        QTreeWidgetItem* bank = new QTreeWidgetItem(bankRange);
                        bank->setText(0, QString("Bank ").append(QString::number(b-50, 10)));
                        bank->setWhatsThis(0, tr("Preset Bank.<br>expand the Bank to view the Patches"));

                        for (int c=1; c<=4; c++)
                        {
                                QTreeWidgetItem* patch = new QTreeWidgetItem(bank);
                                patch->setText(0, QString("Patch ").append(QString::number(c, 10)));
                                patch->setWhatsThis(0, tr("Preset Patches.<br>a single mouse click will only change patch<br>a double mouse click will load the select patch from the GT."));
                        };
                };
                presetBankRanges << bankRange;
                a += 4;
        };
        preset->addChildren(presetBankRanges);


        QTreeWidgetItem *quickFX = new QTreeWidgetItem(newTreeList);
        quickFX->setText(0, "Quick FX");
        quickFX->setWhatsThis(0, "Quick FX Banks");

    QList<QTreeWidgetItem *> userQFXBankRanges;
    for (int a=1; a<=2; a++)
        {
                QTreeWidgetItem* QFXbankRange = new QTreeWidgetItem; // don't pass a parent here!
    QFXbankRange->setText(0, QString("User ").append(QString::number(a, 10)).append("- ").append(QString::number(a+9, 10)) );
                QFXbankRange->setWhatsThis(0, tr("User Banks.<br>expand the Bank to view a section of Patch Banks"));

                        for (int c=1; c<=10; c++)
                        {
                                QTreeWidgetItem* patch = new QTreeWidgetItem(QFXbankRange);//bank);
                                patch->setText(0, QString("QFX User ").append(QString::number(c, 10)));
                                patch->setWhatsThis(0, tr("User Patches.<br>a single mouse click will only change patch<br>a double mouse click will load the select patch from the GT."));
                        };
        userQFXBankRanges << QFXbankRange;
                a += 10;
                };

                QList<QTreeWidgetItem *> presetQFXBankRanges;
    for (int a=1; a<=2; a++)
        {
                QTreeWidgetItem* QFXbankRange = new QTreeWidgetItem; // don't pass a parent here!
    QFXbankRange->setText(0, QString("Preset ").append(QString::number(a, 10)).append(" - ").append(QString::number(a+9, 10)) );
                QFXbankRange->setWhatsThis(0, "Preset Patch-Tree Bank<br>opening the Bank will display the Patches");

                        for (int c=1; c<=10; c++)
                        {
                                QTreeWidgetItem* patch = new QTreeWidgetItem(QFXbankRange);//bank);
                                patch->setText(0, QString("QFX Preset ").append(QString::number(c, 10)));
                                patch->setWhatsThis(0, "Preset Patch-Tree List<br>patch change only with a single mouse click<br>loads in patch data with a double mouse click<br>Preset patches can not be written.");
                        };

                userQFXBankRanges << QFXbankRange;
                a += 10;
                };
        quickFX->addChildren(userQFXBankRanges);
  quickFX->addChildren(presetQFXBankRanges);

        newTreeList->setExpanded(newTreeList->model()->index(1, 0), true);
        newTreeList->setExpanded(newTreeList->model()->index(2, 0), true);
        newTreeList->setExpanded(newTreeList->model()->index(3, 0), true);
        return newTreeList;
};

/*********************** setItemClicked() ***********************************
 * Expands and colapses on a single click and sets patch sellection.
 ****************************************************************************/
void bankTreeList::setItemClicked(QTreeWidgetItem *item, int column)
{
        if(item->childCount() != 0 && item->text(0) != "Temp")
        {
                if(item->isExpanded())
                {
                        item->setExpanded(false);
                }
                else
                {
                        item->setExpanded(true);
                };
        }
        else if (item->childCount() == 0/* && item->text(0) != "Temp" && !item->text(0).contains("QFX Preset")*/)
        {
                SysxIO *sysxIO = SysxIO::Instance();
                if(sysxIO->isConnected() && sysxIO->deviceReady())
                {
                int bank;
                int patch;
                if (!item->text(0).contains("QFX") && !item->text(0).contains("Temp"))
                {
                        bool ok;
                        bank = item->parent()->text(0).section(" ", 1, 1).trimmed().toInt(&ok, 10);
                        patch = item->parent()->indexOfChild(item) + 1;
                        QString preset = item->parent()->parent()->text(0);
                        if (preset.contains("P")) { bank = bank + 50; };
                }
                else if (item->text(0).contains("QFX"))
                {
                 if (item->text(0).contains("QFX User"))
                 {bank = 101;} else {bank = 105;}
      patch = item->parent()->indexOfChild(item) + 1;
    }
    else
    {
      bank = 0;
      patch = 0;
      //emit patchSelectSignal(bank, patch);
    };

                        if (item->text(0) != "Temp" && !item->text(0).contains("QFX"))
                        { sysxIO->requestPatchChange(bank, patch); }; // extra to try patch change
                        sysxIO->setRequestName(item->text(0));	// Set the name of the patch we have sellected in case we load it.
                        //sysxIO->setBank(bank);
                        //sysxIO->setPatch(patch);
                        emit patchSelectSignal(bank, patch);
                };

        };
};

/*********************** setItemDoubleClicked() *****************************
 * Handles when a patch is double clicked in the tree list. Patch will be
 * loaded into the temp buffer and will tell to request the data afterwards.
 ****************************************************************************/
void bankTreeList::setItemDoubleClicked(QTreeWidgetItem *item, int column)
{
        int bank = 0;
        int patch = 0;
        SysxIO *sysxIO = SysxIO::Instance();
        if(item->childCount() == 0 && sysxIO->deviceReady() && sysxIO->isConnected())
                // Make sure it's a patch (Patches are the last in line so no children).
        {
                emit setStatusSymbol(2);
                emit setStatusMessage(tr("Patch request"));

                sysxIO->setDeviceReady(false);
                sysxIO->setRequestName(item->text(0));	// Set the name of the patch we are going to load, so we can check if we have loaded the correct patch at the end.
     if (item->text(0) != "Temp" && !item->text(0).contains("QFX"))
     {
                bool ok;
                bank = item->parent()->text(0).section(" ", 1, 1).trimmed().toInt(&ok, 10); // Get the bank
                patch = item->parent()->indexOfChild(item) + 1;								// and the patch number.
                QString preset = item->parent()->parent()->text(0);
                if (preset.contains("P")) { bank = bank + 50; };

                        requestPatch(bank, patch); // use

                }
                else if(item->text(0) == "Temp")
        {
  requestPatch();
  }
        else
        {
        patch = item->parent()->indexOfChild(item) + 1;								// and the patch number.
        if(item->text(0).contains("QFX User"))
        {  requestPatch(101, patch); };
        if(item->text(0).contains("QFX Preset"))
        {  requestPatch(105, patch); };
        };
         emit patchSelectSignal(bank, patch);
        };
};
/*********************** requestPatch() *******************************
 * Does the actual requesting of the patch data and hands the
 * reception over to updatePatch function.
 **********************************************************************/
void bankTreeList::requestPatch()
{
        SysxIO *sysxIO = SysxIO::Instance();

        QObject::disconnect(sysxIO, SIGNAL(isChanged()),
                        this, SLOT(requestPatch()));

        if(sysxIO->isConnected())
        {
                QObject::connect(sysxIO, SIGNAL(sysxReply(QString)),	// Connect the result of the request
                        this, SLOT(updatePatch(QString)));					// to updatePatch function.

                emit setStatusSymbol(3);
                emit setStatusMessage(tr("Receiving Patch"));
                sysxIO->requestPatch(0, 0);
        };
};

void bankTreeList::requestPatch(int bank, int patch)
{
        SysxIO *sysxIO = SysxIO::Instance();
        if(sysxIO->isConnected())
        {
                QObject::connect(sysxIO, SIGNAL(sysxReply(QString)),	// Connect the result of the request
                        this, SLOT(updatePatch(QString)));					// to updatePatch function.

                emit setStatusSymbol(3);
                emit setStatusMessage(tr("Receiving Patch"));

                sysxIO->requestPatch(bank, patch);
        };
};

/*********************** updatePatch() *******************************
 * Updates the source of the currently handled patch and set the
 * attributes accordingly.
 *********************************************************************/
void bankTreeList::updatePatch(QString replyMsg)
{
        SysxIO *sysxIO = SysxIO::Instance();

        sysxIO->setDeviceReady(true); // Free the device after finishing interaction.

        QObject::disconnect(sysxIO, SIGNAL(sysxReply(QString)),
                this, SLOT(updatePatch(QString)));


        replyMsg = replyMsg.remove(" ").toUpper();       /* TRANSLATE SYSX MESSAGE FORMAT to 128 byte data blocks */
        if (replyMsg.size()/2 == patchReplySize && replyMsg.startsWith("F0410000002F12") && replyMsg.endsWith("F7")){
        QString header = "F0410000002F12";
        QString footer ="00F7";
        QString addressMsb = replyMsg.mid(14,4);
        QString part1 = replyMsg.mid(22, 256);
  part1.prepend("0000").prepend(addressMsb).prepend(header).append(footer);
        QString part2 = replyMsg.mid(278, 228);
        QString part2B = replyMsg.mid(532, 28);
        part2.prepend("0100").prepend(addressMsb).prepend(header).append(part2B).append(footer);
        QString part3 = replyMsg.mid(560, 256);
        part3.prepend("0200").prepend(addressMsb).prepend(header).append(footer);
        QString part4 = replyMsg.mid(816, 200);
        QString part4B = replyMsg.mid(1042, 56);
        part4.prepend("0300").prepend(addressMsb).prepend(header).append(part4B).append(footer);
        QString part5 = replyMsg.mid(1098, 256);
        part5.prepend("0400").prepend(addressMsb).prepend(header).append(footer);
        QString part6 = replyMsg.mid(1354, 172);   //
        part6.prepend("0500").prepend(addressMsb).prepend(header).append(footer);
        QString part7 = replyMsg.mid(1552, 256);   // 0x308+128
        part7.prepend("0600").prepend(addressMsb).prepend(header).append(footer);
        QString part8 = replyMsg.mid(1808, 228);  // 0x388+114
        QString part8B = replyMsg.mid(2062, 28);   //
        part8.prepend("0700").prepend(addressMsb).prepend(header).append(part8B).append(footer);
        QString part9 = replyMsg.mid(2090, 256);
        part9.prepend("0800").prepend(addressMsb).prepend(header).append(footer);
        QString part10 = replyMsg.mid(2346,200);    //
        part10.prepend("0900").prepend(addressMsb).prepend(header).append(footer);
        QString part11 = replyMsg.mid(2572, 256);
        part11.prepend("0A00").prepend(addressMsb).prepend(header).append(footer);
        QString part12 = replyMsg.mid(2828, 226);   //
        QString part12B = replyMsg.mid(3080, 30);
        part12.prepend("0B00").prepend(addressMsb).prepend(header).append(part12B).append(footer);
        QString part13 = replyMsg.mid(3110, 256);
        part13.prepend("0C00").prepend(addressMsb).prepend(header).append(footer);

        QString QFX = "false";
        if (replyMsg.contains("F0410000002F1230") || replyMsg.contains("F0410000002F1240")) // if a QFX patch
          {QFX = "true"; };                                                     // update the temp buffer

        replyMsg = "";
        replyMsg.append(part1).append(part2).append(part3).append(part4).append(part5).append(part6)
  .append(part7).append(part8).append(part9).append(part10).append(part11).append(part12).append(part13);

  QByteArray data;
  QFile file(":default.syx");   // Read the default GT-10 sysx file so we don't start empty handed.
    if (file.open(QIODevice::ReadOnly))
          {	data = file.readAll(); };
          QByteArray temp;
    temp = data.mid(1763, 282);           // copy patch description from default.syx  address 00 0D 00 00

        QString sysxBuffer;
        for(int i=0;i<temp.size();i++)
        {
                unsigned char byte = (char)temp[i];
                unsigned int n = (int)byte;
                QString hex = QString::number(n, 16).toUpper();     // convert QByteArray to QString
                if (hex.length() < 2) hex.prepend("0");
                sysxBuffer.append(hex);
  };
        replyMsg.append(sysxBuffer);

        QString reBuild = "";       /* Add correct checksum to patch strings */
  QString sysxEOF = "";
  QString hex = "";
  int msgLength = replyMsg.length()/2;
  for(int i=0;i<msgLength*2;++i)
  {
        hex.append(replyMsg.mid(i*2, 2));
        sysxEOF = (replyMsg.mid((i*2)+4, 2));
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
        reBuild.append(hex);

                hex = "";
                sysxEOF = "";
                i=i+2;
    };
  };
        replyMsg = reBuild.simplified().toUpper().remove("0X").remove(" ");
        emit setStatusMessage(tr("Ready"));

                QString area = "Structure";
                sysxIO->setFileSource(area, replyMsg);		// Set the source to the data received.
                sysxIO->setFileName(tr("Patch from ") + deviceType);	// Set the file name to GT-10 patch for the display.
                sysxIO->setDevice(true);				// Patch received from the device so this is set to true.
                sysxIO->setSyncStatus(true);			// We can't be more in sync than right now! :)

                sysxIO->setLoadedBank(sysxIO->getBank());
                sysxIO->setLoadedPatch(sysxIO->getPatch());

                emit updateSignal();
                emit setStatusProgress(0);
    if (QFX == "true") { sysxIO->writeToBuffer(); };
        };
        if(!replyMsg.isEmpty() && replyMsg.size()/2 != fullPatchSize)
        {
                //emit notConnectedSignal();				// No message returned so connection must be lost.
                /* NO-REPLY WARNING */
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(QObject::tr("Warning - Patch data received is incorrect!")+(QString::number(replyMsg.size()/2, 10)));
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setTextFormat(Qt::RichText);
        QString msgText;
        msgText.append("<font size='+1'><b>");
        msgText.append(QObject::tr("Patch data transfer wrong size or data error"));
        msgText.append("<b></font><br>");
        msgText.append(QObject::tr("Please make sure the ") + deviceType + QObject::tr(" is connected correctly and re-try."));
        msgBox->setText(msgText);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();
        /* END WARNING */
        };
                if(replyMsg.isEmpty())
        {
                //emit notConnectedSignal();				// No message returned so connection must be lost.
                /* NO-REPLY WARNING */
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setWindowTitle(QObject::tr("Warning - Patch data not received!"));
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setTextFormat(Qt::RichText);
        QString msgText;
        msgText.append("<font size='+1'><b>");
        msgText.append(QObject::tr("Patch data transfer failed, are the correct midi ports selected?"));
        msgText.append("<b></font><br>");
        msgText.append(QObject::tr("Please make sure the ") + deviceType + QObject::tr(" is connected correctly and re-try."));
        msgBox->setText(msgText);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->exec();
        /* END WARNING */
        };
        /*
        Preferences *preferences = Preferences::Instance(); // Load the preferences.
        if(preferences->getPreferences("Midi", "DBug", "bool")=="true")
        {
        if (replyMsg.size() > 0){
                QString snork;
                        snork.append("<font size='-1'>");
                        snork.append(tr("{ size="));
                        snork.append(QString::number(replyMsg.size()/2, 10));
                        snork.append("}");
                        snork.append(tr("<br> midi data received"));
                        for(int i=0;i<replyMsg.size();++i)
                        {
                                snork.append(replyMsg.mid(i, 2));
                                snork.append(" ");
                                i++;
                        };
                        snork.replace("F7", "F7 } <br>");
                        snork.replace("F0", "{ F0");


                        QMessageBox *msgBox = new QMessageBox();
                        msgBox->setWindowTitle(tr("dBug Result for re-formatted GT-10 patch data"));
                        msgBox->setIcon(QMessageBox::Information);
                        msgBox->setText(snork);
                        msgBox->setStandardButtons(QMessageBox::Ok);
                        msgBox->exec();
                        };
                };	*/
};

/********************************** connectedSignal() ****************************
* This slot reloads all patch names of expanded items, if any, on (re)connection to a
* device.
*********************************************************************************/
void bankTreeList::connectedSignal()
{
  //requestPatch(); //load the current temp buffer
        SysxIO *sysxIO = SysxIO::Instance();
        if(this->openPatchTreeItems.size() != 0 && sysxIO->deviceReady() && sysxIO->isConnected())
        {
                 sysxIO->setDeviceReady(false);

                this->listIndex = 0;
                this->itemIndex = 0;

                QObject::disconnect(sysxIO, SIGNAL(patchName(QString)),
                        this, SLOT(updatePatchNames(QString)));
                QObject::connect(sysxIO, SIGNAL(patchName(QString)),
                        this, SLOT(updatePatchNames(QString)));

                this->currentPatchTreeItems.clear();
                this->currentPatchTreeItems = this->openPatchTreeItems;
                qSort(this->currentPatchTreeItems);
                this->updatePatchNames("");
        };
  //requestPatch();
};

/********************************** updateTree() ********************************
* This handles whether we add the newly expanded item to the current job or
* start a new one.
*********************************************************************************/
void bankTreeList::updateTree(QTreeWidgetItem *item)
{
        SysxIO *sysxIO = SysxIO::Instance();
        if(sysxIO->deviceReady() && sysxIO->isConnected())
        {
                sysxIO->setDeviceReady(false);

                this->listIndex = 0;
                this->itemIndex = 0;

                QObject::disconnect(sysxIO, SIGNAL(patchName(QString)),
                        this, SLOT(updatePatchNames(QString)));
                QObject::connect(sysxIO, SIGNAL(patchName(QString)),
                        this, SLOT(updatePatchNames(QString)));

                //this->currentPatchTreeItems.append(item);  //3 lines of mods added below
                this->currentPatchTreeItems.clear();
                this->currentPatchTreeItems = this->openPatchTreeItems;
                qSort(this->currentPatchTreeItems);
                this->updatePatchNames("");
        }
        else
        {
                //this->currentPatchTreeItems.append(item);
        };
};

/***************************** updatePatchNames() ********************************
* This updates the patch names in the treeList of all expanded items. this is
* done when a bank is expanded or when we (re)connect to a device.
*********************************************************************************/
void bankTreeList::updatePatchNames(QString name)
{		SysxIO *sysxIO = SysxIO::Instance();
                if(!name.isEmpty() && sysxIO->isConnected()) //  If not empty we can assume that we did receive a patch name.
                {
                        this->currentPatchTreeItems.at(listIndex)->child(itemIndex)->setText(0, name); // Set the patch name of the item in the tree list.
                        if(itemIndex >= patchPerBank - 1) // If we reach the last patch in this bank we need to increment the bank and restart at patch 1.
                        {
                                this->listIndex++;
                                this->itemIndex = 0;
                        }
                        else
                        {
                                this->itemIndex++;
                        };
                };

                if(listIndex < currentPatchTreeItems.size()) // As long as we have items in the list we continue, duh! :)
                {
                        bool ok;
                        int bank = this->currentPatchTreeItems.at(listIndex)->text(0).section(" ", 1, 1).trimmed().toInt(&ok, 10);
                        int patch = itemIndex + 1 ;
                        QString preset = this->currentPatchTreeItems.at(listIndex)->parent()->text(0);
                        if (preset.contains("P")) { bank = bank + 50; };
                  sysxIO->requestPatchName(bank, patch); // The patch name request.

             if(sysxIO->isConnected())
             {
                      emit setStatusSymbol(3);
                      emit setStatusMessage(tr("Reading names"));
             }
                  else
                    {
                        sysxIO->setDeviceReady(true);

                        this->currentPatchTreeItems.clear(); // We are done so we can safely reset items that need to be named.
                        this->listIndex = 0;
                        this->itemIndex = 0;

                        emit setStatusSymbol(1);
                        emit setStatusMessage(tr("Ready"));
                        emit setStatusProgress(0);
                    };
             }
                else {SysxIO *sysxIO = SysxIO::Instance();
                          sysxIO->setDeviceReady(true);
                          emit setStatusSymbol(1);
        emit setStatusMessage(tr("Ready"));
         };
};

