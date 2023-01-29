/**
  *********************************************************************************************************************************************************
  @file     :fontmanager.h
  @brief    :Header for FontManager class
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

#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <QImage>
#include <QString>
#include <QDebug>
#include <QColor>
#include <QVector>

#include "Managers/prefmanager.h"
#include "glyphloader.h"

#include "ft2build.h"
#include FT_FREETYPE_H

typedef struct _fontNameFile
{
    QString name;
    QString filename;
} fontNameFile;

//This classes loads all the fonts declared in the system
class fontManager
{
public:
    fontManager(appPreferencesStruct *pref);
    int getGlyph(QString font, QImage *glyph, QVector<Character> *ctr, int *mWidth, int *mHeight, int *nC);
//    QVector<Character> getMetrics(QString font);
//    void getCellWidthHeight(QString font, int* mWidth, int* mHeight);
//    int getColumns(QString font);
    bool isLoaded() { return loaded; }
    QVector<QString> getFontList();

    void setDefaultFont(QString fontname);
    void addFont(QString fontname, QString filename);

    void loadFonts();

    QString getFontName(QString filename, int *ok);

    int findFont(QString font);

private:
    QVector<fontNameFile> fontNameFileList;
    QVector<glyphLoader*> fontData;

    appPreferencesStruct *preferences;

    bool loaded;

    QString defaultFont;  //name of the default font

    long long librarySize;

    void createFontNameFileList();
};

#endif // FONTMANAGER_H
