/**
  *********************************************************************************************************************************************************
  @file     :legendCreator.h
  @brief    :Header for legendCreator class
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

#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QImage>
#include <QFont>

#include <math.h>

#include "textrenderer.h"
#include "FontManager/fontmanager.h"
#include "definitions.h"

class legendCreator
{
public:
    legendCreator(QString fontName, fontManager *font);
    ~legendCreator();

    void setSignals(QVector<SigProperty> properties) { sig_properties = properties; }
    void setCharSize(int size) { if (char_Size >= 1) char_Size = size; }
    void setScreenSize(int width, int height);
    void setLineSpace(int space) { if (space >= 0) line_space = space; }
    void setBackGroundColor(QColor color) { backGround = color; }

    void createLegend(float x, float y);  //creates the legend starting to write at x, y intended topLeft
    QVector<float> *getLegendData() { return &legendData; }
    QVector<float> *getLegendAreaData() { return &legendAreaData; }
    QImage getLegendTexture() { return txtRnd->getTexture(); }
    int verifyMousePosition(float x, float y);  //verifies if the position belong to the legend and in case to which signal

private:
    fontManager *fontMgr;
    QVector<float> indexes;  //stores the starting y-coordinate per each signal's string, used to identify where the user has clecked
    QVector<SigProperty> sig_properties;

    textRenderer *txtRnd;

    int char_Size;
    int screen_Width;
    int screen_Height;
    int line_space;
    QColor backGround;

    //indicate the display area also used for showing the below area
    QPointF topLeft;
    QPointF btmRight;

    QString font_filename;

    QVector<float> legendData;  //it will contains all the strings
    QVector<float> legendAreaData;  //contains the vertexes for plotting the legend background area

    void prepareBackgroundArea(void);
};

#endif // LEGENDWIDGET_H
