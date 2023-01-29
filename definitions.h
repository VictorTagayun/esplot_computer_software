/**
  *********************************************************************************************************************************************************
  @file     :definitions.h
  @brief    :Definition of the structs used for plotting
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

#ifndef DEFINITIONS_H 
#define DEFINITIONS_H

#include <QVector>
#include <QString>
#include <QAction>

QT_FORWARD_DECLARE_CLASS(plot_Window)
QT_FORWARD_DECLARE_CLASS(xy_plot_Window)

typedef struct _SignalInfo
{
    uint32_t signal_ID;
    QColor signal_color;
    float line_width;
    bool visible;
} SignalInfo;

typedef struct _statisticInfo
{
    bool initialized;  //says whether min and max have been initialized
    float min, max, mean;
    long n_samples;  //number of samples used on the mean
} statisticInfo;

typedef struct _SigProperty
{
    uint32_t index;
    bool lineRendering;
    bool dotRendering;
    QString name;
    QColor color;
    statisticInfo stats;
    QAction *triggerAct;
    float line_width;
} SigProperty;

typedef struct _XY_SigProperty
{
    uint32_t x_index;
    uint32_t y_index;
    bool lineRendering;
    bool dotRendering;
    QString name;
    QColor color;
    float line_width;
} XY_SigProperty;

typedef struct _Plot_Structure
{
    QString title;  //title of the plotter
    uint32_t index;  //index of the plotter
    QVector<SignalInfo> signals_associated;  //contains info about the associated info
    plot_Window *plot;  //this links to the plotting window
    bool closed;  //it says if the window has been closed or not
} Plot_Structure;

typedef struct _XY_Plot_Structure
{
    QString title;  //title of the plotter
    uint32_t index;  //index of the plotter
    QVector<SignalInfo> x_signals_associated;
    QVector<SignalInfo> y_signals_associated;
    xy_plot_Window *plot;  //this links to the plotting window
    bool closed;
} XY_Plot_Structure;

#endif // DEFINITIONS_H
