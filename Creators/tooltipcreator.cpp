/**
  *********************************************************************************************************************************************************
  @file     :tooltipcreator.cpp
  @brief    :Functions for tooltipcreator class (tooltips in plots)
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

#include "tooltipcreator.h"

toolTipCreator::toolTipCreator(QString fontname, fontManager *font)
{
    fontMgr = font;

    char_Size = 12;  //12 pixels by default
    line_space = 10;  //10 pixels by default

    txtRnd = new textRenderer(fontname, fontMgr);
}

toolTipCreator::~toolTipCreator()
{
    delete txtRnd;
}

void toolTipCreator::setScreenSize(int width, int height)
{
    if (width > 0)
        screen_Width = width;
    if (height > 0)
        screen_Height = height;
}

void toolTipCreator::createToolTip(float x, float y, QColor color)
{
    float penY;  //keeps track of the y location
    QString text1, text2;
    QLocale locale;

    //first we delete all the data
    toolTipData.clear();

    //Adds a little offset not to have collision with the indicator
    x = x + 0.01f;
    y = y + 0.01f;

    penY = y;

    text1 = "Y: " + locale.toString(y_value, 'g', 3);
    text2 = "X: " + locale.toString(x_value, 'g', 3);

    txtRnd->prepareText(x, penY, text1, color, char_Size, screen_Width, screen_Height, true);
    toolTipData.append(*txtRnd->getTextBuffer());
    penY -= static_cast<float>( (char_Size + line_space)) / static_cast<float>(screen_Height) * 2.0f;
    txtRnd->prepareText(x, penY, text2, color, char_Size, screen_Width, screen_Height, true);
    toolTipData.append(*txtRnd->getTextBuffer());
}


