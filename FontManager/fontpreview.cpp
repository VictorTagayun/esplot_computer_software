/**
  *********************************************************************************************************************************************************
  @file     :fontpreview.cpp
  @brief    :Functions for FontPreview class
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

#include "fontpreview.h"
#include <QDebug>

fontPreview::fontPreview(QWidget *parent, fontManager *font)
{
    (void) parent;

    fontMng = font;
    txtRnd = 0;

    backgroundCol = QColor("White");

    fontLoaded = false;

    this->setMinimumSize(150, 100);
}

fontPreview::~fontPreview()
{
    delete m_tex_vao;
    delete m_texVbo;
    delete m_program_tex;
}

void fontPreview::loadFont(QString fontname, QColor fontcolor, int fontsize)
{
    if (fontLoaded == true)
    {
        delete txtRnd;
    }
    txtRnd = new textRenderer(fontname, fontMng);
    fontColor = fontcolor;
    fontSize = fontsize;
    fontLoaded = true;
    update();
}

void fontPreview::initializeGL()
{
    makeCurrent();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    initializeOpenGLFunctions();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    backgroundCol.getRgbF(&backR, &backG, &backB, &backA);
    f->glClearColor(static_cast<float>(backR), static_cast<float>(backG), static_cast<float>(backB), static_cast<float>(backA));

    m_program_tex = new QOpenGLShaderProgram;
    if (m_program_tex->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/Shaders/Texture.vsh") == false)
        qDebug("Error compiling Texture Vertex Shader");
    if (m_program_tex->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/Shaders/Texture.fsh") == false)
        qDebug("Error compiling Texture Fragment Shader");
    if (m_program_tex->link() == false)
        qDebug("Texture shaders were not linked correctly\n");
    if (m_program_tex->bind() == false)
        qDebug("Texture shaders not corrently bound\n");

    m_tex_vao = new QOpenGLVertexArrayObject;
    if (m_tex_vao->create() == false)
        qDebug("Texture VAO error!\n");
    m_tex_vao->bind();

    m_texVbo = new QOpenGLBuffer;
    m_texVbo->create();
    m_texVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);

    doneCurrent();
}

void fontPreview::paintGL()
{
    backgroundCol.getRgbF(&backR, &backG, &backB, &backA);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(static_cast<float>(backR), static_cast<float>(backG), static_cast<float>(backB), static_cast<float>(backA));

    if (fontLoaded == true)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        txtRnd->prepareText(-0.90f, 0.00f, defaultString, fontColor, fontSize, this->width(), this->height(), true);
        text_data.clear();
        text_data.append(*txtRnd->getTextBuffer());

        m_texVbo->bind();
        m_texVbo->allocate(text_data.data(), text_data.length() * static_cast<int>(sizeof(float)));

        fontTex = new QOpenGLTexture(txtRnd->getTexture().mirrored());
        m_program_tex->bind();
        m_tex_vao->bind();
        fontTex->bind(0);
        m_texVbo->bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(4 * sizeof(GLfloat)));
        glDrawArrays(GL_TRIANGLES, 0, text_data.length() / 48 * 6);
        fontTex->release();
        m_texVbo->release();
        m_tex_vao->release();
        m_program_tex->release();

        fontTex->destroy();
        delete fontTex;
    }
}
