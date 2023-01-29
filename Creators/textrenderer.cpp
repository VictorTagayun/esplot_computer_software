/**
  *********************************************************************************************************************************************************
  @file     :textrenderer.cpp
  @brief    :Functions for textrenderer class
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

#include "textrenderer.h"


textRenderer::textRenderer(QString fontName, fontManager *font)
{
    fontMgr = font;

    fontMgr->getGlyph(fontName, &glyph, &metrics, &CellWidth, &CellHeight, &nColumns);
}

float textRenderer::prepareText(float x, float y, QString text, QColor color, int pixel, int width, int height, bool topLeft)
{
    //The textBuffer is prepared in the following way
    // X Y ImgX ImgY R G B A

    //The arguments x and y represent the coordinates of the top-left corner if topLeft is true
    //The argument scale is needed to change dimension of the writing
    int i;
    float penX;  //gives the starting x point for writing. y is fixed
    double r, g, b, a;
    int glyphWidth, glyphHeight;

    //pixel indicates the height of the capital letter A in pixels
    float scaleX, scaleY;
    int char_A_width, char_A_height;
    float pixelX;  //while pixel gives the number of pixel in height referred to character H, here we determine given the aspect ratio of A the pixel in the X direction

    char_A_width = metrics[static_cast<unsigned char>(QChar('A').toLatin1()) - 32].Width;
    char_A_height = metrics[static_cast<unsigned char>(QChar('A').toLatin1()) - 32].Rows;

    pixelX = static_cast<float>(pixel) * static_cast<float>(char_A_width) / static_cast<float>(char_A_height);
    scaleY = static_cast<float>(pixel) / static_cast<float>(char_A_height) * 2.0f / static_cast<float>(height);
    scaleX = pixelX / static_cast<float>(char_A_width) * 2.0f / static_cast<float>(width);

    //we bring y now to indicate the bottomLeft corner by subtracting the height of the capital letter A
    if (topLeft == true)
        y -= static_cast<float>(char_A_height) * scaleY;

    //the rest is to be implemented
    penX = x;
    color.getRgbF(&r, &g, &b, &a);
    glyphWidth = glyph.width();
    glyphHeight = glyph.height();

    textBuffer.clear();

    for (i = 0; i < text.length(); i++)
    {
        //here we create the textBuffer
        //first we locate the character
        unsigned char c = static_cast<unsigned char>(text.at(i).toLatin1()) - 32;  //the first 32 characters are ignored in the glyph
        float x_c_pos, y_c_pos;
        int x_c_i, y_c_i;
        float y0, y1;
        float x0, x1;
        float char_width, char_height;

        x_c_i = (c % nColumns) * CellWidth;
        y_c_i = glyph.height() - (c / nColumns * CellHeight) - CellHeight;  //the second cellheight is because of the different axis direction of the y-axis
        x_c_pos = static_cast<float>(x_c_i) / static_cast<float>(glyphWidth);  //bring into 0 - 1 range
        y_c_pos = static_cast<float>(y_c_i) / static_cast<float>(glyphHeight); //bring into 0 - 1 range

        //We define now the x,y position of the rectangle
        x0 = penX + (static_cast<float>(metrics[c].Left) * scaleX);
        x1 = x0 + (static_cast<float>(metrics[c].Width * scaleX));
        y0 = y - (static_cast<float>(metrics[c].Rows - metrics[c].Top) * scaleY);
        y1 = y0 + (static_cast<float>(metrics[c].Rows) * scaleY);

        //Prepares the data to be inserted in textBuffer
        char_width = static_cast<float>(metrics[c].Width) / static_cast<float>(glyphWidth);
        char_height = static_cast<float>(metrics[c].Rows) / static_cast<float>(glyphHeight);

        textBuffer.append(x0); textBuffer.append(y0); textBuffer.append(x_c_pos); textBuffer.append(y_c_pos); textBuffer.append(static_cast<float>(r));
        textBuffer.append(static_cast<float>(g)); textBuffer.append(static_cast<float>(b)); textBuffer.append(static_cast<float>(a));

        textBuffer.append(x0); textBuffer.append(y1); textBuffer.append(x_c_pos); textBuffer.append(y_c_pos + char_height); textBuffer.append(static_cast<float>(r));
        textBuffer.append(static_cast<float>(g)); textBuffer.append(static_cast<float>(b)); textBuffer.append(static_cast<float>(a));

        textBuffer.append(x1); textBuffer.append(y1); textBuffer.append(x_c_pos + char_width); textBuffer.append(y_c_pos + char_height);
        textBuffer.append(static_cast<float>(r)); textBuffer.append(static_cast<float>(g)); textBuffer.append(static_cast<float>(b)); textBuffer.append(static_cast<float>(a));

        textBuffer.append(x1); textBuffer.append(y1); textBuffer.append(x_c_pos + char_width); textBuffer.append(y_c_pos + char_height);
        textBuffer.append(static_cast<float>(r)); textBuffer.append(static_cast<float>(g)); textBuffer.append(static_cast<float>(b)); textBuffer.append(static_cast<float>(a));

        textBuffer.append(x0); textBuffer.append(y0); textBuffer.append(x_c_pos); textBuffer.append(y_c_pos); textBuffer.append(static_cast<float>(r));
        textBuffer.append(static_cast<float>(g)); textBuffer.append(static_cast<float>(b)); textBuffer.append(static_cast<float>(a));

        textBuffer.append(x1); textBuffer.append(y0); textBuffer.append(x_c_pos + char_width); textBuffer.append(y_c_pos); textBuffer.append(static_cast<float>(r));
        textBuffer.append(static_cast<float>(g)); textBuffer.append(static_cast<float>(b)); textBuffer.append(static_cast<float>(a));

        penX += metrics[c].AdvanceX / 64 * scaleX;
    }

    return penX;  //return the position of the end of the line
}
