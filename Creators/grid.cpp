/**
  *********************************************************************************************************************************************************
  @file     :grid.cpp
  @brief    :Functions for drawing a grid for time plots
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

#include "grid.h"

#include <QElapsedTimer>
#include <QDebug>

#include <math.h>

Grid::Grid(appPreferencesStruct *pref, QString title, int width, int height, fontManager *font)
    : data_count(0)
{
    preferences = pref;
    fontMgr = font;

    transfMatrix.setToIdentity();

    drawGrid = true;  //by default
    drawTitle = false;  //by default
    drawYLabels = true;  //by default
    drawXLabels = true;  //by default

    n_points = abs(pref->N_points);
    if (n_points <= 1) n_points = 2;  //by default

    max_y = pref->max_Y;
    min_y = pref->min_Y;

    if (max_y <= min_y)
        max_y = min_y + 100;  //by default

    grid_y = abs_GLfloat(pref->grid_Y);
    grid_x = abs_GLfloat(pref->grid_X);

    xGridRatio = 20; yGridRatio = 20;

    plotTitle = title;
    title_fontname = preferences->titleAxis_font;

    screen_width = width;
    screen_height = height;

    txtRnd = new textRenderer(title_fontname, fontMgr);

    prepare_grid_data();
}

Grid::~Grid()
{
    delete txtRnd;
}

void Grid::recreate_Grid()
{
    grid_data.clear();  //deletes the actual buffer
    prepare_grid_data();
}

void Grid::recreate_Grid(float max_Y, float min_Y)
{
    grid_data.clear();  //deletes the actual buffer
    max_y = max_Y;
    min_y = min_Y;

    if (max_y <= min_y)
        max_y = min_y + 100;  //by default

    prepare_grid_data();
}

void Grid::update_Text(int width, int height)
{
    screen_width = width;
    screen_height = height;

    prepare_Text();
}

void Grid::set_x_grid_ratio(int ratio)
{
    if ((ratio >= 5) && (ratio <= 200))
        xGridRatio = ratio;
}

void Grid::set_y_grid_ratio(int ratio)
{
    if ((ratio >= 5) && (ratio <= 200))
        yGridRatio = ratio;
}

void Grid::updateFont()
{
    delete txtRnd;

    title_fontname = preferences->titleAxis_font;
    txtRnd = new textRenderer(title_fontname, fontMgr);
}

void Grid::prepare_Axis()
{
    //Add Y-Axis === This is always fixed in position on the screen => no changes
    if (smooth_axes == true)
    {
        add_in_buffer(-0.95f, -0.95f);
        add_in_buffer(-0.95f, 0.95f);
    }
    else
    {
        add_in_buffer(-0.95f, -0.95f);
        add_in_buffer(-0.95f, 0.95f);
    }


    //Add X-Axis => if the x_axis_position is out of bondary we don't plot it
    if ((x_axis_position <= 1.0f) || (x_axis_position >= -1.0f))
    {
        if (smooth_axes == true)
        {
            add_in_buffer(-0.95f, x_axis_position);
            add_in_buffer(0.95f, x_axis_position);
        }
        else
        {
            add_in_buffer(-0.95f, x_axis_position);
            add_in_buffer(0.95f, x_axis_position);
        }
    }
}

void Grid::prepare_Grid()
{
    bool out_of_bound;
    float step, index;
    float y_value;
    int i;

    y_ticks_pos.clear(); y_ticks_value.clear();
    x_ticks_pos.clear(); x_ticks_value.clear();

    //Y_axis lines
    //We go from min_y to max_y casting the y_axis grid lines
    y_value = static_cast<float>(static_cast<int>(min_y / grid_y)) * grid_y;  //this is the first index from where we start => now we go up until we go out of bound
    y_value = getMinYAxisGrid(yGridRatio);  //this changes also grid_y
    out_of_bound = false;
    while(out_of_bound == false)
    {
        index = (y_value * conversion_factor) + x_axis_position;
        y_ticks_value.append(y_value);
        y_ticks_pos.append(index);
        add_in_buffer(-0.95f, index);
        add_in_buffer(0.95f, index);

        y_value += grid_y;
        if (y_value > max_y)
            out_of_bound = true;
    }

    //X_axis lines
    //We start at -0.95f as default and go on until the number of points
    out_of_bound = false;
    getMinXAxisGrid(xGridRatio);  //this updates grid_x
    step = step_x * (grid_x / base_time);
    index = -0.95f;
    i = 0;
    while (out_of_bound == false)
    {
        x_ticks_pos.append(index); x_ticks_value.append(static_cast<float>(i) * grid_x); i++;
        index = index + step;
        if (index <= 0.95f)
        {
            add_in_buffer(index, -0.95f);
            add_in_buffer(index, 0.95f);
        }
        else
            out_of_bound = true;
    }
    x_ticks_pos.append(index); x_ticks_value.append(static_cast<float>(i) * grid_x * base_time);
}

void Grid::prepare_Text()
{
    //Here we prepare the text to be plotted => y_min, y_max, Title
    text_data.clear();
    QLocale locale;
    float x, y, x1;
    int i;
    float x_ax;
    QVector4D vec;
    QVector4D res;

    vec.setZ(0.0f); vec.setW(1.0f);

    if (drawGrid == true)
    {
        if (drawYLabels == true)
        {
            x = -0.95f;
            vec.setX(x); vec.setY(0.0); res = transfMatrix * vec; x = res.x();
            if (x < -0.95f) x = -0.95f;
            if (x > 0.95f) x = 0.95f;
            //Render y_ticks
            vec.setX(x);
            for (i = 0; i < y_ticks_pos.count(); i++)
            {
                vec.setY(y_ticks_pos[i]); res = transfMatrix * vec;
                txtRnd->prepareText(x, res.y(), locale.toString(y_ticks_value[i], 'g', 3), preferences->axis_color, preferences->axisLabel_size, screen_width, screen_height, true);
                text_data.append(*txtRnd->getTextBuffer());
            }
        }

        if (drawXLabels == true)
        {
            y = -0.95f;

            vec.setX(0.0f); vec.setY(x_axis_position);
            res = transfMatrix * vec;
            x_ax = res.y();
            if (x_ax > 0.95f)
                x_ax = 0.95f;
            if (x_ax < -0.95f)
                x_ax = -0.95f;
            vec.setY(x_ax);
            //Render x_ticks
            for (i = 0; i < x_ticks_pos.count(); i++)
            {
                vec.setX(x_ticks_pos[i]); res = transfMatrix * vec;
                txtRnd->prepareText(res.x(), x_ax, locale.toString(x_ticks_value[i], 'g', 3), preferences->axis_color, preferences->axisLabel_size, screen_width, screen_height, true);
                text_data.append(*txtRnd->getTextBuffer());
            }
        }
    }

    if (drawTitle == true)
    {
        //Render title
        x = 0.0f; y = 0.975f;
        x1 = txtRnd->prepareText(x, y, plotTitle, titleLabelColor, titleLabel_size, screen_width, screen_height, true);
        x = x - ((x1 - x) / 2.0f);  //centers the title
        txtRnd->prepareText(x, y, plotTitle, preferences->titleLabel_color, preferences->titleLabel_size, screen_width, screen_height, true);
        text_data.append(*txtRnd->getTextBuffer());
    }

    fontTexture = txtRnd->getTexture();
}

void Grid::add_in_buffer(float x, float y)
{
    //Append in data
    grid_data.append(x); grid_data.append(y);

    data_count = grid_data.count();
}

GLfloat Grid::abs_GLfloat(GLfloat value)
{
    if (value < 0)
        return value * (-1);
    else
        return value;
}

void Grid::prepare_grid_data()
{
    grid_data.clear();

    if (max_y <= min_y)
        max_y = min_y + 100;  //by default

    color_axis = preferences->axis_color;
    color_grid = preferences->grid_color;
    axisLabelColor = color_axis;
    titleLabelColor = preferences->titleLabel_color;

    axis_width = preferences->axis_linewidth;
    grid_width = preferences->grid_linewidth;

    smooth_axes = preferences->smoothGrid;
    smooth_grid = preferences->smoothGrid;

    titleLabel_size = preferences->titleLabel_size;
    axisLabel_size = preferences->axisLabel_size;

    //Calculate the X-Axis position and calculates the conversion factor
    conversion_factor = (1.90f) / (abs_GLfloat(max_y - min_y));
    //we know now the conversion factor => i can obtain the
    x_axis_position = -0.95f - (min_y * conversion_factor);
    //calculate the step along the x_axis
    step_x = 1.9f / (n_points - 1);

    if (drawGrid == true)
        prepare_Grid();
    prepare_Axis();
    prepare_Text();
}

float Grid::getMinYAxisGrid(int ratio)
{
    double diff, diff_log;
    double tick_size;

    double log_10 = 2.30258509299;

    if (ratio < 5)
        ratio = 5;
    if (ratio > 200)
        ratio = 200;

    diff = static_cast<double>(max_y - min_y);
    diff_log = floor(log(diff)/log_10);
    tick_size = pow(10, diff_log) / 100;

    if (diff / tick_size < (ratio / 5))
        tick_size /= 5;
    else if (diff / tick_size < ratio / 2)
          tick_size = tick_size / 2;
    else if (diff / tick_size < ratio * 2)
          tick_size = tick_size * 2;
    else if (diff / tick_size < ratio * 5)
          tick_size = tick_size * 5;
    else if (diff / tick_size < ratio * 10)
          tick_size = tick_size * 10;
    else if (diff / tick_size < ratio * 20)
          tick_size = tick_size * 20;
    else if (diff / tick_size < ratio * 50)
          tick_size = tick_size * 50;
    else if (diff / tick_size < ratio * 100)
          tick_size = tick_size * 100;
    else if (diff / tick_size < ratio * 200)
          tick_size = tick_size * 200;

    grid_y = static_cast<float>(tick_size);
    return static_cast<float>(floor(static_cast<double>(min_y) / tick_size) * tick_size);
}

float Grid::getMinXAxisGrid(int ratio)
{
    double diff, diff_log;
    double tick_size;

    double log_10 = 2.30258509299;

    if (ratio < 5)
        ratio = 5;
    if (ratio > 200)
        ratio = 200;

    diff = static_cast<double>(n_points * base_time);
    diff_log = floor(log(diff)/log_10);
    tick_size = pow(10, diff_log) / 100;

    if (diff / tick_size < (ratio / 5))
        tick_size /= 5;
    else if (diff / tick_size < ratio / 2)
          tick_size = tick_size / 2;
    else if (diff / tick_size < ratio * 2)
          tick_size = tick_size * 2;
    else if (diff / tick_size < ratio * 5)
          tick_size = tick_size * 5;
    else if (diff / tick_size < ratio * 10)
          tick_size = tick_size * 10;
    else if (diff / tick_size < ratio * 20)
          tick_size = tick_size * 20;
    else if (diff / tick_size < ratio * 50)
          tick_size = tick_size * 50;
    else if (diff / tick_size < ratio * 100)
          tick_size = tick_size * 100;
    else if (diff / tick_size < ratio * 200)
          tick_size = tick_size * 200;


    grid_x = static_cast<float>(tick_size);
    return 0;
}
