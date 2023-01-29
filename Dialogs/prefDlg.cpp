/**
  *********************************************************************************************************************************************************
  @file     :prefDlg.cpp
  @brief    :Functions for Preferences Dialog class
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

#include "prefDlg.h"

#include <QDebug>

prefDlg::prefDlg(QMainWindow *parent, appPreferencesStruct *pref, fontManager *font)
{
    parentWindow = parent;
    mainAppPref = pref;
    preferences = *pref;
    fontMgr = font;
    fillFontList();

    //Creates dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &prefDlg::apply);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &prefDlg::applyandclose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    gridWidget = new QWidget(this);
    titleWidget = new QWidget(this);
    legendWidget = new QWidget(this);
    fontPrev = new fontPreview(this, fontMgr);
    fontPrev->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    createGridWidget();
    createTitleWidget();
    createLegendWidget();

    //Creates the tab structure
    tabWidget = new QTabWidget;
    tabWidget->addTab(gridWidget, "Window and Grid");
    tabWidget->addTab(titleWidget, "Title and Tooltip");
    tabWidget->addTab(legendWidget, "Legend and Statistics");

    QSplitter *split = new QSplitter(Qt::Vertical);
    split->addWidget(tabWidget);
    split->addWidget(fontPrev);

    QMainWindow *mainW = new QMainWindow();
    createToolBar(mainW->addToolBar("ToolBar"));

    QVBoxLayout *innerLayout = new QVBoxLayout;
    innerLayout->addWidget(split);
    innerLayout->addWidget(buttonBox);

    QWidget *innerWid = new QWidget;
    innerWid->setLayout(innerLayout);

    mainW->setCentralWidget(innerWid);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(mainW);

    setLayout(mainLayout);

    setWindowTitle("Preferences");

    updateValues();
}

void prefDlg::showEvent(QShowEvent *event)
{
    (void) event;
    resize(600, 1048);
}

void prefDlg::apply(QAbstractButton *abstButt)
{
    if (abstButt == buttonBox->button(QDialogButtonBox::Apply))
    {
        *mainAppPref = preferences;
        emit(updateFonts());
    }
}

void prefDlg::applyandclose()
{
    *mainAppPref = preferences;
    emit(updateFonts());
    this->close();
}

void prefDlg::loadPref()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load preferences", "", "Preference file (*.lpp)");

    if (filename.isEmpty() == false)
    {
        prefMgr.loadFromFile(filename);
        preferences = prefMgr.preferences;
        updateValues();  //loads the new preferences in the window dialog
    }
}

void prefDlg::savePref()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save preferences", "", "Preference file (*.lpp)");

    if (filename.isEmpty() == false)
    {
        prefMgr.preferences = preferences;
        prefMgr.saveOnFile(filename);
    }
}

void prefDlg::addFonts()
{
    int ok;
    QString fontname, fontDir;
    QStringList fontDirList;

    fontDirList = QStandardPaths::standardLocations(QStandardPaths::FontsLocation);
    if (fontDirList.count() > 0)  //we take the first one
        fontDir = fontDirList.at(0);
    else
        fontDir = QDir::currentPath();

    QStringList filenames = QFileDialog::getOpenFileNames(this, "Font files", fontDir, "All files (*.*)");
    if (!filenames.isEmpty())
    {
        for (int i = 0; i < filenames.count(); i++)
        {
            fontname = fontMgr->getFontName(filenames.at(i), &ok);
            if ((ok == 0) && (fontMgr->findFont(fontname) == -1))  //the font is valid and it does not exist in the font manager already
                fontMgr->addFont(fontname, filenames.at(i));
        }
    }

    //we update the font list in the comboboxes
    updateFontList();
}

void prefDlg::changeBackGroundColor()
{
    QColor color = QColorDialog::getColor(preferences.backGround_color, this);
    if (color.isValid())
    {
        preferences.backGround_color = color;
        updateValues();
        fontPrev->setBackgroundColor(color);
        fontPrev->update();
    }
}

void prefDlg::changeWindowWidth()
{
    preferences.plot_width_size = plotwidthSB->value();
}

void prefDlg::changeWindowHeight()
{
    preferences.plot_height_size = plotheightSB->value();
}

void prefDlg::changefontResolution()
{
    preferences.font_resolution = fontresSB->value();
}

void prefDlg::changeNPoints()
{
    preferences.N_points = npointsSB->value();
}

void prefDlg::changeMaxY()
{
    preferences.max_Y = static_cast<float>(maxYSB->value());
}

void prefDlg::changeMinY()
{
    preferences.max_Y = static_cast<float>(minYSB->value());
}

void prefDlg::changeGridY()
{
    preferences.grid_Y = static_cast<float>(gridYSB->value());
}

void prefDlg::changeGridX()
{
    preferences.grid_X = static_cast<float>(gridXSB->value());
}

void prefDlg::changeGridColor()
{
    QColor color = QColorDialog::getColor(preferences.grid_color, this);
    if (color.isValid())
    {
        preferences.grid_color = color;
        updateValues();
    }
}

void prefDlg::changeAxisColor()
{
    QColor color = QColorDialog::getColor(preferences.axis_color, this);
    if (color.isValid())
    {
        preferences.axis_color = color;
        updateValues();
    }
}

void prefDlg::changeGridLW()
{
    preferences.grid_linewidth = static_cast<float>(gridLWSB->value()) / 10000.0f;
}

void prefDlg::changeAxisLW()
{
    preferences.axis_linewidth = static_cast<float>(axisLWSB->value()) / 10000.0f;
}

void prefDlg::changeSmoothGrid()
{
    preferences.smoothGrid = smoothCB->isChecked();
}

void prefDlg::changeZoomColor()
{
    QColor color = QColorDialog::getColor(preferences.zoom_color, this);
    if (color.isValid())
    {
        preferences.zoom_color = color;
        updateValues();
    }
}

void prefDlg::changeTitleColor()
{
    QColor color = QColorDialog::getColor(preferences.titleLabel_color, this);
    if (color.isValid())
    {
        preferences.titleLabel_color = color;
        updateValues();
        fontPrev->loadFont(titleFontCB->currentText(), preferences.titleLabel_color, titleSizeSB->value());
        fontPrev->update();
    }
}

void prefDlg::changeTitleSize()
{
    fontPrev->loadFont(titleFontCB->currentText(), preferences.titleLabel_color, titleSizeSB->value());
    preferences.titleLabel_size = titleSizeSB->value();
    fontPrev->update();
}

void prefDlg::changeAxisSize()
{
    preferences.axisLabel_size = axisSizeSB->value();
    fontPrev->loadFont(titleFontCB->currentText(), preferences.titleLabel_color, axisSizeSB->value());
    fontPrev->update();
}

void prefDlg::changeTitleFont(int index)
{
    (void) index;

    preferences.titleAxis_font = titleFontCB->currentText();
    fontPrev->loadFont(titleFontCB->currentText(), preferences.titleLabel_color, titleSizeSB->value());
    fontPrev->update();
}

void prefDlg::changeToolTipFont(int index)
{
    (void) index;

    preferences.toolTip_font = toolTipFontCB->currentText();
    fontPrev->loadFont(toolTipFontCB->currentText(), preferences.toolTip_color, toolTipSizeSB->value());
    fontPrev->update();
}

void prefDlg::changeToolTipSize()
{
    preferences.toolTip_char_size = toolTipSizeSB->value();
    fontPrev->loadFont(toolTipFontCB->currentText(), preferences.toolTip_color, toolTipSizeSB->value());
    fontPrev->update();
}

void prefDlg::changeToolTipLineSpace()
{
    preferences.toolTip_line_space = toolTipLineSpaceSB->value();
}

void prefDlg::changeToolTipColor()
{
    QColor color = QColorDialog::getColor(preferences.toolTip_color, this);
    if (color.isValid())
    {
        preferences.toolTip_color = color;
        updateValues();
        fontPrev->loadFont(toolTipFontCB->currentText(), color, toolTipSizeSB->value());
        fontPrev->update();
    }
}

void prefDlg::changeLegendFont(int index)
{
    (void) index;

    preferences.legend_font = legendFontCB->currentText();
    fontPrev->loadFont(legendFontCB->currentText(), QColor("Black"), legendSizeSB->value());
    fontPrev->update();
}

void prefDlg::changeLegendBckAreaColor()
{
    QColor color = QColorDialog::getColor(preferences.legend_background_color, this);
    if (color.isValid())
    {
        preferences.legend_background_color = color;
        updateValues();
    }
}

void prefDlg::changeLegendCharSize()
{
    preferences.legend_char_size = legendSizeSB->value();
    fontPrev->loadFont(legendFontCB->currentText(), invertColor(preferences.backGround_color), legendSizeSB->value());
    fontPrev->update();
}

void prefDlg::changeLegendLineSpace()
{
    preferences.legend_line_space = legendLineSpaceSB->value();
}

void prefDlg::changeStatsFont(int index)
{
    (void) index;

    preferences.stats_font = statsFontCB->currentText();
    fontPrev->loadFont(statsFontCB->currentText(), invertColor(preferences.backGround_color), statsSizeSB->value());
    fontPrev->update();
}

void prefDlg::changeStatsCharSize()
{
    preferences.stats_char_size = statsSizeSB->value();
    fontPrev->loadFont(statsFontCB->currentText(), invertColor(preferences.backGround_color), statsSizeSB->value());
    fontPrev->update();
}

void prefDlg::changeStatsLineSpace()
{
    preferences.stats_line_space = statsLineSpaceSB->value();
}


void prefDlg::createToolBar(QToolBar *toolBar)
{
    loadAct = new QAction("&Load from file...");
    loadAct->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    loadAct->setShortcut(QKeySequence::Open);
    loadAct->setStatusTip("Load preferences from file");
    connect(loadAct, &QAction::triggered, this, &prefDlg::loadPref);

    saveAct = new QAction("&Save to file...");
    saveAct->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    saveAct->setStatusTip("Save preferences to file...");
    connect(saveAct, &QAction::triggered, this, &prefDlg::savePref);

    exitAct = new QAction("&Close");
    exitAct->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    exitAct->setShortcut(QKeySequence::Quit);
    exitAct->setStatusTip("Close");
    connect(exitAct, &QAction::triggered, this, &prefDlg::close);

    toolBar->addAction(loadAct);
    toolBar->addAction(saveAct);
    toolBar->addSeparator();
    toolBar->addAction(exitAct);
    toolBar->setIconSize(QSize(32,32));
}

void prefDlg::createGridWidget()
{
    //WINDOW
    QLabel *bckGndColorLab = new QLabel("Background color:");
    bckGndColorPB = new QPushButton(this);
    bckGndColorPB->setAutoFillBackground(true); bckGndColorPB->setText("..."); bckGndColorPB->update();
    QHBoxLayout *hLbckGnd = new QHBoxLayout;
    hLbckGnd->addWidget(bckGndColorLab); hLbckGnd->addWidget(bckGndColorPB); hLbckGnd->addStretch();
    connect(bckGndColorPB, &QPushButton::clicked, this, &prefDlg::changeBackGroundColor);

    QLabel *plotwidthlabel = new QLabel("Window width:");
    plotwidthSB = new QSpinBox(this);
    plotwidthSB->setMinimum(50); plotwidthSB->setMaximum(4096);  //4K maximum
    connect(plotwidthSB, &QSpinBox::editingFinished, this, &prefDlg::changeWindowWidth);

    QLabel *plotheightlabel = new QLabel("Window height:");
    plotheightSB = new QSpinBox(this);
    plotheightSB->setMinimum(50); plotheightSB->setMaximum(4096);  //4K maximum
    connect(plotheightSB, &QSpinBox::editingFinished, this, &prefDlg::changeWindowHeight);

    QLabel *fontreslabel = new QLabel("Font rendering resolution (4 - 64):");
    fontresSB = new QSpinBox(this);
    fontresSB->setMinimum(4); fontresSB->setMaximum(64);
    connect(fontresSB, &QSpinBox::editingFinished, this, &prefDlg::changefontResolution);

    addFontSB = new QPushButton(this);
    addFontSB->setText("Add fonts..:");
    connect(addFontSB, &QPushButton::clicked, this, &prefDlg::addFonts);

    QVBoxLayout *layoutWindow = new QVBoxLayout;
    layoutWindow->addLayout(hLbckGnd);
    QVBoxLayout *v1 = new QVBoxLayout; QVBoxLayout *v2 = new QVBoxLayout;
    v1->addWidget(plotwidthlabel); v1->addWidget(plotwidthSB);
    v2->addWidget(plotheightlabel); v2->addWidget(plotheightSB);
    QHBoxLayout *hwdw1 = new QHBoxLayout; hwdw1->addLayout(v1); hwdw1->addLayout(v2);
    layoutWindow->addLayout(hwdw1);
    QVBoxLayout *v3 = new QVBoxLayout; v3->addWidget(fontreslabel); v3->addWidget(fontresSB);
    QVBoxLayout *v4 = new QVBoxLayout; v4->addStretch(); v4->addWidget(addFontSB);
    QHBoxLayout *hwdw2 = new QHBoxLayout; hwdw2->addLayout(v3); hwdw2->addLayout(v4);
    layoutWindow->addLayout(hwdw2);

    windowGB = new QGroupBox(this);
    windowGB->setTitle("Window properties");
    windowGB->setLayout(layoutWindow);

    //GRID
    QLabel *npointsLabel = new QLabel("Number of samples:");
    npointsSB = new QSpinBox(this);
    npointsSB->setMinimum(2); npointsSB->setMaximum(0x7FFFFFFF);
    connect(npointsSB, &QSpinBox::editingFinished, this, &prefDlg::changeNPoints);

    QLabel *maxYLabel = new QLabel("Y-Axis maximum:");
    maxYSB = new QDoubleSpinBox(this);
    maxYSB->setMinimum(std::numeric_limits<double>::max() * -1); maxYSB->setMaximum(std::numeric_limits<double>::max());
    connect(maxYSB, &QDoubleSpinBox::editingFinished, this, &prefDlg::changeMaxY);

    QLabel *minYLabel = new QLabel("Y-Axis minimum:");
    minYSB = new QDoubleSpinBox(this);
    minYSB->setMinimum(std::numeric_limits<double>::max() * -1); minYSB->setMaximum(std::numeric_limits<double>::max());
    connect(minYSB, &QDoubleSpinBox::editingFinished, this, &prefDlg::changeMinY);

    QLabel *gridYLabel = new QLabel("Y-Axis grid step:");
    gridYSB = new QDoubleSpinBox(this);
    gridYSB->setMinimum(std::numeric_limits<double>::max() * -1); gridYSB->setMaximum(std::numeric_limits<double>::max());
    connect(gridYSB, &QDoubleSpinBox::editingFinished, this, &prefDlg::changeGridY);

    QLabel *gridXLabel = new QLabel("X-Axis grid step:");
    gridXSB = new QDoubleSpinBox(this);
    gridXSB->setMinimum(std::numeric_limits<double>::max() * -1); gridXSB->setMaximum(std::numeric_limits<double>::max());
    connect(gridXSB, &QDoubleSpinBox::editingFinished, this, &prefDlg::changeGridX);

    QHBoxLayout *hLgridCol = new QHBoxLayout;
    QLabel *gridColLabel = new QLabel("Grid color:");
    gridColorPB = new QPushButton(this);
    gridColorPB->setAutoFillBackground(true); gridColorPB->setText("..."); gridColorPB->update();
    hLgridCol->addWidget(gridColLabel); hLgridCol->addWidget(gridColorPB); hLgridCol->addStretch();
    connect(gridColorPB, &QPushButton::clicked, this, &prefDlg::changeGridColor);

    QHBoxLayout *hLaxisCol = new QHBoxLayout;
    QLabel *axisColLabel = new QLabel("Axis color:");
    axisColorPB = new QPushButton(this);
    axisColorPB->setAutoFillBackground(true); axisColorPB->setText("..."); axisColorPB->update();
    hLaxisCol->addWidget(axisColLabel); hLaxisCol->addWidget(axisColorPB); hLaxisCol->addStretch();
    connect(axisColorPB, &QPushButton::clicked, this, &prefDlg::changeAxisColor);

    QLabel *gridLWLabel = new QLabel("Grid Line Width:");
    gridLWSB = new QSpinBox(this);
    gridLWSB->setMinimum(1); gridLWSB->setMaximum(0x7FFFFFFF);
    connect(gridLWSB, &QSpinBox::editingFinished, this, &prefDlg::changeGridLW);

    QLabel *axisLWLabel = new QLabel("Axis Line Width:");
    axisLWSB = new QSpinBox(this);
    axisLWSB->setMinimum(1); axisLWSB->setMaximum(0x7FFFFFFF);
    connect(axisLWSB, &QSpinBox::editingFinished, this, &prefDlg::changeAxisLW);

    QHBoxLayout *hLzoomCol = new QHBoxLayout;
    QLabel *zoomColLabel = new QLabel("Zoom area color:");
    zoomColorPB = new QPushButton(this);
    zoomColorPB->setAutoFillBackground(true); zoomColorPB->setText("..."); zoomColorPB->update();
    hLzoomCol->addWidget(zoomColLabel); hLzoomCol->addWidget(zoomColorPB); hLzoomCol->addStretch();
    connect(zoomColorPB, &QPushButton::clicked, this, &prefDlg::changeZoomColor);

    smoothCB = new QCheckBox(this);
    smoothCB->setText("Smooth axis and grid"); smoothCB->setCheckable(true);
    connect(smoothCB, &QCheckBox::toggled, this, &prefDlg::changeSmoothGrid);

    QVBoxLayout *layoutGrid = new QVBoxLayout;
    layoutGrid->addWidget(npointsLabel); layoutGrid->addWidget(npointsSB);
    QVBoxLayout *v5 = new QVBoxLayout; QVBoxLayout *v6 = new QVBoxLayout;
    v5->addWidget(maxYLabel);    v5->addWidget(maxYSB);
    v6->addWidget(minYLabel);    v6->addWidget(minYSB);
    QHBoxLayout *hg1 = new QHBoxLayout; hg1->addLayout(v5); hg1->addLayout(v6); layoutGrid->addLayout(hg1);
    QVBoxLayout *v7= new QVBoxLayout; QVBoxLayout *v8 = new QVBoxLayout;
    v7->addWidget(gridYLabel);   v7->addWidget(gridYSB);
    v8->addWidget(gridXLabel);   v8->addWidget(gridXSB);
    QHBoxLayout *hg2 = new QHBoxLayout; hg2->addLayout(v7); hg2->addLayout(v8); layoutGrid->addLayout(hg2);
    QVBoxLayout *v9= new QVBoxLayout; QVBoxLayout *v10 = new QVBoxLayout;
    v9->addWidget(gridLWLabel);  v9->addWidget(gridLWSB);
    v10->addWidget(axisLWLabel);  v10->addWidget(axisLWSB);
    QHBoxLayout *hg3 = new QHBoxLayout; hg3->addLayout(v9); hg3->addLayout(v10); layoutGrid->addLayout(hg3);
    QHBoxLayout *h1 = new QHBoxLayout;
    h1->addLayout(hLgridCol); h1->addLayout(hLaxisCol);
    layoutGrid->addLayout(h1);
    QHBoxLayout *hg4 = new QHBoxLayout; hg4->addWidget(smoothCB); hg4->addLayout(hLzoomCol); layoutGrid->addLayout(hg4);

    gridGB = new QGroupBox(this);
    gridGB->setTitle("Grid properties");
    gridGB->setLayout(layoutGrid);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(windowGB);
    mainLayout->addWidget(gridGB);
    mainLayout->addStretch();

    gridWidget->setLayout(mainLayout);
}

void prefDlg::createTitleWidget()
{
    //Title
    QLabel *titleSizeLabel = new QLabel("Size of the title label:");
    titleSizeSB = new QSpinBox(this);
    titleSizeSB->setMinimum(2); titleSizeSB->setMaximum(4096);
    connect(titleSizeSB, &QSpinBox::editingFinished, this, &prefDlg::changeTitleSize);

    QLabel *axisSizeLabel = new QLabel("Size of the axis label:");
    axisSizeSB = new QSpinBox(this);
    axisSizeSB->setMinimum(2); axisSizeSB->setMaximum(4096);
    connect(axisSizeSB, &QSpinBox::editingFinished, this, &prefDlg::changeAxisSize);

    QLabel *titleFontLabel = new QLabel("Title font:");
    titleFontCB = new QComboBox(this);
    titleFontCB->addItems(fontsList);
    connect(titleFontCB, SIGNAL (currentIndexChanged(int)), this, SLOT (changeTitleFont(int)));

    QLabel *titleColorLabel = new QLabel("Title color:");
    titleColorPB = new QPushButton(this);
    titleColorPB->setAutoFillBackground(true); titleColorPB->setText("..."); titleColorPB->update();
    QHBoxLayout *hLtColLab = new QHBoxLayout;
    hLtColLab->addWidget(titleColorLabel); hLtColLab->addWidget(titleColorPB); hLtColLab->addStretch();
    connect(titleColorPB, &QPushButton::clicked, this, &prefDlg::changeTitleColor);

    QVBoxLayout *layoutTitle = new QVBoxLayout;
    layoutTitle->addWidget(titleSizeLabel);    layoutTitle->addWidget(titleSizeSB);
    layoutTitle->addWidget(axisSizeLabel);     layoutTitle->addWidget(axisSizeSB);
    layoutTitle->addWidget(titleFontLabel);    layoutTitle->addWidget(titleFontCB);       layoutTitle->addLayout(hLtColLab);

    titleGB = new QGroupBox(this);
    titleGB->setTitle("Title and Axis properties:");
    titleGB->setLayout(layoutTitle);

    //ToolTip
    QLabel *toolTipSizeLabel = new QLabel("Size of the tooltip label:");
    toolTipSizeSB = new QSpinBox(this);
    toolTipSizeSB->setMinimum(2); toolTipSizeSB->setMaximum(4096);
    connect(toolTipSizeSB, &QSpinBox::editingFinished, this, &prefDlg::changeToolTipSize);

    QLabel *toolTipLSLabel = new QLabel("Size of the tooltip line-space:");
    toolTipLineSpaceSB = new QSpinBox(this);
    toolTipLineSpaceSB->setMinimum(2); toolTipLineSpaceSB->setMaximum(4096);
    connect(toolTipLineSpaceSB, &QSpinBox::editingFinished, this, &prefDlg::changeToolTipLineSpace);

    QLabel *tooltipFontLabel = new QLabel("Title font:");
    toolTipFontCB = new QComboBox(this);
    toolTipFontCB->addItems(fontsList);
    connect(toolTipFontCB, SIGNAL (currentIndexChanged(int)), this, SLOT (changeToolTipFont(int)));

    QLabel *toolTipColorLabel = new QLabel("Tooltip color:");
    toolTipColorPB = new QPushButton(this);
    toolTipColorPB->setAutoFillBackground(true); toolTipColorPB->setText("..."); toolTipColorPB->update();
    QHBoxLayout *hLTTColLab = new QHBoxLayout;
    hLTTColLab->addWidget(toolTipColorLabel); hLTTColLab->addWidget(toolTipColorPB); hLTTColLab->addStretch();
    connect(toolTipColorPB, &QPushButton::clicked, this, &prefDlg::changeToolTipColor);

    QVBoxLayout *layoutToolTip = new QVBoxLayout;
    layoutToolTip->addWidget(toolTipSizeLabel);    layoutToolTip->addWidget(toolTipSizeSB);
    layoutToolTip->addWidget(toolTipLSLabel);      layoutToolTip->addWidget(toolTipLineSpaceSB);
    layoutToolTip->addWidget(tooltipFontLabel);    layoutToolTip->addWidget(toolTipFontCB);       layoutToolTip->addLayout(hLTTColLab);

    toolTipGB = new QGroupBox(this);
    toolTipGB->setTitle("Tooltip properties:");
    toolTipGB->setLayout(layoutToolTip);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(titleGB);
    mainLayout->addWidget(toolTipGB);
    mainLayout->addStretch();

    titleWidget->setLayout(mainLayout);
}

void prefDlg::createLegendWidget()
{
    //Legend
    QLabel *legendSizeLabel = new QLabel("Size of the legend label:");
    legendSizeSB = new QSpinBox(this);
    legendSizeSB->setMinimum(2); legendSizeSB->setMaximum(4096);
    connect(legendSizeSB, &QSpinBox::editingFinished, this, &prefDlg::changeLegendCharSize);

    QLabel *legendLSLabel = new QLabel("Size of the legend line-space:");
    legendLineSpaceSB = new QSpinBox(this);
    legendLineSpaceSB->setMinimum(2); legendLineSpaceSB->setMaximum(4096);
    connect(legendLineSpaceSB, &QSpinBox::editingFinished, this, &prefDlg::changeLegendLineSpace);

    QLabel *legendFontLabel = new QLabel("Legend font:");
    legendFontCB = new QComboBox(this);
    legendFontCB->addItems(fontsList);
    connect(legendFontCB, SIGNAL (currentIndexChanged(int)), this, SLOT (changeLegendFont(int)));

    QLabel *legendColorLabel = new QLabel("Legend background color:");
    legendBckColorPB = new QPushButton(this);
    legendBckColorPB->setAutoFillBackground(true); legendBckColorPB->setText("..."); legendBckColorPB->update();
    QHBoxLayout *hLlColLab = new QHBoxLayout;
    hLlColLab->addWidget(legendColorLabel); hLlColLab->addWidget(legendBckColorPB);
    connect(legendBckColorPB, &QPushButton::clicked, this, &prefDlg::changeLegendBckAreaColor);

    QVBoxLayout *layoutLegend = new QVBoxLayout;
    layoutLegend->addWidget(legendSizeLabel);    layoutLegend->addWidget(legendSizeSB);
    layoutLegend->addWidget(legendLSLabel);      layoutLegend->addWidget(legendLineSpaceSB);
    layoutLegend->addWidget(legendFontLabel);    layoutLegend->addWidget(legendFontCB);       layoutLegend->addLayout(hLlColLab);

    legendGB = new QGroupBox(this);
    legendGB->setTitle("Legend properties:");
    legendGB->setLayout(layoutLegend);

    //Stats
    QLabel *statsSizeLabel = new QLabel("Size of the statistics label:");
    statsSizeSB = new QSpinBox(this);
    statsSizeSB->setMinimum(2); statsSizeSB->setMaximum(4096);
    connect(statsSizeSB, &QSpinBox::editingFinished, this, &prefDlg::changeStatsCharSize);

    QLabel *statsLSLabel = new QLabel("Size of the statistics line-space:");
    statsLineSpaceSB = new QSpinBox(this);
    statsLineSpaceSB->setMinimum(2); statsLineSpaceSB->setMaximum(4096);
    connect(statsLineSpaceSB, &QSpinBox::editingFinished, this, &prefDlg::changeStatsLineSpace);

    QLabel *statsFontLabel = new QLabel("Statistics font:");
    statsFontCB = new QComboBox(this);
    statsFontCB->addItems(fontsList);
    connect(statsFontCB, SIGNAL (currentIndexChanged(int)), this, SLOT (changeStatsFont(int)));

    QVBoxLayout *layoutStats = new QVBoxLayout;
    layoutStats->addWidget(statsSizeLabel);    layoutStats->addWidget(statsSizeSB);
    layoutStats->addWidget(statsLSLabel);      layoutStats->addWidget(statsLineSpaceSB);
    layoutStats->addWidget(statsFontLabel);    layoutStats->addWidget(statsFontCB);

    statsGB = new QGroupBox(this);
    statsGB->setTitle("Statistics properties:");
    statsGB->setLayout(layoutStats);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(legendGB);
    mainLayout->addWidget(statsGB);
    mainLayout->addStretch();

    legendWidget->setLayout(mainLayout);
}

QColor prefDlg::invertColor(QColor in)
{
    double r, g, b, a;
    QColor out;

    in.getRgbF(&r, &g, &b, &a);
    out.setRedF(fabs(1.0 - r));
    out.setGreenF(fabs(1.0 - g));
    out.setBlueF(fabs(1.0 - b));
    out.setAlphaF(a);

    return out;
}

void prefDlg::fillFontList()
{
    int i;

    for (i = 0; i < fontMgr->getFontList().count(); i++)
        fontsList.append(fontMgr->getFontList()[i]);
}

void prefDlg::updateFontList()
{
    //clear the items from the comboboxes
    titleFontCB->clear(); toolTipFontCB->clear();
    legendFontCB->clear(); statsFontCB->clear();
    //and populate them again
    fontsList.clear();
    fillFontList();
    titleFontCB->addItems(fontsList); toolTipFontCB->addItems(fontsList);
    legendFontCB->addItems(fontsList); statsFontCB->addItems(fontsList);
}

void prefDlg::updateValues()
{
    bckGndColorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(preferences.backGround_color.name()).arg(invertColor(preferences.backGround_color).name()));
    bckGndColorPB->update();
    plotwidthSB->setValue(preferences.plot_width_size);
    plotheightSB->setValue(preferences.plot_height_size);
    fontresSB->setValue(preferences.font_resolution);

    npointsSB->setValue(preferences.N_points);
    maxYSB->setValue(static_cast<double>(preferences.max_Y));
    minYSB->setValue(static_cast<double>(preferences.min_Y));
    gridYSB->setValue(static_cast<double>(preferences.grid_Y));
    gridXSB->setValue(static_cast<double>(preferences.grid_X));
    gridColorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(preferences.grid_color.name()).arg(invertColor(preferences.grid_color).name()));
    gridColorPB->update();
    axisColorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(preferences.axis_color.name()).arg(invertColor(preferences.axis_color).name()));
    axisColorPB->update();
    gridLWSB->setValue(static_cast<int>(static_cast<int>(preferences.grid_linewidth * 10000)));
    axisLWSB->setValue(static_cast<int>(static_cast<int>(preferences.axis_linewidth * 10000)));
    zoomColorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(preferences.zoom_color.name()).arg(invertColor(preferences.zoom_color).name()));
    zoomColorPB->update();
    smoothCB->setChecked(preferences.smoothGrid);

    titleSizeSB->setValue(preferences.titleLabel_size);
    axisSizeSB->setValue(preferences.axisLabel_size);
    titleColorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(preferences.titleLabel_color.name()).arg(invertColor(preferences.titleLabel_color).name()));
    titleColorPB->update();

    toolTipSizeSB->setValue(preferences.toolTip_char_size);
    toolTipLineSpaceSB->setValue(preferences.toolTip_line_space);
    toolTipColorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(preferences.toolTip_color.name()).arg(invertColor(preferences.toolTip_color).name()));
    toolTipColorPB->update();

    legendSizeSB->setValue(preferences.legend_char_size);
    legendLineSpaceSB->setValue(preferences.legend_line_space);
    legendBckColorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(preferences.legend_background_color.name()).arg(invertColor(preferences.legend_background_color).name()));
    legendBckColorPB->update();

    statsSizeSB->setValue(preferences.stats_char_size);
    statsLineSpaceSB->setValue(preferences.stats_line_space);

    titleFontCB->setCurrentIndex(findFont(legendFontCB, preferences.titleAxis_font));
    toolTipFontCB->setCurrentIndex(findFont(statsFontCB, preferences.toolTip_font));
    legendFontCB->setCurrentIndex(findFont(legendFontCB, preferences.legend_font));
    statsFontCB->setCurrentIndex(findFont(statsFontCB, preferences.stats_font));
}

int prefDlg::findFont(QComboBox *ptr, QString ref)
{
    int i;

    for (i = 0; i < ptr->count(); i++)
    {
        if (QString::compare(static_cast<QString>(ptr->itemText(i)), ref, Qt::CaseInsensitive) == 0)
            return i;
    }
    return 0;
}
