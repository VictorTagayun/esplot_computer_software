/**
  *********************************************************************************************************************************************************
  @file     :grid_xy.h
  @brief    :Header of the grid class for xy plots
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

#ifndef GRID_XY_H
#define GRID_XY_H

#include <qopengl.h>
#include <QVector>
#include <QVector3D>
#include <QColor>
#include <QString>
#include <QLocale>
#include <QMatrix4x4>
#include <QVector4D>

#include "Managers/prefmanager.h"
#include "Creators/textrenderer.h"
#include "FontManager/fontmanager.h"

//VERTEX DESCRIPTION
//  X  Y X_AXIS  R  G  B ALFA LW  CF  ORD   (linewidth conversionfactor order (if order = 0 use as float, if order > 0 use as integer)

class Grid_XY
{
public:
    Grid_XY(appPreferencesStruct *pref, QString title, int width, int height, fontManager *font);
    ~Grid_XY();
    const GLfloat *Data() { return grid_data.constData(); }
    QVector<GLfloat> *getTextData() { return &text_data; }
    int count() { return data_count; }
    int vertexCount() { return data_count / 2; }
    void setTitle(QString title) { plotTitle = title; }

    void setTransformationMatrix(QMatrix4x4 mat) { transfMatrix = mat; }

    void setDrawGrid(bool enable) { drawGrid = enable; }
    bool getDrawGrid() { return drawGrid; }
    void setDrawTitle(bool enable) { drawTitle = enable; }
    void setDrawYLabels(bool enable) { drawYLabels = enable; }
    void setDrawXLabels(bool enable) { drawXLabels = enable; }

    void recreate_Grid(float max_X, float min_X, float max_Y, float min_Y);
    void recreate_Grid();
    void update_Text(int width, int height);

    float get_ConvFact_X() { return conversion_factor_x; }
    float get_ConvFact_Y() { return conversion_factor_y; }
    float get_X_Axis() { return x_axis_position; }
    float get_Y_Axis() { return y_axis_position; }
    QImage getFontTexture() { return fontTexture; }

    float get_min_x() { return min_x; }
    float get_max_x() { return max_x; }
    float get_min_y() { return min_y; }
    float get_max_y() { return max_y; }
    float get_grid_y() { return grid_y; }
    float get_grid_x() { return grid_x; }

    void set_max_x(float max) { max_x = max; }
    void set_min_x(float min) { min_x = min; }
    void set_max_y(float max) { max_y = max; }
    void set_min_y(float min) { min_y = min; }
    void set_grid_x(float gridx) { grid_x = gridx; }
    void set_grid_y(float gridy) { grid_y = gridy; }
    void set_axis_color(QColor color);
    void set_grid_color(QColor color);
    void set_axis_width(GLfloat width);
    void set_grid_width(GLfloat width);
    void set_base_time(float time) { if (time > 0) base_time = time; }
    void set_x_grid_ratio(int ratio);
    void set_y_grid_ratio(int ratio);

    void updateFont();

private:
    appPreferencesStruct *preferences;
    fontManager *fontMgr;

    QMatrix4x4 transfMatrix;

    int xGridRatio;
    int yGridRatio;

    QVector<GLfloat> grid_data;
    QVector<GLfloat> text_data;
    int data_count;

    textRenderer *txtRnd;

    QString plotTitle;
    QString title_fontname;

    QImage fontTexture;

    bool drawGrid;
    bool drawTitle;
    bool drawYLabels;
    bool drawXLabels;

    int titleLabel_size;
    int axisLabel_size;

    float max_x, min_x;  //maximum and minimum value of the X axis
    float max_y, min_y;  //maximum and minimum value of the Y axis
    float grid_x, grid_y;  //interval for the grid

    QVector<float> y_ticks_value;
    QVector<float> y_ticks_pos;
    QVector<float> x_ticks_value;
    QVector<float> x_ticks_pos;
    float base_time;

    QColor color_axis;
    QColor color_grid;
    QColor axisLabelColor;
    QColor titleLabelColor;

    GLfloat axis_width;
    GLfloat grid_width;

    float x_axis_position;  //Y-index of the x_axis
    float y_axis_position;  //X-index of the y_axis
    float conversion_factor_x;  //transforms input data into x_positions
    float conversion_factor_y;  //transforms input data into y_positions

    const float alfa_axis = 1.0f;  //transparence
    const float alfa_grid = 0.7f;

    int screen_width, screen_height;

    bool smooth_grid;
    bool smooth_axes;

    void prepare_Axis();
    void prepare_Grid();
    void prepare_Text();
    void add_in_buffer(float x, float y);

    GLfloat abs_GLfloat(GLfloat value);

    void prepare_grid_data();

    float getMinYAxisGrid(int ratio);
    float getMinXAxisGrid(int ratio);
};

#endif // GRID_H
