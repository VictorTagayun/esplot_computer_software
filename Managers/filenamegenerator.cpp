/**
  *********************************************************************************************************************************************************
  @file     :filenamegenerator.cpp
  @brief    :Functions for Filenamegenerator class
  *********************************************************************************************************************************************************
  ESPlot allows real-time communication between an embedded system and a computer offering signal processing and plotting capabilities and it relies on
  hardware graphics acceleration for systems disposing of OpenGL-compatible graphic units. More info at www.uni-saarland.de/lehrstuhl/nienhaus/esplot.
 
  Copyright (C) Universität des Saarlandes 2020. Authors: Emanuele Grasso and Niklas König.
 
  The Software and the associated materials have been developed at the Universität des Saarlandes (hereinafter "UdS").
  Any copyright or patent right is owned by and proprietary material of the UdS hereinafter the “Licensor”.
 
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Affero General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
 
  This Agreement shall be governed by the laws of the Federal Republic of Germany except for the UN Sales Convention and the German rules of conflict of law.
 
  Commercial licensing opportunities
  For commercial uses of the Software beyond the conditions applied by AGPL 3.0 please contact the Licensor sending an email to patentverwertungsagentur@uni-saarland.de
  *********************************************************************************************************************************************************
  */

#include "filenamegenerator.h"

filenameGenerator::filenameGenerator()
{
    saveCounter = 0;
    saveMode = 0;
    saveFolder = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
    saveFileName = "data";
    saveType = ".mat";
}

QString filenameGenerator::generateAutoFileName(uint8_t *out)
{
    //We check in which mode we are to create the new filename
    QString filename;

    if (saveMode == 0)
    {
        saveCounter++;
        filename = saveFolder + QDir::separator() + saveFileName + QString::number(saveCounter).rightJustified(3,'0') + saveType;
        //check if the file exists. in that case update counter and try again until saveCounter reaches 999
        while ((QFile::exists(filename) == true) && (saveCounter < 999))
        {
            saveCounter++;
            filename = saveFolder + QDir::separator() + saveFileName + QString::number(saveCounter).rightJustified(3,'0') + saveType;
        }
        if (saveCounter > 999)  //maximum number reached
        {
            QMessageBox *mbox = new QMessageBox;
            mbox->setWindowTitle("Info");
            mbox->setText("Automatic save counter reached the limit (999)");
            mbox->show();
            QTimer::singleShot(2000, mbox, SLOT(hide()));
            *out = 1;
            return QString();
        }
    }
    if (saveMode == 1)
    {
        QString day, month, year, hour, minute, second;
        day = QString::number(QDateTime::currentDateTime().date().day());
        month = QString::number(QDateTime::currentDateTime().date().month());
        year = QString::number(QDateTime::currentDateTime().date().year());
        hour = QString::number(QDateTime::currentDateTime().time().hour());
        minute = QString::number(QDateTime::currentDateTime().time().minute());
        second = QString::number(QDateTime::currentDateTime().time().second());

        filename = year + "_" + month + "_" + day + "_" + hour + "_" + minute + "_" + second + "_" + saveFileName + saveType;
        filename = saveFolder + QDir::separator() + filename;
        //check if the file exists.. if it does, wait one second (filename will change) and try again until 10 seconds passed
        int counter = 0;
        while ((QFile::exists(filename) == true) && (counter <= 10))
        {
            counter++;
            //delay
            QTime dieTime= QTime::currentTime().addSecs(1);
            while (QTime::currentTime() < dieTime)
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            //create the new file
            day = QString::number(QDateTime::currentDateTime().date().day());
            month = QString::number(QDateTime::currentDateTime().date().month());
            year = QString::number(QDateTime::currentDateTime().date().year());
            hour = QString::number(QDateTime::currentDateTime().time().hour());
            minute = QString::number(QDateTime::currentDateTime().time().minute());
            second = QString::number(QDateTime::currentDateTime().time().second());

            filename = year + "_" + month + "_" + day + "_" + hour + "_" + minute + "_" + second + "_" + saveFileName + saveType;
            filename = saveFolder + QDir::separator() + filename;
        }
        if (counter > 10)
        {
            QMessageBox *mbox = new QMessageBox;
            mbox->setWindowTitle("Info");
            mbox->setText("Automatic save counter reached timeout while trying to save");
            mbox->show();
            QTimer::singleShot(2000, mbox, SLOT(hide()));
            *out = 1;
            return QString();
        }
    }

    *out = 0;
    return filename;
}
