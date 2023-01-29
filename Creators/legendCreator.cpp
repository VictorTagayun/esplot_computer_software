/**
  *********************************************************************************************************************************************************
  @file     :legendCreator.cpp
  @brief    :Functions for legendCreator class
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

#include "legendCreator.h"

legendCreator::legendCreator(QString fontName, fontManager *font)
{
    fontMgr = font;
    char_Size = 12;  //12 pixels by default
    line_space = 10;  //10 pixels by default

    txtRnd = new textRenderer(fontName, fontMgr);

    legendAreaData.resize(28);
}

legendCreator::~legendCreator()
{
    delete txtRnd;
}

void legendCreator::setScreenSize(int width, int height)
{
    if (width > 0)
        screen_Width = width;
    if (height > 0)
        screen_Height = height;
}

void legendCreator::createLegend(float x, float y)
{
    int i, N;
    float penY;  //keeps track of the y location
    float width, height;
    float start_x, end_x, length;
    float minX;

    //x and y are referring to the topright position. we then aling to the right

    //first we delete all the data
    legendData.clear();
    indexes.clear();

    penY = y;
    N = sig_properties.count();

//    qDebug() << "Number of signals " << N;

    minX = x;
    for (i = 0; i < N; i++)
    {
        indexes.append(penY);
        end_x = txtRnd->prepareText(x, penY, sig_properties[i].name, sig_properties[i].color, char_Size, screen_Width, screen_Height, true);
        length = end_x - x;
        start_x = x - length;

        txtRnd->prepareText(start_x, penY, sig_properties[i].name, sig_properties[i].color, char_Size, screen_Width, screen_Height, true);
        if (start_x < minX)
            minX = start_x;

        legendData.append(*txtRnd->getTextBuffer());  //we add the new data

        //now we update the penY position
        penY -= static_cast<float>( (char_Size + line_space)) / static_cast<float>(screen_Height) * 2.0f;
    }

    penY = penY + (static_cast<float>(line_space) / static_cast<float>(screen_Height) * 2.0f);

    width = static_cast<float>(fabs(static_cast<double>(minX - x)));
    height = static_cast<float>(fabs(static_cast<double>(penY - y)));

    //Fixes the area
    topLeft.setX(static_cast<double>(minX - (width * 0.1f)));
    topLeft.setY(static_cast<double>(y + (height * 0.1f)));
    btmRight.setX(static_cast<double>(x + (width * 0.1f)));
    btmRight.setY(static_cast<double>(penY - (height * 0.1f)));

    prepareBackgroundArea();
}

int legendCreator::verifyMousePosition(float x, float y)
{
    int i, idx;
    float y0, y1, x0, x1;
    float startY;

    x0 = static_cast<float>(topLeft.x());
    y0 = static_cast<float>(topLeft.y());
    x1 = static_cast<float>(btmRight.x());
    y1 = static_cast<float>(btmRight.y());

    if ((y <= y0) && (y >= y1) && (x >= x0) && (x <= x1))  //mouse is in the box
    {
        idx = 0; startY = y0;
        for (i = 1; i < indexes.count(); i++)
        {
            if ((y < startY) && (y > indexes[i]))
                idx = i;
            startY = indexes[i];
        }
        if ((y < startY) && (y > y1))
            idx = i;
        return idx - 1;
    }
    else
        return -1;  //mouse not on the box
}

void legendCreator::prepareBackgroundArea()
{
    double r, g, b, a;

    backGround.getRgbF(&r, &g, &b, &a);

    a = 0.2;  //applies transparency

    legendAreaData[0] = static_cast<float>(topLeft.x());
    legendAreaData[1] = static_cast<float>(topLeft.y());
    legendAreaData[2] = 0.0f;
    legendAreaData[3] = static_cast<float>(r);
    legendAreaData[4] = static_cast<float>(g);
    legendAreaData[5] = static_cast<float>(b);
    legendAreaData[6] = static_cast<float>(a);

    legendAreaData[7] = static_cast<float>(topLeft.x());
    legendAreaData[8] = static_cast<float>(btmRight.y());
    legendAreaData[9] = 0.0f;
    legendAreaData[10] = static_cast<float>(r);
    legendAreaData[11] = static_cast<float>(g);
    legendAreaData[12] = static_cast<float>(b);
    legendAreaData[13] = static_cast<float>(a);

    legendAreaData[14] = static_cast<float>(btmRight.x());
    legendAreaData[15] = static_cast<float>(btmRight.y());
    legendAreaData[16] = 0.0f;
    legendAreaData[17] = static_cast<float>(r);
    legendAreaData[18] = static_cast<float>(g);
    legendAreaData[19] = static_cast<float>(b);
    legendAreaData[20] = static_cast<float>(a);

    legendAreaData[21] = static_cast<float>(btmRight.x());
    legendAreaData[22] = static_cast<float>(topLeft.y());
    legendAreaData[23] = 0.0f;
    legendAreaData[24] = static_cast<float>(r);
    legendAreaData[25] = static_cast<float>(g);
    legendAreaData[26] = static_cast<float>(b);
    legendAreaData[27] = static_cast<float>(a);
}
