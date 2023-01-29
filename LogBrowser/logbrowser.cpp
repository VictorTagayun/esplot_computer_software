/**
  *********************************************************************************************************************************************************
  @file     :logbrowser.cpp
  @brief    :Functions for LogBrowser class
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

#include "logbrowser.h"

#include <QMetaType>

#include "logbrowserdialog.h"

LogBrowser::LogBrowser(QObject *parent) :
 QObject(parent)
{
 qRegisterMetaType<QtMsgType>("QtMsgType");
 browserDialog = new LogBrowserDialog;
 connect(this, SIGNAL (sendMessage(QtMsgType,QString)), browserDialog, SLOT (outputMessage(QtMsgType,QString)), Qt::QueuedConnection);
}

LogBrowser::~LogBrowser()
{
 delete browserDialog;
}

void LogBrowser::outputMessage(QtMsgType type, const QString &msg)
{
 emit sendMessage( type, msg );
}
