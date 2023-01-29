/**
  *********************************************************************************************************************************************************
  @file     :mainApplication.cpp
  @brief    :Main Program Body of the QT Application
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

#include "mainApplication.h"
#include <QFile>
#include <QApplication>

mainApplication::mainApplication(QString pref_filename)
{
    setAcceptDrops(true);

    autorecord_status = false;

    (void) pref_filename;

    prefMng = new prefManager;

    fileGen = new filenameGenerator;

    fontMgr = new fontManager(prefMng->getPreferences());
    //We add the default fonts
    fontMgr->addFont("Euler", ":/fonts/3rdparty/Fonts/Euler/euler.otf");
    fontMgr->addFont("Libertine", ":/fonts/3rdparty/Fonts/Linux Libertine/LinLibertine_M.otf");
    fontMgr->addFont("Quattrocento", ":/fonts/3rdparty/Fonts/Quattrocento/Quattrocento-Regular.ttf");
    fontMgr->setDefaultFont("Euler");

    spManager = 0;
    devEdit = 0;
    isClosing = false;
    connectionStatus = false;

    playInterval = 500;  //default value

    try {
        ftDevice = new ft4222_dev;
    } catch (...) {

    }

    serialDevice = new serial_dev(this);
    commProtocol = new comm_prot;

    CreateMainWindow();

    connect(&playTimer, &QTimer::timeout, this, &mainApplication::PollDataAndPlot);
    connect(&recordTimer, &QTimer::timeout, this, &mainApplication::updateRecordTime);

    updateStatus();

    setMinimumWidth(sizeHint().width());
    setMinimumHeight(sizeHint().height());
}

mainApplication::~mainApplication()
{
    playTimer.stop();
    delete fileGen;
    delete prefMng;
    delete fontMgr;
    delete ftDevice;
    delete serialDevice;
    delete commProtocol;
    logBrowser->getDialog()->close();
    delete logBrowser;
    qDebug() << "Deleting application \n";
}

void mainApplication::updatePlotFonts()
{
    spManager->updateFonts();
}

void mainApplication::replotSlot()
{
    //checks the grid
    bool stat;

    stat = spManager->checkGrids();
    gridTrigAct->setChecked(stat);
}

void mainApplication::gridChanged(int npoints)
{
    if (npoints > nSamplesSB->value())
    {
        nSamplesSB->setValue(npoints);
        nSamplesSB->editingFinished();
    }
}

void mainApplication::closeEvent(QCloseEvent *event)
{
    (void) event;

    isClosing = true;
    //here we delete the created instances
    if (connectionStatus == true)
        closeConnection();
    playTimer.stop();
    if (spManager != 0)
        delete spManager;
    if (devEdit != 0)
        delete devEdit;

    QApplication::instance()->quit();
}

bool mainApplication::event(QEvent *ev) {
    if(ev->type() == QEvent::LayoutRequest) {
        setMinimumSize(sizeHint());
    }
    return QMainWindow::event(ev);
}

void mainApplication::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() == true)
        event->accept();
}

void mainApplication::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        qDebug() << "Loading.. maybe..";
        prefMng->loadFromFile(event->mimeData()->urls().first().path().remove(0, 1));  //removes the first character being a '/'
        if (spManager != 0)
            spManager->updateFonts();
    }
}

void mainApplication::openConnection()
{
    int baudrate;

    cD = new connectDlg(this, ftDevice, serialDevice);

    this->setWindowModality(Qt::WindowModal);
    cD->setModal(true);
    cD->exec();

    if (cD->isDeviceSelected() == true)
    {
        selectedDeviceType = cD->get_SelectedDeviceType();
        baudrate = cD->get_BaudRate();
        ConnectAndPrepare(cD->get_SelectedDevice().idx, baudrate);
    }

    delete cD;

    resize(sizeHint());
}

void mainApplication::closeConnection()
{
    if (connectionStatus == true)
    {
        playTimer.stop();
        commProtocol->disconnect();
        delete spManager;
        delete devEdit;
        spManager = 0;
        devEdit = 0;
        connectionStatus = false;
        sig_indexes.clear();
        plot_indexes.clear();
        delete mainScrollArea;
        resize(sizeHint());
        updateStatus();
    }
}

void mainApplication::exportSignals()
{
    //First we check if there are data to be saved in the buffer
    if ((connectionStatus == true) && (spManager != 0))
    {
        if (spManager->getSignalMemoryData() == 0)
            return;
    }

    //First we get the savefile
    QString filename = QFileDialog::getSaveFileName(this, "Export data", "", "Matlab File (*.mat)");

    if (filename.isEmpty() == false)
    {
        if ((connectionStatus == true) && (spManager != 0))
        {
            int res = spManager->exportToFile(filename);
            if (res != 0)
            {
                QMessageBox msgBox;
                msgBox.setText("Error during export of data");
                msgBox.exec();
            }
        }
    }
}

void mainApplication::autoexportSignals()
{
    //First we check if there are data to be saved in the buffer
    if ((connectionStatus == true) && (spManager != 0))
    {
        if (spManager->getSignalMemoryData() == 0)
            return;
    }

    //We check in which mode we are to create the new filename
    QString filename;
    uint8_t out;

    filename = fileGen->generateAutoFileName(&out);

    if (out != 0)
        return;

    //if the file does not exists, we can save it
    if ((connectionStatus == true) && (spManager != 0))
    {
        int res = spManager->exportToFile(filename);
        if (res != 0)
        {
            QMessageBox *mbox = new QMessageBox;
            mbox->setWindowTitle(tr("Info"));
            mbox->setText("Error during export of data");
            mbox->show();
            QTimer::singleShot(2000, mbox, SLOT(hide()));
        }
    }
    else
    {
        QMessageBox *mbox = new QMessageBox;
        mbox->setWindowTitle(tr("Info"));
        mbox->setText("Data have been saved in " + filename);
        mbox->show();
        QTimer::singleShot(1500, mbox, SLOT(hide()));
    }
}

void mainApplication::saveSettings()
{
    bool ok;
    //ask for what kind of saving mechanism to use
    QStringList items;
    items << QString("Numeric counter ([name]counter.mat)");
    items << QString("Date and time (yyyy_mm_dd_hh_mm_ss_[name].mat)");

    QString selected = QInputDialog::getItem(this, "Automatic export mode", "Automatic filename generation method", items, static_cast<int>(fileGen->getSaveMode()), false, &ok);

    if (ok == false)
        return;

    if (QString::compare(selected, "Numeric counter ([name]counter.mat)", Qt::CaseInsensitive) == 0)
        fileGen->setSaveMode(0);
    if (QString::compare(selected, "Date and time (yyyy_mm_dd_hh_mm_ss_[name].mat)", Qt::CaseInsensitive) == 0)
        fileGen->setSaveMode(1);

    //ask the user to select the folder
    fileGen->setSaveFolder(QFileDialog::getExistingDirectory(this, "Select export folder", fileGen->getSaveFolder(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

    //ask the user for the name
    QString file = QInputDialog::getText(this, "Insert base filename", "Base filename", QLineEdit::Normal, fileGen->getSaveFileName(), &ok);

    if ((ok == false) || (file.isEmpty()))
        return;

    fileGen->setSaveFileName(file);

    //ask if the user wants to reset the numeric counter
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Numeric counter", "Do you want to reset the numeric counter?", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
        fileGen->resetSaveCounter();
}

void mainApplication::preferenceWindow()
{
    pD = new prefDlg(this, prefMng->getPreferences(), spManager->getFontManager());

    connect(pD, &prefDlg::updateFonts, this, &mainApplication::updatePlotFonts);

    this->setWindowModality(Qt::WindowModal);
    pD->setModal(true);
    pD->exec();
    delete pD;
}

void mainApplication::loadSettings()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load preferences", "", "Preference file (*.lpp)");

    if (filename.isEmpty() == false)
    {
        prefMng->loadFromFile(filename);
        if (spManager != 0)
        {
            spManager->updateFonts();
        }
    }
}

void mainApplication::loadStyle1()
{
    prefMng->loadFromFile(":/preferences/Preferences/pref1");
    updatePlotFonts();
}

void mainApplication::loadStyle2()
{
    prefMng->loadFromFile(":/preferences/Preferences/pref2");
    updatePlotFonts();
}

void mainApplication::loadStyle3()
{
    prefMng->loadFromFile(":/preferences/Preferences/pref3");
    updatePlotFonts();
}

void mainApplication::loadStyle4()
{
    prefMng->loadFromFile(":/preferences/Preferences/pref4");
    updatePlotFonts();
}

void mainApplication::infoWindow()
{
    infoDialog *iD = new infoDialog;

    this->setWindowModality(Qt::WindowModal);
    iD->setModal(true);
    iD->exec();
    delete iD;
}

void mainApplication::playData()
{
    //For safety reason, we check that the application is connected
    if (connectionStatus == false)
        return;

    if (playAct->isChecked() == true)
    {
        spManager->Enable_Record_All(false);  //just playing
        appStatus = PLAYING;
        updateStatus();
        //At this point, we start the pollnewdata slot which will use a timer to call itself back at a regular interval
        PollDataAndPlot();
    }
}

void mainApplication::autorecordData()
{
    autorecord_status = autoRecordAct->isChecked();
    qDebug() << autorecord_status;
    updateStatus();
}

void mainApplication::recordData()
{
    if (connectionStatus == false)
        return;

    if (recordAct->isChecked() == true)
    {
        //if it's not playing, we first start the playing
        spManager->Enable_Record_All(true);  //recording now
        PollDataAndPlot();
        appStatus = RECORDING;
        updateStatus();
    }
    else
    {
        spManager->Enable_Record_All(false);
    }
}

void mainApplication::stopData()
{
    playTimer.stop();
    appStatus = STOP;
    updateStatus();
}

void mainApplication::clearData()
{
    //ask first for confirmation
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Data", "This command will remove all recorded data from the memory buffer. Do you want to continue?", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        spManager->clearAllData();  //clears all the data
        memoryLabel->setText(interpretMemorySize(spManager->getSignalMemoryData()));
        updateStatus();
    }
}

void mainApplication::organizeWds()
{
    if (spManager != 0)
        spManager->Organize_Windows();
}

void mainApplication::playIntervalChanged()
{
    playInterval = playTimeSB->value();
}

void mainApplication::nSamplesChanged()
{
    if (spManager != 0)
        spManager->set_Max_N_Data(static_cast<unsigned int>(nSamplesSB->value()));  //sets the maximum number of data to be stored during play
}

void mainApplication::nGridChanged()
{
    if (spManager != 0)
        spManager->set_Plots_N_Points(static_cast<unsigned int>(nGridSB->value()));  //sets the number of samples for the plot grid
}

void mainApplication::gridTrigActivated()
{
    bool gridstat;

    gridstat = spManager->checkGrids();

    //we toggle it
    gridstat = !gridstat;

    spManager->setAllGrid(gridstat);
}

void mainApplication::newFFTWindow()
{
    spManager->newFFTWindow();
}

void mainApplication::remFFTWindow()
{
    spManager->remFFTWindow();
}

void mainApplication::associateSignalFFT()
{
    spManager->assSigFFT();
}

void mainApplication::removeSignalFFT()
{
    spManager->remSigFFT();
}

void mainApplication::exportSignalFFT()
{
    spManager->exportFFT();
}

void mainApplication::CreateMenuBar()
{
    fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(openConnAct);
    fileMenu->addAction(closeConnAct);
    fileMenu->addSeparator();
    fileMenu->addAction(recordAct);
    fileMenu->addAction(playAct);
    fileMenu->addAction(stopAct);
    fileMenu->addAction(clearAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exportAct);
    fileMenu->addAction(autoExportAct);
    fileMenu->addAction(saveSettAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    optionsMenu = menuBar()->addMenu("&Options");
    optionsMenu->addAction(prefAct);
    optionsMenu->addSeparator();
    optionsMenu->addAction(loadSettAct);
    optionsMenu->addSeparator();
    optionsMenu->addAction(loadStyle1Act);
    optionsMenu->addAction(loadStyle2Act);
    optionsMenu->addAction(loadStyle3Act);
    optionsMenu->addAction(loadStyle4Act);

    toolMenu = menuBar()->addMenu("&Tools");
    fftMenu = toolMenu->addMenu("&FFT");
    fftMenu->addAction(newFFTWindowAct);
    fftMenu->addAction(remFFTWindowAct);
    fftMenu->addSeparator();
    fftMenu->addAction(associateFFTAct);
    fftMenu->addAction(removeFFTAct);
    fftMenu->addSeparator();
    fftMenu->addAction(exportSignalFFTAct);
    toolMenu->addSeparator();
    toolMenu->addAction(organizeWndsAct);

    helpMenu = menuBar()->addMenu("&?");
    helpMenu->addAction(showInfoDlg);
    helpMenu->addSeparator();
    helpMenu->addAction(infoAct);

    toolBar = this->addToolBar("Main Toolbar");
    toolBar->setIconSize(QSize(32,32));

    toolBar->addAction(openConnAct);
    toolBar->addAction(closeConnAct);
    toolBar->addSeparator();
    toolBar->addAction(exportAct);
    toolBar->addAction(autoExportAct);
    toolBar->addSeparator();
    toolBar->addAction(autoRecordAct);
    toolBar->addSeparator();
    toolBar->addAction(recordTimeAct);
    toolBar->addAction(recordAct);
    toolBar->addAction(playAct);
    toolBar->addAction(stopAct);
    toolBar->addAction(clearAct);
    toolBar->addSeparator();   
    toolBar->addAction(organizeWndsAct);
    toolBar->addSeparator();
    toolBar->addWidget(new QLabel("Time(ms):  "));
    toolBar->addWidget(playTimeSB);
    toolBar->addSeparator();
    toolBar->addWidget(new QLabel("Samples:  "));
    toolBar->addWidget(nSamplesSB);
    toolBar->addSeparator();
    toolBar->addWidget(new QLabel("Grid:   "));
    toolBar->addWidget(nGridSB);
    toolBar->addAction(gridTrigAct);

    statusLabel = new QLabel;
    infoLabel = new QLabel;
    statusBar()->addWidget(statusLabel);
    statusBar()->addPermanentWidget(infoLabel);
    statusBar()->addPermanentWidget(memoryLabel);
    statusBar()->addPermanentWidget(messLabel);
}

void mainApplication::CreateActions()
{
    openConnAct = new QAction("&Open Connection");
    openConnAct->setIcon(QIcon(":/Icons/Icons/Connect.ico"));
    openConnAct->setShortcut(QKeySequence::Open);
    openConnAct->setStatusTip("Establish a connection with the device");
    connect(openConnAct, &QAction::triggered, this, &mainApplication::openConnection);

    closeConnAct = new QAction("&Close Connection");
    closeConnAct->setIcon(QIcon(":/Icons/Icons/Disconnect.ico"));
    closeConnAct->setStatusTip("Close the established connection");
    connect(closeConnAct, &QAction::triggered, this, &mainApplication::closeConnection);

    exportAct = new QAction("E&xport data...");
    exportAct->setIcon(QIcon(":/Icons/Icons/save.ico"));
    exportAct->setShortcut(QKeySequence::SaveAs);
    exportAct->setStatusTip("Exports the recorded data");
    connect(exportAct, &QAction::triggered, this, &mainApplication::exportSignals);

    autoExportAct = new QAction("&Auto Export");
    autoExportAct->setIcon(QIcon(":/Icons/Icons/autosave.ico"));
    autoExportAct->setStatusTip("Automatically exports the recorded data");
    connect(autoExportAct, &QAction::triggered, this, &mainApplication::autoexportSignals);

    saveSettAct = new QAction("Auto Export Settings...");
    saveSettAct->setToolTip("Settings for the automatic data export");
    connect(saveSettAct, &QAction::triggered, this, &mainApplication::saveSettings);

    exitAct = new QAction("&Exit");
    exitAct->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    exitAct->setShortcut(QKeySequence::Quit);
    exitAct->setStatusTip("Exit the application");
    connect(exitAct, &QAction::triggered, this, &mainApplication::close);

    prefAct = new QAction("&Preferences...");
    prefAct->setShortcut(QKeySequence::Preferences);
    prefAct->setStatusTip("Set application preferences...");
    connect(prefAct, &QAction::triggered, this, &mainApplication::preferenceWindow);

    loadSettAct = new QAction("&Load Preferences...");
    loadSettAct->setStatusTip("Load preferred settings");
    connect(loadSettAct, &QAction::triggered, this, &mainApplication::loadSettings);

    loadStyle1Act = new QAction("Load Style 1");
    loadStyle1Act->setStatusTip("Loads predefined style 1");
    connect(loadStyle1Act, &QAction::triggered, this, &mainApplication::loadStyle1);

    loadStyle2Act = new QAction("Load Style 2");
    loadStyle2Act->setStatusTip("Loads predefined style 2");
    connect(loadStyle2Act, &QAction::triggered, this, &mainApplication::loadStyle2);

    loadStyle3Act = new QAction("Load Style 3");
    loadStyle3Act->setStatusTip("Loads predefined style 3");
    connect(loadStyle3Act, &QAction::triggered, this, &mainApplication::loadStyle3);

    loadStyle4Act = new QAction("Load Style 4");
    loadStyle4Act->setStatusTip("Loads predefined style 4");
    connect(loadStyle4Act, &QAction::triggered, this, &mainApplication::loadStyle4);

    infoAct = new QAction("&Info");
    infoAct->setShortcut(QKeySequence::HelpContents);
    infoAct->setStatusTip("Information about this application");
    connect(infoAct, &QAction::triggered, this, &mainApplication::infoWindow);

    gridTrigAct = new QAction(this);
    gridTrigAct->setIcon(QIcon(":/Icons/Icons/grid.ico"));
    gridTrigAct->setCheckable(true);
    gridTrigAct->setChecked(true);
    gridTrigAct->setToolTip("Grid (CTRL+G)");
    gridTrigAct->setText("&Grid");
    connect(gridTrigAct, &QAction::triggered, this, &mainApplication::gridTrigActivated);

    autoRecordAct = new QAction;
    autoRecordAct->setIcon(QIcon(":/Icons/Icons/Autorecord.ico"));
    autoRecordAct->setCheckable(true);
    autoRecordAct->setChecked(autorecord_status);
    autoRecordAct->setToolTip("Enable/Disable automatic recording of signals...");
    autoRecordAct->setText("&Enable auto recording");
    connect(autoRecordAct, &QAction::triggered, this, &mainApplication::autorecordData);

    recordAct = new QAction;
    recordAct->setIcon(QIcon(":/Icons/Icons/Record.ico"));
    recordAct->setCheckable(true);
    recordAct->setChecked(false);
    recordAct->setToolTip("Start recording signals...");
    recordAct->setText("&Record");
    connect(recordAct, &QAction::triggered, this, &mainApplication::recordData);

    playAct = new QAction;
    playAct->setIcon(QIcon(":/Icons/Icons/Play.ico"));
    playAct->setCheckable(true);
    playAct->setChecked(false);
    playAct->setToolTip("Start receiving signals...");
    playAct->setText("&Play");
    connect(playAct, &QAction::triggered, this, &mainApplication::playData);

    stopAct = new QAction;
    stopAct->setIcon(QIcon(":/Icons/Icons/Stop.ico"));
    stopAct->setCheckable(false);
    stopAct->setToolTip("Stop receiving signals...");
    stopAct->setText("&Stop");
    connect(stopAct, &QAction::triggered, this, &mainApplication::stopData);    

    clearAct = new QAction;
    clearAct->setIcon(QIcon(":/Icons/Icons/Clear.ico"));
    clearAct->setCheckable(false);
    clearAct->setToolTip("Clear data buffer...");
    clearAct->setText("&Clear");
    connect(clearAct, &QAction::triggered, this, &mainApplication::clearData);

    organizeWndsAct = new QAction(this);
    organizeWndsAct->setIcon(QIcon(":/Icons/Icons/windows.ico"));
    organizeWndsAct->setCheckable(false);
    organizeWndsAct->setToolTip("Organize Plot Windows");
    organizeWndsAct->setText("&Organize Plot Windows");
    connect(organizeWndsAct, &QAction::triggered, this, &mainApplication::organizeWds);

    newFFTWindowAct  = new QAction(this);
    newFFTWindowAct->setToolTip("Creates a new FFT window");
    newFFTWindowAct->setText("&New FFT Window");
    connect(newFFTWindowAct, &QAction::triggered, this, &mainApplication::newFFTWindow);

    remFFTWindowAct = new QAction(this);
    remFFTWindowAct->setToolTip("Deletes a FFT window");
    remFFTWindowAct->setText("&Remove FFT Window");
    connect(remFFTWindowAct, &QAction::triggered, this, &mainApplication::remFFTWindow);

    associateFFTAct = new QAction(this);
    associateFFTAct->setToolTip("Associate a signal to a FFT window");
    associateFFTAct->setText("&Assign signal to FFT");
    connect(associateFFTAct, &QAction::triggered, this, &mainApplication::associateSignalFFT);

    removeFFTAct = new QAction(this);
    removeFFTAct->setToolTip("Eliminate a signal from a FFT window");
    removeFFTAct->setText("R&emove signal from FFT");
    connect(removeFFTAct, &QAction::triggered, this, &mainApplication::removeSignalFFT);

    exportSignalFFTAct = new QAction(this);
    exportSignalFFTAct->setToolTip("Export the FFT data of a window to a Matlab file");
    exportSignalFFTAct->setText("&Export to Matlab");
    connect(exportSignalFFTAct, &QAction::triggered, this, &mainApplication::exportSignalFFT);

    showInfoDlg = new QAction(this);
    showInfoDlg->setToolTip("Show the info log");
    showInfoDlg->setText("Show info log");
    LogBrowserDialog *dlg = logBrowser->getDialog();
    connect(showInfoDlg, &QAction::triggered, dlg, &LogBrowserDialog::show);

    QFont fnt;
    fnt.setBold(true);
    fnt.setPixelSize(32);
    recordTimeAct = new QAction(this);
    recordTimeAct->setText("0:00");
    recordTimeAct->setFont(fnt);
}

void mainApplication::CreateMainWindow()
{
    playTimeSB = new QSpinBox(this);
    playTimeSB->setMinimum(10);
    playTimeSB->setMaximum(std::numeric_limits<int>::max());
    playTimeSB->setValue(playInterval);
    playTimeSB->resize(playTimeSB->sizeHint());
    connect(playTimeSB, SIGNAL (editingFinished()), this, SLOT (playIntervalChanged()));

    nSamplesSB = new QSpinBox(this);
    nSamplesSB->setMinimum(2);
    nSamplesSB->setMaximum(std::numeric_limits<int>::max());
    nSamplesSB->setValue(prefMng->getPreferences()->N_points);
    nSamplesSB->resize(nSamplesSB->sizeHint());
    connect(nSamplesSB, SIGNAL (editingFinished()), this, SLOT (nSamplesChanged()));

    nGridSB = new QSpinBox(this);
    nGridSB->setMinimum(2);
    nGridSB->setMaximum(std::numeric_limits<int>::max());
    nGridSB->setValue(prefMng->getPreferences()->N_points);
    nGridSB->resize(nGridSB->sizeHint());
    connect(nGridSB, SIGNAL (editingFinished()), this, SLOT (nGridChanged()));

    infoLabel = new QLabel("ES Plot v0.9");
    infoLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    infoLabel->setAlignment(Qt::AlignRight);

    memoryLabel = new QLabel("");
    memoryLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    memoryLabel->setAlignment(Qt::AlignRight);
    memoryLabel->setText("0 byte");

    messLabel = new QLabel("");
    messLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    messLabel->setAlignment(Qt::AlignRight);
    messLabel->setText("- ms");

    setWindowTitle("ES Plot v0.9");
    setMinimumSize(400, 200);

    //Create here the MenuBar
    CreateActions();
    CreateMenuBar();

    infoLabel->setText("ES Plot v0.9 - OpenGL Accelerated");

    this->resize(this->sizeHint());
}

void mainApplication::ConnectAndPrepare(unsigned int index, int baudrate)
{
    QMessageBox msgBox;
    comm_prot::error_t error;

    //QElapsedTimer timer;
    //long long time_connect, time_info_frame, time_prepare1, time_prepare2;

    if (selectedDeviceType == 0)  //FT device
    {
        comm_dev::connection_status_t res = ftDevice->connect(index);
        if (res == comm_dev::CONNECTED)  //connection successful
        {
            //timer.start();

            commProtocol->connect(ftDevice);

            //time_connect = timer.nsecsElapsed();

            //In this case we request for info frame
            connectionStatus = true;
            QThread::msleep(250);  //waits for connection to be established
            error = commProtocol->request_descriptor_frame_and_initialize_comm_prot();

            //time_info_frame = timer.nsecsElapsed();

            if (error != comm_prot::SUCCESS)
            {
                //Error during obtaining info frame
                qDebug() << "Error during parsing the information frame...";
                msgBox.setText("Error during parsing the information frame");
                msgBox.exec();
                connectionStatus = false;
                commProtocol->disconnect();
                ftDevice->disconnect();
                updateStatus();
            }
            else
            {
                //We create the deviceEditor
                devEdit = new devEditor;
                devEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                devEdit->setMinimumHeight(300);
                connect(devEdit, &devEditor::dataReady, this, &mainApplication::txDataReady);

                //We create spManager
                spManager = new SgnalPlotterManager(prefMng->getPreferences(), fontMgr, fileGen);
                spManager->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                spManager->setMinimumHeight(600);
                connect(spManager, &SgnalPlotterManager::replotCalled, this, &mainApplication::replotSlot);
                connect(spManager, &SgnalPlotterManager::gridChanged, this, &mainApplication::gridChanged);

                QSplitter *split = new QSplitter(Qt::Vertical);
                split->addWidget(devEdit);
                split->addWidget(spManager);

                mainScrollArea = new QScrollArea;
                mainScrollArea->setWidget(split);
                mainScrollArea->setWidgetResizable(true);
                setCentralWidget(mainScrollArea);

                setMinimumHeight(1000);

                //Info frame received successfully => ready to communicate
                connectionStatus = true;
                appStatus = INITIALIZED;

                //Set correct playInterval
                playInterval = 50; //static_cast<int>(commProtocol->get_recommended_trigger_time());
                playTimeSB->setValue(playInterval);

                PreparePlots();
                //time_prepare1 = timer.nsecsElapsed();

                spManager->Enable_Record_All(false);
                spManager->Prepare_and_Plot();

                //time_prepare2 = timer.nsecsElapsed();

                //qDebug() << "Time connect    = " << time_connect / 1000;
                //qDebug() << "Time info frame = " << (time_info_frame - time_connect) / 1000;
                //qDebug() << "Time prepare 1  = " << (time_prepare1 - time_info_frame) / 1000;
                //qDebug() << "Time prepare 2  = " << (time_prepare2 - time_prepare1) / 1000;

                qDebug() << "Connection successfull!";

                spManager->Organize_Windows();

                updateStatus();
            }
        }
        else
        {
            qDebug() << "Error during connection to the device";
            msgBox.setText("Error during connection to the device");
            msgBox.exec();
        }

    }
    else  //Serial device
    {
        serialDevice->set_baudrate(baudrate);
        comm_dev::connection_status_t res = serialDevice->connect(index);

        if (res == comm_dev::CONNECTED)  //connection successful
        {
            commProtocol->connect(serialDevice);

            //In this case we request for info frame
            connectionStatus = true;
            QThread::msleep(1000);  //waits for connection to be established
            error = commProtocol->request_descriptor_frame_and_initialize_comm_prot();
            if (error != comm_prot::SUCCESS)
            {
                //Error during obtaining info frame
                qDebug() << "Error during parsing the information frame...";
                msgBox.setText("Error during parsing the information frame");
                msgBox.exec();
                connectionStatus = false;
                commProtocol->disconnect();
                serialDevice->disconnect();
                updateStatus();
            }
            else
            {
                //We create the deviceEditor
                devEdit = new devEditor;
                devEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                connect(devEdit, &devEditor::dataReady, this, &mainApplication::txDataReady);

                //We create spManager
                spManager = new SgnalPlotterManager(prefMng->getPreferences(), fontMgr, fileGen);
                spManager->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

                QSplitter *split = new QSplitter(Qt::Vertical);
                split->addWidget(devEdit);
                split->addWidget(spManager);
                setCentralWidget(split);

                //Info frame received successfully => ready to communicate
                connectionStatus = true;
                appStatus = INITIALIZED;

                //Set correct playInterval
                playInterval = commProtocol->get_recommended_trigger_time();
                playTimeSB->setValue(playInterval);


                PreparePlots();

                spManager->Enable_Record_All(false);
                spManager->Prepare_and_Plot();

                qDebug() << "Connection successfull!";

                updateStatus();
            }
        }
        else
        {
            qDebug() << "Error during connection to the device";
            msgBox.setText("Error during connection to the device");
            msgBox.exec();
        }

    }
}

void mainApplication::PreparePlots()
{
    unsigned int N;
    int i, j;
    const int max_N_plots = 16;
    //const int max_N_FFT = 3;
    //const int max_N_XY = 1;
    //const int max_N_XYZ = 1;
    unsigned short plots;
    unsigned int frequency;
    QString sig_name;

    vector<comm_prot::comm_data_descriptor_t> info;
    vector<comm_prot::comm_data_descriptor_t> tx_info;

    frequency = commProtocol->get_process_freq();
    if (frequency == 0)
        frequency = 1;

    spManager->set_plot_frequency(static_cast<double>(frequency));

    info = commProtocol->get_rx_data_descriptor_list();
    N = static_cast<unsigned int>(info.size());
    sig_indexes.resize(static_cast<int>(N));
    plot_indexes.resize(max_N_plots);
    for (i = 0; i < max_N_plots; i++)  //initializes all the indexes to -1
        plot_indexes[i] = -1;

    for (i = 0; i < static_cast<int>(N); i++)
    {
        //Add a new signal
        sig_name = QString::fromStdString(info[static_cast<unsigned int>(i)].signal_name);
        sig_name.remove(QChar::Null);
        sig_indexes[i] = static_cast<int>(spManager->Add_Signal(sig_name, static_cast<int>(info[static_cast<unsigned int>(i)].type), info[static_cast<unsigned int>(i)].scaling_factor));
        //We check to which plots it is associated. If the plot to which it is associated has not been added, it will be added
        plots = info[static_cast<unsigned int>(i)].representation;
        for (j = 0; j < max_N_plots; j++)  //the MSP bit indicates if the signal has to be shown numerically
        {
            if ((plots & 0x0001) == 1)  //there is association
            {
                //we check if a plot has already been initialized in this position
                if (plot_indexes[j] == -1)  //the plot does not exist
                    plot_indexes[j] = static_cast<int>(spManager->Add_Plot("Plot " + sig_name, static_cast<double>(frequency)));
                //At this points we associate the signal to the plot
                QColor color;
                if (info[static_cast<unsigned int>(i)].alpha == 0x00)
                    color = get_random_color();
                else
                    color.setRgb(static_cast<int>(info[static_cast<unsigned int>(i)].r), static_cast<int>(info[static_cast<unsigned int>(i)].g), static_cast<int>(info[static_cast<unsigned int>(i)].b));
                spManager->Associate(static_cast<unsigned int>(sig_indexes[i]), static_cast<unsigned int>(plot_indexes[j]), color, static_cast<float>(info[static_cast<unsigned int>(i)].line_width));
            }
            plots = plots >> 1;
        }
    }

    spManager->Organize_Windows();

    tx_info = commProtocol->get_tx_data_descriptor_list();
    QString st;
    unsigned int k;
    for (i = 0; i < static_cast<int>(tx_info.size()); i++)
    {
        st = QString::fromStdString(tx_info[static_cast<unsigned int>(i)].signal_name);
        k = 0;
        while ((st[k] != 0) && (k < static_cast<unsigned int>(st.length())))
            k++;
        st = st.mid(0, k);
        devEdit->Add_Signal(st, i, 0);
    }

    //we bring the mainwindow on top
    this->activateWindow();  //might not work on OSX or Linux => Gotta check it out
}

void mainApplication::PollDataAndPlot()
{
    vector<vector<float>> data;
    vector<uint8_t> cmd;
    QElapsedTimer timer;  //measures the time needed for polling data and plot, cannot be stopped
    unsigned int N_sig, i, N_data;
    comm_prot::error_t res;
    uint64_t passed_time;
    int64_t interval;

    //int64_t time1, time2, time3;

    if (isClosing == true)
        return;

    playTimer.stop();
    timer.start();

    res = commProtocol->comm_manager();

    //time1 = timer.nsecsElapsed();

    if ((res != comm_prot::error_t::SUCCESS) && (res != comm_prot::error_t::NO_DATA_AVAILABLE))
    {
        //trigger error => to be implemented
        qDebug() << "Error";
    }
    else if (res == comm_prot::error_t::NO_DATA_AVAILABLE)
    {
        //do nothing
    }
    else
    {
        N_sig = commProtocol->get_n_rx_data();
        data = commProtocol->get_rx_data();
        cmd = commProtocol->get_cmd();

        N_data = static_cast<unsigned int>(data[0].size());

        spManager->Pass_Cmd_to_Pool(cmd.data(), static_cast<int>(N_data));

        for (i = 0; i < N_sig; i++)
        {
            spManager->Pass_Data_to_Signal(static_cast<unsigned int>(sig_indexes[static_cast<int>(i)]), data[i].data(), static_cast<int>(N_data));
        }

        if (autorecord_status == true)
        {
            parse_res res = parseCmd(cmd);

            switch(res)
            {
            case NO_ACT:
                //do nothing at all
                break;

            case RECORD_ACT:
                if (appStatus != RECORDING)
                {
                    spManager->Enable_Record_All(true);  //recording now
                    appStatus = RECORDING;
                    updateStatus();
                }
                break;

            case SAVE_ACT:
                switch(appStatus)
                {
                case RECORDING:
                    //here we need to emulate the stop, save the data and then start the play again
                    spManager->autoExportToFile();
                    //restore to playing
                    spManager->Enable_Record_All(false);  //just playing
                    appStatus = PLAYING;
                    updateStatus();
                    break;

                case PLAYING:
                    //here we just need to take the current data and pass them to the saving file
                    spManager->autoExportToFile();
                    break;

                default:
                    break;
                }
                break;
            }
        }

        //time2 = timer.nsecsElapsed();
        spManager->Prepare_and_Plot();
        //time3 = timer.nsecsElapsed();
    }

    //qDebug() << "N_data = " << N_sig*N_data << "; Comm time: " << time1 / 1000 << " us; Pass time: " << (time2-time1)/1000 << " us; Plot time: " << (time3-time2)/1000 << " us";

    //we calculate now how much time has passed and we set the timer for the next poll

    passed_time = static_cast<uint64_t>(timer.nsecsElapsed()) / 1000000; //divided by 1000000 to bring it in milliseconds

    interval = static_cast<int64_t>(playInterval) - static_cast<int64_t>(passed_time);

    if (interval <= 0)
        interval = 10;  //10 milliseconds in case more time has passed than playInterval

    playTimer.setInterval(static_cast<int>(interval));

    if (passed_time > (static_cast<unsigned int>(playInterval)))  //critical timing
        messLabel->setStyleSheet("QLabel { color : red; }");
    else
        messLabel->setStyleSheet("QLabel { color : black; }");
    messLabel->setText(QString::number(passed_time) + " ms");

    memoryLabel->setText(interpretMemorySize(spManager->getSignalMemoryData()));

    playTimer.start();
}

void mainApplication::updateRecordTime()
{
    recordTime += 1;
    int sec, min, hour;
    hour = recordTime / 3600;
    min = (recordTime % 3600) / 60;
    sec = (recordTime % 3600) % 60;

    recordTimeAct->setText(QString::number(hour).rightJustified(2, '0') + ":" + QString::number(min).rightJustified(2, '0') + ":" + QString::number(sec).rightJustified(2, '0'));
}

void mainApplication::txDataReady()
{
    if (connectionStatus == false)
        return;

    if (devEdit != 0)
    {
        QVector<int> tx_data;
        vector<int> tx_data_conv;

        tx_data = devEdit->get_Data();
        tx_data_conv.resize(static_cast<std::size_t>(tx_data.count()));

        for (int i = 0; i < tx_data.count(); i++)
            tx_data_conv[static_cast<std::size_t>(i)] = tx_data[i];

        commProtocol->set_tx_data(tx_data_conv);
        
        commProtocol->set_terminal_command(devEdit->get_Command().toStdString());
    }
}

QColor mainApplication::get_random_color()
{
    QColor col;

#ifndef NO_RANDOM_COLOR
    int r, g, b;

    r = QRandomGenerator::global()->bounded(255);
    g = QRandomGenerator::global()->bounded(255);
    b = QRandomGenerator::global()->bounded(255);

    col.setRed(r); col.setGreen(g); col.setBlue(b);
#else
    col = QColor("Black");
#endif

    return col;
}

QString mainApplication::interpretMemorySize(qint64 mem)
{
    QString ret;

    if (mem < 1024)  //we express it in bytes
        return QString::number(mem) + " byte";
    mem = mem / 1024;
    if (mem < 1024)  //we express it in kbytes
        return QString::number(mem) + " Kbyte";
    mem = mem / 1024;
    if (mem < 1024)  //we express it in mbytes
        return QString::number(mem) + " Mbyte";
    //otherwise gbytes
    return QString::number(mem) + " Gbytes";
}

parse_res mainApplication::parseCmd(vector<uint8_t> c)
{
    bool found;

    found = false;

    if (c.size() == 0)
        return NO_ACT;

    //we check immediately the last command
    if (c[c.size() - 1] == RECORD_CMD)
        return RECORD_ACT;

    //if the start command is a RECORD_CMD and we end with a NO_CMD then we need to save (stop and save)
    if ((c[0] == RECORD_CMD) && (c[c.size()-1] == NO_CMD))
        return SAVE_ACT;

    //if we start with NO_CMD and end with NO_CMD, then we return NO_ACT is no transition has happend or SAVE_ACT if a transition has happened
    if ((c[0] == NO_CMD) && (c[c.size()-1] == NO_CMD))
    {
        //we check for a transition => enough to look for one RECORD_CMD
        for (unsigned long i = 0; i < c.size(); i++)
            if (c[i] == RECORD_CMD)
            {
                found = true;
                break;
            }
    }
    if (found == true)
        return SAVE_ACT;
    else {
        return NO_ACT;
    }
}

void mainApplication:: updateStatus()
{
    QString stat;

    if (connectionStatus == false)
    {
        stat = "Not connected";
        recordTimer.stop();
        openConnAct->setEnabled(true);
        closeConnAct->setEnabled(false);
        recordTimeAct->setVisible(false);
        autoRecordAct->setEnabled(false);
        recordAct->setEnabled(false);
        playAct->setEnabled(false);
        stopAct->setEnabled(false);
        prefAct->setEnabled(false);
        clearAct->setEnabled(false);
        loadSettAct->setEnabled(false);
        organizeWndsAct->setEnabled(false);
        fftMenu->setEnabled(false);
        newFFTWindowAct->setEnabled(false);
        remFFTWindowAct->setEnabled(false);
        associateFFTAct->setEnabled(false);
        removeFFTAct->setEnabled(false);
        exportSignalFFTAct->setEnabled(false);
        nSamplesSB->setEnabled(false);
        nGridSB->setEnabled(false);
        gridTrigAct->setEnabled(false);
        exportAct->setEnabled(false);
        autoExportAct->setEnabled(false);
        loadStyle1Act->setEnabled(false);
        loadStyle2Act->setEnabled(false);
        loadStyle3Act->setEnabled(false);
        loadStyle4Act->setEnabled(false);
        if (spManager != 0)
            spManager->setPushButtonsEnable(false);
    }
    else
    {
        prefAct->setEnabled(true);
        loadSettAct->setEnabled(true);
        organizeWndsAct->setEnabled(true);
        fftMenu->setEnabled(true);
        nSamplesSB->setEnabled(true);
        nGridSB->setEnabled(true);
        gridTrigAct->setEnabled(true);
        loadStyle1Act->setEnabled(true);
        loadStyle2Act->setEnabled(true);
        loadStyle3Act->setEnabled(true);
        loadStyle4Act->setEnabled(true);

        switch (appStatus)
        {
        case INITIALIZED:
            stat = "Ready for communication";
            recordTimer.stop();
            recordTimeAct->setVisible(false);
            exportAct->setEnabled(true);
            autoExportAct->setEnabled(true);
            openConnAct->setEnabled(false);
            closeConnAct->setEnabled(true);
            autoRecordAct->setEnabled(true);
            autoRecordAct->setChecked(autorecord_status);
            if (autorecord_status == true)
            {
                recordAct->setChecked(false);
                recordAct->setEnabled(false);
            }
            else
            {
                recordAct->setEnabled(true);
            }
            playAct->setEnabled(true);
            stopAct->setEnabled(false);
            clearAct->setEnabled(false);
            newFFTWindowAct->setEnabled(true);
            remFFTWindowAct->setEnabled(true);
            associateFFTAct->setEnabled(true);
            removeFFTAct->setEnabled(true);
            exportSignalFFTAct->setEnabled(true);
            if (spManager != 0)
                spManager->setPushButtonsEnable(true);
            break;

        case PLAYING:
            stat = "Communicating...";
            recordTimer.stop();
            recordTimeAct->setVisible(false);
            exportAct->setEnabled(false);
            autoExportAct->setEnabled(false);
            openConnAct->setEnabled(false);
            closeConnAct->setEnabled(true);
            autoRecordAct->setEnabled(true);
            if (autorecord_status == true)
            {
                recordAct->setChecked(false);
                recordAct->setEnabled(false);
            }
            else
                recordAct->setEnabled(true);
            recordAct->setChecked(false);
            playAct->setEnabled(false);
            stopAct->setEnabled(true);
            clearAct->setEnabled(false);
            newFFTWindowAct->setEnabled(false);
            remFFTWindowAct->setEnabled(false);
            associateFFTAct->setEnabled(false);
            removeFFTAct->setEnabled(false);
            exportSignalFFTAct->setEnabled(false);
            if (spManager != 0)
                spManager->setPushButtonsEnable(false);
            break;

        case RECORDING:
            stat = "Recording...";
            recordTimeAct->setVisible(true);
            recordTimeAct->setText("00:00:00");
            recordTime = 0;
            recordTimer.start(1000);  //updates each second
            exportAct->setEnabled(false);
            autoExportAct->setEnabled(false);
            openConnAct->setEnabled(false);
            closeConnAct->setEnabled(true);
            autoRecordAct->setEnabled(false);
            recordAct->setEnabled(false);
            playAct->setEnabled(false);
            playAct->setChecked(true);
            stopAct->setEnabled(true);
            clearAct->setEnabled(false);
            newFFTWindowAct->setEnabled(false);
            remFFTWindowAct->setEnabled(false);
            associateFFTAct->setEnabled(false);
            removeFFTAct->setEnabled(false);
            exportSignalFFTAct->setEnabled(false);
            if (spManager != 0)
                spManager->setPushButtonsEnable(false);
            break;

        case STOP:
            stat = "Ready for communication";
            recordTimer.stop();
            exportAct->setEnabled(true);
            autoExportAct->setEnabled(true);
            openConnAct->setEnabled(false);
            closeConnAct->setEnabled(true);
            autoRecordAct->setEnabled(true);
            recordAct->setEnabled(true);
            recordAct->setChecked(false);
            playAct->setEnabled(true);
            playAct->setChecked(false);
            stopAct->setEnabled(false);
            if (spManager->getSignalMemoryData() == 0)  //no data available
                clearAct->setEnabled(false);
            else {
                clearAct->setEnabled(true);
            }
            newFFTWindowAct->setEnabled(true);
            remFFTWindowAct->setEnabled(true);
            associateFFTAct->setEnabled(true);
            removeFFTAct->setEnabled(true);
            exportSignalFFTAct->setEnabled(true);
            if (spManager != 0)
                spManager->setPushButtonsEnable(true);
            break;

        case COMM_ERROR:
            stat = "Communication error: data might be lost";
            break;
        }
    }
    statusLabel->setText(stat);
}
