/**
  *********************************************************************************************************************************************************
  @file     :fontpreview.h
  @brief    :Header for FontPreview class
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

#ifndef FONTPREVIEW_H
#define FONTPREVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>

#include <QColor>
#include <QString>

#include "fontmanager.h"
#include "Creators/textrenderer.h"

class fontPreview : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    fontPreview(QWidget *parent = 0, fontManager *font = 0);
    ~fontPreview() override;

    void loadFont(QString fontname, QColor fontcolor, int fontsize);
    void setBackgroundColor(QColor color) { backgroundCol = color; }

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    fontManager *fontMng;
    textRenderer  *txtRnd;

    const QString defaultString = "AaBbCc-12345";

    QOpenGLShaderProgram *m_program_tex;
    QOpenGLVertexArrayObject *m_tex_vao;
    QOpenGLBuffer *m_texVbo;
    QOpenGLTexture *fontTex;

    bool fontLoaded;

    QColor backgroundCol;
    double backR, backG, backB, backA;

    QString fontName;
    QColor fontColor;
    int fontSize;

    QVector<GLfloat> text_data;
};

#endif // FONTPREVIEW_H
