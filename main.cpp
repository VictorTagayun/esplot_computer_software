/**
  *********************************************************************************************************************************************************
  @file     :main.cpp
  @brief    :Main Programm Body
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

#include <QApplication>
#include <QDesktopWidget>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QtConcurrent/QtConcurrent>
#include <QIcon>
#include <QFile>
#include <QPixmap>
#include <QSplashScreen>
#include <QThreadPool>
#include <QMessageBox>
#include <QDebug>
#include <QtGlobal>

#include "mainApplication.h"

#include "Managers/sgnalplottermanager.h"
#include "Dialogs/plot_window.h"

QPointer<LogBrowser> logBrowser;

void delay(int n);

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    (void) context;

    if(logBrowser)
        logBrowser->outputMessage( type, msg );
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);
    QApplication app(argc, argv);

    logBrowser = new LogBrowser;

    QApplication::setWindowIcon(QIcon(":/Icons/Icons/esp_logo_icon.ico"));
    QCoreApplication::setApplicationName("ES Plot v0.9");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setVersion(3, 2);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSamples(16);
    fmt.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(fmt);

    qDebug() << "OpenGL 3.2 context acquired...";

    //If number of cores of the system is smaller than two, the application will not start
    int n_threads = QThreadPool::globalInstance()->maxThreadCount();
    if (n_threads < 2)
    {
        qDebug() << "This system does not have two available CPU cores!";
        QMessageBox msgBox;
        msgBox.setText("This application requires a dual core CPU!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return 0;
    }

    //Load style
    QFile file(":/styles/Styles/customstyle.qss");
    file.open(QFile::ReadOnly);
    QString s_sheet = QLatin1String(file.readAll());
    file.close();
    app.setStyleSheet(s_sheet);
    qDebug() << "Application style has been loaded...";

    //Load splash screen
    QPixmap logo(":/Icons/Icons/EsPlotLogo.png");
    logo = logo.scaled(350, 350, Qt::KeepAspectRatio);
    QSplashScreen loadWdw(logo);
    loadWdw.setWindowFlags(loadWdw.windowFlags() | Qt::WindowStaysOnTopHint);
    loadWdw.show();
    loadWdw.showMessage("Loading...", Qt::AlignBottom);
    qDebug() << "Loading the application...";

    delay(1500);

    app.processEvents();

    QString filename;

    if (argc == 1)  //no files passed
        filename = ":/preferences/Preferences/style1.lpp";
    else
        filename = argv[1];  //passes the file given to the application by the user

    mainApplication mainApp(filename);

    qInstallMessageHandler(myMessageOutput);

    mainApp.show();
    loadWdw.finish(&mainApp);

    return app.exec();
}

void delay(int n)
{
    QTime dieTime= QTime::currentTime().addMSecs(n);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
