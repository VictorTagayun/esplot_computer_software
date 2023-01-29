/**
  *********************************************************************************************************************************************************
  @file     :fftmanager.h
  @brief    :Header for FFT manager class
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

#ifndef FFTMANAGER_H
#define FFTMANAGER_H

#include <QWidget>
#include <QString>
#include <QThread>
#include <QVector>
#include <QColor>
#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QTableView>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QStandardItem>

#include <math.h>

#include "matlabfilesaver.h"

#include "Dialogs/fft_plot_window.h"
#include "FontManager/fontmanager.h"
#include "prefmanager.h"

#include "3rdparty/kissFFT/kiss_fftr.h"

typedef struct _fftWindow
{
    int idx;
    fft_plot_Window *fftWnd;
    int n_Samples;  //how bis is the signal's data going to be (possibly a multiple of 2)
    int fftType;  //0 = Amplitude; 1 = Power; 2 = Power_dB
    QString title;
    float stepFrequency;
    int N_sig;  //number of signals
    QVector<QString> sig_name;
    QVector<float> line_width;
    QVector<int> sigIdx;
    QVector<QColor> sigCol;
    QVector<QVector<float>> sig_data;  //contains the data of the signals to be calculated => this matrix size is initialized at the creation of the window and it is not supposed to be modified
    QVector<QVector<float>> fft_data;  //contains the calculated fft data => the size of this container is ((n_Samples/2) + 1) * N_sig
    QVector<float> nullVector;  //used for accelerating the copy of data
} fftWindow;

class fftManager : public QWidget
{
    Q_OBJECT
public:
    fftManager(appPreferencesStruct *pref, fontManager *font);
    ~fftManager();

    int createFFTWindow(QString title, int n_Samples, int type);  //returns window's index
    void removeFFTWindow(int index);  //deletes a window

    void addSigFFT(int windowIdx, int sigIdx, QString name, float line_width, QColor col);
    int remSigFFT(int windowIdx, int sigIdx);

    void startFFTCalculation();

    void setFrequency(float freq) { if (freq > 0.0) frequency = freq; }

    QVector<int> getSignalIndexPerWindow(int wdwIdx);

    void updateSigData(int wdwIdx, int sigIdx, float *data, int N_data);

    bool getStatus() { return status; }

    int getNWindows() { return windowPool.count(); }
    QString getWindowName(int wdwIdx, bool *ok);

    fft_plot_Window *getWindowHandler(int pos);

    int exportToFile(QString filename, int index);

    void checkGrids(int *n, int *m);
    void setAllGrids(bool en);

    appPreferencesStruct *preferences;
    fontManager *fontMgr;  //it manages all the fonts

private slots:
    void jobHasFinished();

    void plotListDoubleClicked(const QModelIndex & index);

    void gridTriggered();

signals:
    void calculationFinished();  //signals that all the FFTs have been calculated
    void gridActTriggered();

private:
    float frequency;
    bool status;  //false = not calculating; true = calculating
    int N_WDWs;  //number of opened windows
    QVector<fftWindow> windowPool;  //contains the fftWindows and its settings

    QFutureWatcher<void> jobWatch;

    QTableView *plotList;
    QStandardItemModel *plotListModel;
    QMap<QStandardItem*, int> plotListAss;

    int getNewIndex();
    int findWindow(int index);
    int findSig(int wdwIdx, int sigIdx);

    void prepareViewModel();
    QString getTypeText(int type);

    void calculateFFTdata();
};

#endif // FFTMANAGER_H
