/**
  *********************************************************************************************************************************************************
  @file     :xy_plot_window.h
  @brief    :Header for Signal Assignment Dialog class
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

#ifndef XY_PLOT_WINDOW_H
#define XY_PLOT_WINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMainWindow>
#include <QCheckBox>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QToolButton>
#include <QMenuBar>
#include <QIcon>
#include <QStyle>
#include <QFileDialog>
#include <QColor>
#include <QString>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QGroupBox>
#include <QLabel>
#include <QSizePolicy>
#include <QLocale>
#include <QSizePolicy>
#include <QKeyEvent>
#include <QDockWidget>
#include <QStatusBar>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include "definitions.h"
#include "Creators/statcreator.h"
#include "FontManager/fontmanager.h"
#include "Managers/prefmanager.h"
#include "xy_glwindow.h"
#include "Creators/legendCreator.h"


class xy_plot_Window : public QMainWindow
{
    Q_OBJECT

public:
    xy_plot_Window(QString title, int index, double frequency, appPreferencesStruct *pref, fontManager *font);
    ~xy_plot_Window() override;

    XY_GLWindow* get_Plot_Pointer(void) { return glPlot; }
    void addSignal(uint32_t x_index, uint32_t y_index, QString x_name, QString y_name, QColor color);
    void removeSignal(uint32_t x_index, uint32_t y_index);
    void setNPoints(int N);
    void setWindowFrequency(double freq) { if (freq > 0) windowFrequency = freq; }
    void setTitle(QString title);

    int prepare_Signal_Data(int n_signals, int n_points, float** x_buff_ptr, float** y_buff_ptr, QColor* colors, float *line_widths);  //points to n_signals buffers and indicates how many points to be prepared

    void update(void);
    void updateFonts() { glPlot->updateFonts(); }

    void setGrid(bool en);
    bool isGridEnabled() { return glPlot->get_Grid()->getDrawGrid(); }

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *ev) override;

private slots:
    void exportFigure(void);
    void closeWindow(void);
    void triggerAutoscale(void);
    void triggerZoomXY(void);
    void triggerZoomX(void);
    void triggerZoomY(void);
    void triggerPan(void);
    void triggerLegend(void);
    void triggerGrid(void);
    void triggerYTicks(void);
    void triggerXTicks(void);
    void triggerTitle(void);
    void minXSBchanged();
    void maxXSBchanged();
    void minYSBchanged();
    void maxYSBchanged();
    void nsamplesSBchanged();
    void nsamplesSliderchanged(int);
    void nsamplesSliderreleased();
    void xGridRatioChanged(int);
    void yGridRatioChanged(int);
    void autoscaleCenterChanged(int);
    void pastValueGainChanged(int);
    void updateSigProperties(QVector<XY_SigProperty> properties);

signals:
    void replot(int index);
    void closing(int index);
    void gridNSamplesChanged(int nsamples);

public slots:
    void gridChanged(void);

private:
    fontManager *fontMgr;
    int plotIndex;
    appPreferencesStruct *preferences;
    XY_GLWindow *glPlot;

    int N_Samples;
    double windowFrequency;

    QVector<XY_SigProperty> sig_properties;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *toolMenu;
    QToolBar *toolBar;
    QAction *exportFigAct;
    QAction *closeAct;
    QAction *gridAct;
    QAction *yTicksAct;
    QAction *xTicksAct;
    QAction *titleAct;
    QAction *autoscaleAct;
    QVector<QAction*> triggerSignals;
    QActionGroup *triggerGroup;
    QAction *zoomXYAct;
    QAction *zoomXAct;
    QAction *zoomYAct;
    QAction *panAct;
    QAction *legendAct;

    QWidget *centralWidget;

    QVBoxLayout *layout;
    QLabel *statLabel;
    QDoubleSpinBox *minXSB;
    QDoubleSpinBox *maxXSB;
    QDoubleSpinBox *minYSB;
    QDoubleSpinBox *maxYSB;
    QSlider *yGridSlider;
    QSlider *xGridSlider;
    QSlider *autoscaleSlider;
    QSlider *pastValueSlider;
    QSlider *nSamplesSlider;
    QSpinBox *nSamplesSB;
    QGroupBox *groupSB;
    QVBoxLayout *layoutSB;
    QHBoxLayout *layoutSB1;
    QHBoxLayout *layoutSB2;
    QWidget *bottomFill;

    int Number_of_Points;

    bool Autoscale;
    int autoscaleType;  //from 0 to 100 (0 exact positioning)

    float pastValueGain;

    void createActions(void);
    void createSpinBoxes(void);

    QVector<int> indexes;  //all the signals are contained into one single buffer. in this vector we store the location at which each signal starts
    QVector<GLfloat> x_signal_buffer;  //contains the buffer of the x_signals to be displayed => a multiple of N_signals and N_points
    QVector<GLfloat> y_signal_buffer;  //contains the buffer of the y_signals to be displayed
    int x_signal_buffer_count, y_signal_buffer_count;
    void find_min_max_signals(int n_signals, int start, int end, float** buff_ptr, float* Min, float *Max);
};
#endif // XY_PLOT_WINDOW_H
