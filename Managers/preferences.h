/**
  *********************************************************************************************************************************************************
  @file     :preferences.h
  @brief    :Definition of the available preferences
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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QColor>
#include <QString>
#include <QVector>

typedef struct _appPreferences
{
    //PLOT
    QColor backGround_color;
    int plot_width_size;
    int plot_height_size;
    int font_resolution;

    //GRID
    int N_points;
    float max_Y;
    float min_Y;
    float grid_Y;
    float grid_X;
    QColor grid_color;
    QColor axis_color;
    float grid_linewidth;
    float axis_linewidth;
    bool smoothGrid;

    QColor zoom_color;

    //TITLE
    QColor titleLabel_color;
    int titleLabel_size;
    int axisLabel_size;
    QString titleAxis_font;

    //LEGEND
    QString legend_font;
    QColor legend_background_color;
    int legend_char_size;
    int legend_line_space;

    //STATS
    QString stats_font;
    int stats_char_size;
    int stats_line_space;

    //TOOLTIP
    QString toolTip_font;
    int toolTip_char_size;
    int toolTip_line_space;
    QColor toolTip_color;
} appPreferencesStruct;


#endif // PREFERENCES_H
