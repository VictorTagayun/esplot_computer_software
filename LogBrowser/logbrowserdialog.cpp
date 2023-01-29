/**
  *********************************************************************************************************************************************************
  @file     :logbrowserdialog.cpp
  @brief    :Functions for LogBrowserDialog class
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
 
 #include "logbrowserdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QCloseEvent>
#include <QKeyEvent>

LogBrowserDialog::LogBrowserDialog(QWidget *parent)
 : QDialog(parent)
{
 QVBoxLayout *layout = new QVBoxLayout;
 setLayout(layout);

browser = new QTextBrowser(this);
 layout->addWidget(browser);

QHBoxLayout *buttonLayout = new QHBoxLayout;
 buttonLayout->setContentsMargins(0, 0, 0, 0);
 layout->addLayout(buttonLayout);

 buttonLayout->addStretch(10);

 clearButton = new QPushButton(this);
 clearButton->setText("clear");
 buttonLayout->addWidget(clearButton);
 connect(clearButton, SIGNAL (clicked()), browser, SLOT (clear()));

 saveButton = new QPushButton(this);
 saveButton->setText("save output");
 buttonLayout->addWidget(saveButton);
 connect(saveButton, SIGNAL (clicked()), this, SLOT (save()));

 resize(200, 400);

 this->setVisible(false);
}


LogBrowserDialog::~LogBrowserDialog()
{

}


void LogBrowserDialog::outputMessage(QtMsgType type, const QString &msg)
{
 switch (type) {
 case QtDebugMsg:
 browser->append(msg);
 break;

 case QtWarningMsg:
 browser->append(tr("— WARNING: %1").arg(msg));
 break;

 case QtCriticalMsg:
 browser->append(tr("— CRITICAL: %1").arg(msg));
 break;

 case QtFatalMsg:
 browser->append(tr("— FATAL: %1").arg(msg));
 break;

 default:
     break;
 }
}


void LogBrowserDialog::save()
{
 QString saveFileName = QFileDialog::getSaveFileName(
 this,
 tr("Save Log Output"),
 tr("%1/logfile.txt").arg(QDir::homePath()),
 tr("Text Files (*.txt);;All Files (*)")
 );

if(saveFileName.isEmpty())
 return;

QFile file(saveFileName);
 if(!file.open(QIODevice::WriteOnly)) {
 QMessageBox::warning(
 this,
 tr("Error"),
 QString(tr("<nobr>File '%1'<br/>cannot be opened for writing.<br/><br/>"
 "The log output could <b>not</b> be saved!</nobr>"))
 .arg(saveFileName));
 return;
 }

QTextStream stream(&file);
 stream << browser->toPlainText();
 file.close();
}

void LogBrowserDialog::closeEvent(QCloseEvent *e)
{
    (void) e;
}

void LogBrowserDialog::keyPressEvent(QKeyEvent *e)
{
 // ignore all keyboard events
 // protects against accidentally closing of the dialog
 // without asking the user
 e->ignore();
}
