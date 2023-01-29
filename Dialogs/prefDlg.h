/**
  *********************************************************************************************************************************************************
  @file     :prefDlg.h
  @brief    :Header for Preferences Dialog class
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

#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QMainWindow>
#include <QWindow>
#include <QObject>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QColorDialog>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QPalette>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QList>
#include <QDebug>
#include <QAction>
#include <QToolBar>
#include <QFileDialog>
#include <QSplitter>
#include <QStandardPaths>

#include <math.h>

#include "Managers/prefmanager.h"
#include "FontManager/fontmanager.h"
#include "FontManager/fontpreview.h"

class prefDlg : public QDialog
{
    Q_OBJECT

public:
    prefDlg(QMainWindow *parent = 0, appPreferencesStruct *pref = 0, fontManager *font = 0);

signals:
    void updateFonts();

public slots:

    void showEvent(QShowEvent* event) override;

    void apply(QAbstractButton*);
    void applyandclose();
    void loadPref(void);
    void savePref(void);

    void addFonts(void);

    void changeBackGroundColor(void);
    void changeWindowWidth(void);
    void changeWindowHeight(void);
    void changefontResolution(void);

    void changeNPoints(void);
    void changeMaxY(void);
    void changeMinY(void);
    void changeGridY(void);
    void changeGridX(void);
    void changeGridColor(void);
    void changeAxisColor(void);
    void changeGridLW(void);
    void changeAxisLW(void);
    void changeSmoothGrid(void);
    void changeZoomColor(void);

    void changeTitleColor(void);
    void changeTitleSize(void);
    void changeAxisSize(void);
    void changeTitleFont(int index);
    void changeToolTipFont(int index);
    void changeToolTipSize(void);
    void changeToolTipLineSpace(void);
    void changeToolTipColor(void);

    void changeLegendFont(int index);
    void changeLegendBckAreaColor(void);
    void changeLegendCharSize(void);
    void changeLegendLineSpace(void);
    void changeStatsFont(int index);
    void changeStatsCharSize(void);
    void changeStatsLineSpace(void);

private:
    fontManager *fontMgr;
    QMainWindow *parentWindow;

    prefManager prefMgr;
    appPreferencesStruct preferences;
    appPreferencesStruct *mainAppPref;
    QList<QString> fontsList;
    QDialogButtonBox *buttonBox;
    QTabWidget *tabWidget;

    QMainWindow *mainWindow;

    QWidget *gridWidget;  //includes window and grid
    QWidget *titleWidget;  //includes title and tooltip
    QWidget *legendWidget;  //include legend and stats
    fontPreview *fontPrev;  //font preview window

    QVBoxLayout *mainLayout;

    QAction *loadAct;
    QAction *saveAct;
    QAction *exitAct;

    void createToolBar(QToolBar *toolBar);
    void createGridWidget();
    void createTitleWidget();
    void createLegendWidget();
    //group boxes
    QGroupBox *windowGB;
    QGroupBox *gridGB;
    QGroupBox *titleGB;
    QGroupBox *toolTipGB;
    QGroupBox *legendGB;
    QGroupBox *statsGB;
    //window
    QPushButton *bckGndColorPB;
    QSpinBox *plotwidthSB;
    QSpinBox *plotheightSB;
    QSpinBox *fontresSB;
    QPushButton *addFontSB;
    //grid
    QSpinBox *npointsSB;
    QDoubleSpinBox *maxYSB;
    QDoubleSpinBox *minYSB;
    QDoubleSpinBox *gridXSB;
    QDoubleSpinBox *gridYSB;
    QPushButton *gridColorPB;
    QPushButton *axisColorPB;
    QSpinBox *gridLWSB;
    QSpinBox *axisLWSB;
    QCheckBox *smoothCB;
    QPushButton *zoomColorPB;
    //title
    QSpinBox *titleSizeSB;
    QSpinBox *axisSizeSB;
    QComboBox *titleFontCB;
    QPushButton *titleColorPB;
    //tooltip
    QSpinBox *toolTipSizeSB;
    QSpinBox *toolTipLineSpaceSB;
    QComboBox *toolTipFontCB;
    QPushButton *toolTipColorPB;
    //legend
    QSpinBox *legendSizeSB;
    QSpinBox *legendLineSpaceSB;
    QComboBox *legendFontCB;
    QPushButton *legendBckColorPB;
    //stats
    QSpinBox *statsSizeSB;
    QSpinBox *statsLineSpaceSB;
    QComboBox *statsFontCB;

    QColor invertColor(QColor in);
    void fillFontList();
    void updateFontList();
    void updateValues();  //used when a new preference structure is loaded. also used at the creation of the dialog
    int findFont(QComboBox *ptr, QString ref);
};

#endif // PREFERENCEDIALOG_H
