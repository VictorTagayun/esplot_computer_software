/**
  *********************************************************************************************************************************************************
  @file     :fontmanager.cpp
  @brief    :Functions for the Font Manager class
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

#include "fontmanager.h"

fontManager::fontManager(appPreferencesStruct *pref)
{
    preferences = pref;
    loaded = true;
}

int fontManager::getGlyph(QString font, QImage *glyph, QVector<Character> *ctr, int *mWidth, int *mHeight, int *nC)
{
    QImage img;  //empty pictures
    int idx;

    idx = findFont(font);
    if (idx == -1)
        idx = findFont(defaultFont);

    if (idx != -1)  //if it has been found, we load the glyph and output the data
    {
        glyphLoader *gl = new glyphLoader(fontNameFileList[idx].filename, preferences->font_resolution);
        *glyph = gl->getGlyphMap();
        *ctr = gl->getMetrics();
        gl->getCellWidthHeight(mWidth, mHeight);
        *nC =  gl->getNColumns();
        delete gl;
        return 0;
    }
    return -1;
}
/*
QVector<Character> fontManager::getMetrics(QString font)
{
    QVector<Character> metric;
    int idx;

    idx = findFont(font);
    if (idx != -1)
        idx = findFont(defaultFont);

    if (idx != -1)
        metric = fontData[idx]->getMetrics();

    return metric;
}

void fontManager::getCellWidthHeight(QString font, int *mWidth, int *mHeight)
{
    int idx;
    int width, height;

    width = 0; height = 0;

    idx = findFont(font);
    if (idx == -1)
        idx = findFont(defaultFont);

    if (idx != -1)
    {
        fontData[idx]->getCellWidthHeight(&width, &height);
    }

    *mWidth = width;
    *mHeight = height;
}

int fontManager::getColumns(QString font)
{
    int idx, N;
    N = 1;
    idx = findFont(font);
    if (idx == -1)
        idx = findFont(defaultFont);

    if (idx != -1)
        N = fontData[idx]->getNColumns();
    return N;
}
*/

QVector<QString> fontManager::getFontList()
{
    QVector<QString> fonts;
    int i;

    fonts.resize(fontNameFileList.count());
    for (i = 0; i < fonts.count(); i++)
        fonts[i] = fontNameFileList[i].name;
    return fonts;
}

void fontManager::setDefaultFont(QString fontname)
{
    defaultFont = fontname;
}

void fontManager::addFont(QString fontname, QString filename)
{
    fontNameFile f;

    f.name = fontname;
    f.filename = filename;
    fontNameFileList.append(f);
}

void fontManager::loadFonts()
{
    int i;

    //deletes all the previous objects
    for (i = 0; i < fontData.count(); i++)
        delete fontData[i];

    //loads all the fonts
    fontData.resize(fontNameFileList.count());
    for (i = 0; i < fontNameFileList.count(); i++)
    {
        fontData[i] = new glyphLoader(fontNameFileList[i].filename, preferences->font_resolution);
        librarySize += fontData[i]->getSizeBytes();
    }
}

QString fontManager::getFontName(QString filename, int *ok)
{
    FT_Library ft;
    FT_Face face;
    QString name;
    QByteArray ba;
    int i; char c;
    bool error;

    name = "Not found";

    if (FT_Init_FreeType(&ft))
    {
        qDebug() << "Error: Could not initialize FreeType Library";
        *ok = -1;
    }
    else
    {
        ba = filename.toLocal8Bit();
        const char* file_name = ba.data();

        if (FT_New_Face(ft, file_name, 0, &face))
        {
            qDebug() << "Error: Could not load font " << filename;
            *ok = -1;
        }
        else
        {
            error = false;
            //we check if we can load all the characters
            FT_Set_Char_Size(face, 0, 4 * 64, 300, 300);
            for (i = 0; i < 128-32; i++)  //we ignore the first 32 characters of the ASCII table since they are no useful characters. unknown characters will be replaced by a "."
            {
                c = static_cast<char>(i + 32);
                if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER))
                {
                    qDebug() << "Error on character " << (i+32);
                    error = true;
                }
            }
            if (error == true)
            {
                qDebug() << "Error loading the glyph " << filename;
                *ok = -1;
            }
            else
            {
                name = face->family_name;
                *ok = 0;
            }
        }
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return name;
}

int fontManager::findFont(QString font)
{
    int i;
    int idx;

    idx = -1;
    for (i = 0; i < fontNameFileList.count(); i++)
        if (QString::compare(font, fontNameFileList[i].name) == 0)
            idx = i;

    return idx;
}
