/**
  *********************************************************************************************************************************************************
  @file     :statcreator.cpp
  @brief    :Functions for statcreator class
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

#include "statcreator.h"
#include <QLocale>

statCreator::statCreator(QString fontName, fontManager *font)
{
    fontMgr = font;

    char_Size = 12;  //12 pixels by default
    line_space = 10;  //10 pixels by default

    txtRnd = new textRenderer(fontName, fontMgr);
}

statCreator::~statCreator()
{
    delete txtRnd;
}

void statCreator::setScreenSize(int width, int height)
{
    if (width > 0)
        screen_Width = width;
    if (height > 0)
        screen_Height = height;
}

void statCreator::createStat(float x, float y)
{
    int i, N;
    float penY;  //keeps track of the y location
    QString text;
    float start_x, end_x, length;

    //first we delete all the data
    statData.clear();

    penY = y;
    N = sig_properties.count();

//    qDebug() << "Number of signals " << N;
    QLocale locale;

    //we align to the right
    for (i = N - 1; i >= 0; i--)
    {
        //We prepare the string first
        text = "Min: " + locale.toString(sig_properties[i].stats.min, 'g', 3);
        text += " Max: " + locale.toString(sig_properties[i].stats.max, 'g', 3);
        text += " Avg: " + locale.toString(sig_properties[i].stats.mean, 'g', 3);
        text += " Samples: " + locale.toString(static_cast<qlonglong>(sig_properties[i].stats.n_samples));

        end_x = txtRnd->prepareText(x, penY, text, sig_properties[i].color, char_Size, screen_Width, screen_Height, false);
        length = end_x - x;
        start_x = x - length;
        txtRnd->prepareText(start_x, penY, text, sig_properties[i].color, char_Size, screen_Width, screen_Height, false);

        statData.append(*txtRnd->getTextBuffer());  //we add the new data

        //now we update the penY position
        penY += static_cast<float>( (char_Size + line_space)) / static_cast<float>(screen_Height) * 2.0f;
    }
}
