/**
  *********************************************************************************************************************************************************
  @file     :premanager.cpp
  @brief    :Functions of the Preferences Manager class
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

#include "prefmanager.h"
#include <QDebug>

prefManager::prefManager()
{
    load_Default();
}

prefManager::prefManager(QString filename)
{
    int ret = loadFromFile(filename);
    if (ret == -1)
        qDebug() << "Error loading file " << filename;
}

int prefManager::saveOnFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return -1;

    QDataStream stream(&file);  //ready to write

    stream << preferences.backGround_color << preferences.plot_width_size << preferences.plot_height_size << preferences.font_resolution;

    stream << preferences.N_points << preferences.max_Y << preferences.min_Y << preferences.grid_Y << preferences.grid_X;
    stream << preferences.grid_color << preferences.axis_color << preferences.grid_linewidth << preferences.axis_linewidth << preferences.smoothGrid;

    stream << preferences.zoom_color;

    stream << preferences.titleLabel_color << preferences.titleLabel_size << preferences.axisLabel_size << preferences.titleAxis_font;

    stream << preferences.legend_font << preferences.legend_background_color << preferences.legend_char_size << preferences.legend_line_space;

    stream << preferences.stats_font << preferences.stats_char_size << preferences.stats_line_space;

    stream << preferences.toolTip_font << preferences.toolTip_char_size << preferences.toolTip_line_space << preferences.toolTip_color;

    file.close();

    return 0;
}

int prefManager::loadFromFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Error loading...";
        return -1;
    }

    QDataStream stream(&file);  //ready to read

    stream >> preferences.backGround_color >> preferences.plot_width_size >> preferences.plot_height_size >> preferences.font_resolution;

    stream >> preferences.N_points >> preferences.max_Y >> preferences.min_Y >> preferences.grid_Y >> preferences.grid_X;
    stream >> preferences.grid_color >> preferences.axis_color >> preferences.grid_linewidth >> preferences.axis_linewidth >> preferences.smoothGrid;

    stream >> preferences.zoom_color;

    stream >> preferences.titleLabel_color >> preferences.titleLabel_size >> preferences.axisLabel_size >> preferences.titleAxis_font;

    stream >> preferences.legend_font >> preferences.legend_background_color >> preferences.legend_char_size >> preferences.legend_line_space;

    stream >> preferences.stats_font >> preferences.stats_char_size >> preferences.stats_line_space;

    stream >> preferences.toolTip_font >> preferences.toolTip_char_size >> preferences.toolTip_line_space >> preferences.toolTip_color;

    file.close();

    return 0;
}

appPreferencesStruct prefManager::load_Default()
{
    preferences.backGround_color = QColor("White");
    preferences.plot_height_size = 200;
    preferences.plot_width_size = 200;
    preferences.font_resolution = 32;

    preferences.N_points = 1000;
    preferences.max_Y = 100.0;
    preferences.min_Y = -100.0;
    preferences.grid_X = 10.0;
    preferences.grid_Y = 10.0;
    preferences.grid_color = QColor("Grey");
    preferences.axis_color = QColor("Grey");
    preferences.grid_linewidth = 2.0f;
    preferences.axis_linewidth = 2.0f;
    preferences.smoothGrid = false;

    preferences.zoom_color = QColor("Blue");

    preferences.titleLabel_size = 16;
    preferences.axisLabel_size = 10;
    preferences.titleAxis_font = "Euler";
    preferences.titleLabel_color = QColor("Black");

    preferences.legend_font = "Euler";
    preferences.legend_background_color = QColor("Grey");
    preferences.legend_char_size = 16;
    preferences.legend_line_space = 10;

    preferences.stats_font = "Euler";
    preferences.stats_char_size = 20;
    preferences.stats_line_space = 12;

    preferences.toolTip_font = "Euler";
    preferences.toolTip_char_size = 14;
    preferences.toolTip_line_space = 10;
    preferences.toolTip_color = QColor("Black");

    return preferences;
}
