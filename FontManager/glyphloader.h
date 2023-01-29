/**
  *********************************************************************************************************************************************************
  @file     :glyphloader.h
  @brief    :Header for Glyphloader class
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

#ifndef GLYPHLOADER_H
#define GLYPHLOADER_H

#include <QFile>
#include <QDataStream>
#include <QImage>
#include <QString>
#include <QDebug>
#include <QColor>
#include <QVector>
#include <QDir>

#include "ft2build.h"
#include FT_FREETYPE_H

typedef struct _Character {
    qint32 TextureID;
    qint32 Width;
    qint32 Rows;
    qint32 Left;
    qint32 Top;
    qint32 AdvanceX;
} Character;

class glyphLoader
{
public:
    glyphLoader(QString filename, int resolution);
    ~glyphLoader();

    QImage getGlyphMap() { return *glyphMap; }
    QVector<Character> getMetrics() { return charMetrics; }
    void getCellWidthHeight(int* mWidth, int* mHeight) { *mWidth = CellWidth; *mHeight = CellHeight; }
    int getNColumns() { return maxColumns; }

    long long getSizeBytes() { return glyphSizeBytes; }

private:
    QFile *file;
    qint32 N_Chars;

    QVector<Character> charMetrics;  //contains all the characters metrics
    QVector<QVector<unsigned char>> glyphsData;  //contains all the glyphs
    QImage *glyphMap;  //it contains the loaded glyphMap in the correct texture format

    int CellWidth;
    int CellHeight;

    long long glyphSizeBytes;

    const int maxColumns = 16;

    void CreateQImage(void);
    void getMaxWidthHeight(int *width, int *height);

    void loadGlyph(QString filename, int pixelSize, int NChars);
};

#endif // GLYPHLOADER_H
