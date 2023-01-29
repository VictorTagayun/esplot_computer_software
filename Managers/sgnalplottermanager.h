/**
  *********************************************************************************************************************************************************
  @file     :sgnalplottermanager.h
  @brief    :Header of the Signal Plotter Manager Class
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

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include <QWindow>
#include <QScreen>
#include <QVector>
#include <QString>
#include <QListWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QSize>
#include <QTimer>
#include <QLabel>
#include <QElapsedTimer>
#include <QDesktopWidget>
#include <QSplitter>
#include <QPushButton>
#include <QInputDialog>
#include <QMap>
#include <QList>

#include "definitions.h"
#include "fftmanager.h"
#include "matlabfilesaver.h"
#include "FontManager/fontmanager.h"
#include "prefmanager.h"
#include "signal_data.h"
#include "Creators/grid.h"
#include "Dialogs/plot_window.h"
#include "Dialogs/fft_plot_window.h"
#include "Dialogs/xy_plot_window.h"
#include "Dialogs/sigassdlg.h"
#include "filenamegenerator.h"

#define NO_CMD		0
#define RECORD_CMD	1

//This class contains all the signals information and data
//It also contains all the information about the number of plotter class instances
//and by passing their grid info prepares the data to be sent to a particular plot
class SgnalPlotterManager : public QWidget
{
    Q_OBJECT

public:
    SgnalPlotterManager(appPreferencesStruct *pref, fontManager *font, filenameGenerator *gen);
    ~SgnalPlotterManager();

    appPreferencesStruct *preferences;
    fontManager *fontMgr;  //it manages all the fonts

    QTableView *sigView;
    QStandardItemModel *sigViewModel;
    QListWidget *thumbnailPlots;
    QTimer *thumbTimer;
    QPushButton *addPlotPB;
    QPushButton *remPlotPB;
    QPushButton *sigSettPB;
    QPushButton *associatePB;
    QPushButton *deassociatePB;
    QPushButton *associateXYPB;
    QPushButton *deassociateXYPB;
    QPushButton *titleChangePB;

    fontManager *getFontManager() { return fontMgr; }

    uint32_t Add_Signal(QString signal_name, int type, float scaling);  //adds a signal by specifying its name and return the index to the added signal
    void Remove_Signal(uint32_t index);  //removes it by index
    void Pass_Data_to_Signal(uint32_t index, float* data, int N_Data);  //passed the obtained data to the indexed signal
    void Clear_Signal_Data(uint32_t index);  //clears the data of a signal

    void Pass_Cmd_to_Pool(uint8_t* cmd, int N_Data);

    uint32_t Add_Plot(QString plot_name, double frequency);
    void Remove_Plot(uint32_t index);

    uint32_t Add_XY_Plot(QString plot_name, double frequency);
    void Remove_XY_Plot(uint32_t index);

    void Associate(uint32_t signal_index, uint32_t plot_index, QColor color, float line_width);  //associate a signal to a plot
    bool DeAssociate(uint32_t signal_index, uint32_t plot_index);  //deassociate a signal from a plot

    void Associate_XY(uint32_t x_signal_index, uint32_t y_signal_index, uint32_t plot_index, QColor color, float line_width);  //associate two signals to a xy_plot
    bool DeAssociate_XY(uint32_t x_signal_index, uint32_t y_signal_index, uint32_t plot_index);  //deassociate a couple of signals from a xy_plot

    void Enable_Record(uint32_t signal_index, bool record);
    void Enable_Record_All(bool record);

    void set_plot_frequency(double frequency) { if (frequency > 0) { plot_frequency = frequency; fftMgr->setFrequency(frequency); } }

    void Prepare_and_Plot(void);  //prepares the datas per each plot and trigger the painting

    void set_Max_N_Data(unsigned int N) { if (N > 1) maxNData = N; }
    void set_Plots_N_Points(int N);

    void newFFTWindow();
    void remFFTWindow();
    void assSigFFT();
    void remSigFFT();
    void exportFFT();

    int exportToFile(QString filename);
    int autoExportToFile();

    void Organize_Windows();  //automatically organizes windows

    qint64 getSignalMemoryData();
    void clearAllData();

    void updateFonts();

    bool checkGrids();
    void setAllGrid(bool en);

    void setPushButtonsEnable(bool en);

public slots:
    void replot(int index);
    void updateThumbs();
    void plotClose(int index);
    void xy_plotClose(int index);
    void thumbDoubleClick(QListWidgetItem *item);
    void addPlotTriggered();
    void remPlotTriggered();
    void sigSetTriggered();
    void associateTriggered();
    void deassociateTriggered();
    void associateXYTriggered();
    void deassociateXYTriggered();
    void changeTitleTriggered();

    void gridFFTChanged();
    void gridNSampChanged(int npoints);

signals:
    void replotCalled();
    void gridChanged(int npoints);

private:
    fftManager *fftMgr;
    filenameGenerator *fileGen;

    unsigned int maxNData;  //when not in record mode, indicates how many data samples will be stored per signal
    QVector<Signal_Data*> Signal_Pool;  //this is our pool of Signals
    uint32_t N_Signals;  //number of signals in the pool

    QVector<uint8_t> command_Pool;
    bool command_Rec;

    QVector<Plot_Structure> Plot_Pool;  //this is our pool of Plots
    uint32_t N_Plots;  //number of plots in the pool

    QVector<XY_Plot_Structure> XY_Plot_Pool;  //this is our pool of XY_Plots
    uint32_t N_XY_Plots;  //number of xy_plots in the pool

    QMap<QListWidgetItem*, uint32_t> thumbPlotAss;  //associates a thumbnail item to its plot index
    QMap<uint32_t, QListWidgetItem*> thumbPlotAssInv;  //inverse association
    QMap<QStandardItem*, uint32_t> viewSigAss;  //associates a standard item to its signal index

    //FFT section
    QVector<int> fftWdwList;

    double plot_frequency;

    uint32_t get_new_signal_index();  //retrieves a new index to be assigend to a signal
    int find_signal_by_index(uint32_t index);

    uint32_t get_new_plot_index();  //retrieves a new index to be assigned to a plot
    int find_plot_by_index(uint32_t index, int *type);

    int get_Min(int* buff, int N);
    int get_Min_Vector(QVector<int> vect);

    void prepareSigViewModel();

    int Prepare_and_Plot_Individual(int i);
    int Prepare_and_Plot_XY_Individual(int i);

    QString get_type_text(int type);
};

#endif // SIGNALMANAGER_H
