/**
  *********************************************************************************************************************************************************
  @file     :mainApplication.h
  @brief    :Main Header File of the QT Application
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QThread>
#include <QScrollArea>
#ifndef NO_RANDOM_COLOR
#include <QRandomGenerator>
#endif
#include <QTimer>
#include <QElapsedTimer>
#include <QSpinBox>
#include <QSplitter>
#include <QDragMoveEvent>
#include "LogBrowser/logbrowser.h"
#include "LogBrowser/logbrowserdialog.h"

#include "Dialogs/deveditor.h"
#include "Dialogs/infodialog.h"
#include "Dialogs/prefDlg.h"
#include "Dialogs/connectdlg.h"
#include "Managers/prefmanager.h"
#include "Managers/sgnalplottermanager.h"
#include "FontManager/fontmanager.h"

#include "Managers/filenamegenerator.h"

#include "CommProtocol/ft4222_dev.h"
#include "CommProtocol/serial_dev.h"
#include "CommProtocol/comm_prot.h"

extern QPointer<LogBrowser> logBrowser;

typedef enum parse_res
{
    NO_ACT,
    RECORD_ACT,
    SAVE_ACT
} parse_res;

class mainApplication : public QMainWindow
{
    Q_OBJECT

public:
    mainApplication(QString pref_filename);
    ~mainApplication() override;

    typedef enum {INITIALIZED, PLAYING, RECORDING, STOP, COMM_ERROR} app_status_t;

protected:
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent *ev) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void openConnection();
    void closeConnection();
    void exportSignals();
    void autoexportSignals();
    void saveSettings();
    void preferenceWindow();
    void loadSettings();
    void loadStyle1();
    void loadStyle2();
    void loadStyle3();
    void loadStyle4();
    void infoWindow();
    void playData();
    void autorecordData();
    void recordData();
    void stopData();
    void clearData();
    void organizeWds();
    void playIntervalChanged();
    void nSamplesChanged();
    void nGridChanged();
    void gridTrigActivated();
    void newFFTWindow();
    void remFFTWindow();
    void associateSignalFFT();
    void removeSignalFFT();
    void exportSignalFFT();

    void PollDataAndPlot();
    void updateRecordTime();
    void txDataReady();

    void updatePlotFonts();

    void replotSlot();
    void gridChanged(int npoints);

private:
    QScrollArea *mainScrollArea;
    SgnalPlotterManager *spManager;  //it manages plots and signals
    devEditor *devEdit;
    prefManager *prefMng;  //it handles the preferences
    fontManager *fontMgr;

    ft4222_dev *ftDevice;  //it handles the FT4222 communication
    serial_dev *serialDevice; //it handles the serial port communication
    comm_prot *commProtocol;  //it handles the communication protocol
    int selectedDeviceType;  //0: FT; 1: Serial

    filenameGenerator *fileGen;

    bool autorecord_status;

    bool connectionStatus;  //false => not connected; true => connected
    app_status_t appStatus;
    QTimer playTimer;  //used for polling new data
    QTimer recordTimer;  //updates the record time
    uint64_t recordTime;
    int playInterval;  //it indicates the minimum time to wait for polling new data
    QSpinBox *playTimeSB;
    QSpinBox *nSamplesSB;
    QSpinBox *nGridSB;

    QVector<int> sig_indexes;
    QVector<int> plot_indexes;

    bool isClosing;

    prefDlg *pD;
    connectDlg *cD;

    //Menu Bar
    QLabel *statusLabel;  //gives indication about the connection status
    QLabel *infoLabel;  //information about the application
    QLabel *memoryLabel;  //information about the amount of bytes taken by the signals
    QLabel *messLabel; //gives information about time taken for calculation
    QToolBar *toolBar;
    QMenu *fileMenu;
    QMenu *optionsMenu;
    QMenu *toolMenu;
    QMenu *fftMenu;
    QMenu *helpMenu;
    QAction *openConnAct;
    QAction *closeConnAct;
    QAction *exportAct;
    QAction *autoExportAct;
    QAction *saveSettAct;
    QAction *exitAct;
    QAction *prefAct;
    QAction *loadSettAct;
    QAction *loadStyle1Act;
    QAction *loadStyle2Act;
    QAction *loadStyle3Act;
    QAction *loadStyle4Act;
    QAction *infoAct;
    QAction *autoRecordAct;
    QAction *recordAct;
    QAction *playAct;
    QAction *stopAct;
    QAction *clearAct;
    QAction *organizeWndsAct;
    QAction *newFFTWindowAct;
    QAction *remFFTWindowAct;
    QAction *associateFFTAct;
    QAction *removeFFTAct;
    QAction *exportSignalFFTAct;
    QAction *recordTimeAct;
    QAction *showInfoDlg;
    QAction *gridTrigAct;

    void updateStatus(void);  //updates the status of the widgets
    void CreateMenuBar(void);
    void CreateActions(void);
    void CreateMainWindow(void);
    void ConnectAndPrepare(unsigned int index, int baudrate);
    void PreparePlots();
    QColor get_random_color();  //TO BE DELETED LATER ON
    QString interpretMemorySize(qint64 mem);
    parse_res parseCmd(vector<uint8_t> c);
};

#endif // APPLICATION_H
