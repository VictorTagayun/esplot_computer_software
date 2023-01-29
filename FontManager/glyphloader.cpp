/**
  *********************************************************************************************************************************************************
  @file     :glyphloader.cpp
  @brief    :Functions for Glyphloader class
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

#include "glyphloader.h"
#include <QIODevice>

glyphLoader::glyphLoader(QString filename, int resolution)
{
    N_Chars = 0;

    loadGlyph(filename, resolution, 128);

    //We can now create the Image
    CreateQImage();

    glyphSizeBytes = glyphMap->sizeInBytes();
}


glyphLoader::~glyphLoader()
{
    delete glyphMap;
}

void glyphLoader::CreateQImage()
{
    int idx;
    int mW, mH, width, height;
    int i, j;
    int w, h;
    int offset_x, offset_y;
    int x, y;

    //First we determine the size of each cell as the maximum width and height among all characters
    getMaxWidthHeight(&mW, &mH);

    //Now we can create the image and size it accordingly in a fixed number of columns
    width = maxColumns * mW;
    height = (N_Chars / maxColumns) * mH;

    glyphMap = new QImage(width,height,QImage::Format_RGBA8888);

    //Now we load the glyphs in the image
    for (idx = 0; idx < N_Chars; idx++)
    {
        w = charMetrics[idx].Width;
        h = charMetrics[idx].Rows;

        offset_x = (idx % maxColumns) * mW;
        offset_y = (idx / maxColumns) * mH + (mH - h);

        for (i = 0; i < w; i++)
            for (j = 0; j < h; j++)
            {
                x = i + offset_x; y = (h - j - 1) + offset_y;
            //    if (((x >= 0) && (x < glyphMap->width())) && ((y >= 0) && (y < glyphMap->height())))
                    glyphMap->setPixelColor(x, y, QColor(0, 0, 0, glyphsData[idx].data()[(j*w)+i]));
            }
    }
}

void glyphLoader::getMaxWidthHeight(int *width, int *height)
{
    int i, mW, mH;

    mW = 0; mH = 0;

    for (i = 0; i < N_Chars; i++)
    {
        if (charMetrics[i].Width > mW)
            mW = charMetrics[i].Width;
        if (charMetrics[i].Rows > mH)
            mH = charMetrics[i].Rows;
    }

    *width = mW; *height = mH;
    CellWidth = mW;
    CellHeight = mH;
}

void glyphLoader::loadGlyph(QString filename, int pixelSize, int NChars)
{   
    int i, j, k, counter, pitch;

       QByteArray ba;
       FT_Library ft;
       char c;

       //first, we dump a copy of the font file in the temp directory as freetype cannot access resource files, but qfile can
       if (QFile::exists(QDir::tempPath() + "/temp.ttf"))
       {
           QFile::remove(QDir::tempPath() + "/temp.ttf");
       }
       QFile::copy(filename, QDir::tempPath() + "/temp.ttf");
       QFile f(QDir::tempPath() + "/temp.ttf");
       f.setPermissions(QFile::ReadUser|QFile::WriteUser);
       f.close();

       if (FT_Init_FreeType(&ft))
       {
           qDebug() << "Error: Could not initialize FreeType Library";
           return;
       }

       FT_Face face;
       QString temp = QDir::tempPath() + "/temp.ttf";
       ba = temp.toLocal8Bit();
       char* file_name = ba.data();

       if (FT_New_Face(ft, file_name, 0, &face))
       {
           qDebug() << "Error: Could not load font " << filename;
           //Before we exit, we delete the temp file
           QFile file(temp);
           file.remove();
           return;
       }

       //Now that the font has been loaded, we delete the temp file
       QFile file(temp);
       file.remove();

       FT_Set_Char_Size(face, 0, pixelSize * 64, 300, 300);
       if ((NChars <= 0) || (NChars > 255))
           return;

       N_Chars = NChars - 32;
       charMetrics.clear();
       glyphsData.clear();
       Character ctr;
       QVector<unsigned char> *array;

       for (i = 0; i < NChars; i++)  //we ignore the first 32 characters of the ASCII table since they are no useful characters. unknown characters will be replaced by a "."
       {
           c = static_cast<char>(i + 32);
           if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER))
           {
               qDebug() << "Error: Could not load the Glyph";
               return;
           }

           ctr.TextureID = i;
           ctr.Left = face->glyph->bitmap_left;
           ctr.Top = face->glyph->bitmap_top;
           ctr.Width = face->glyph->bitmap.width;
           ctr.Rows = face->glyph->bitmap.rows;
           ctr.AdvanceX = face->glyph->advance.x;
           charMetrics.append(ctr);

           pitch = face->glyph->bitmap.pitch;

           array = new QVector<unsigned char>;
           array->clear();
           array->resize(ctr.Width * ctr.Rows);

           if (pitch > 0)
           {
               counter = 0;
               for (j = ctr.Rows; j > 0; j--)
               {
                   for (k = 0; k < ctr.Width; k++)
                   {
                       array->data()[counter] = face->glyph->bitmap.buffer[(pitch * (j - 1)) + k];
                       counter++;
                   }
               }
           }
           else
           {
               counter = 0;
               for (j = 0; j < ctr.Rows; j++)
               {
                   for (k = 0; k < ctr.Width; k++)
                   {
                       array->data()[counter] = face->glyph->bitmap.buffer[(pitch * j) + k];
                       counter++;
                   }
               }
           }

           glyphsData.append(*array);

           delete array;
       }

       FT_Done_Face(face);
       FT_Done_FreeType(ft);
}
