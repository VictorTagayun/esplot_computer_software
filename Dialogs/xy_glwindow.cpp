/**
  *********************************************************************************************************************************************************
  @file     :xy_glwindow.cpp
  @brief    :Function for xy GLwindow class (window containing XY OpenGL plot)
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

#include "xy_glwindow.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QtDebug>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>

#include <math.h>

XY_GLWindow::XY_GLWindow(QWidget *parent, QString title, appPreferencesStruct *pref, fontManager *font) : QOpenGLWidget(parent)
{
    this->setMinimumSize(pref->plot_width_size, pref->plot_height_size);

    sig_properties.clear();

    preferences = pref;
    fontMgr = font;

    plotTitle = title;

    N_signals = 0;
    Number_of_Points = 0;

    pastValueGain = 1.0f;

    this->setCursor(Qt::BusyCursor);

    setMouseTracking(false);  //mousemove event happens only when one mouse button is clicked
    zoom_mode = 0;
    zoom_enabled = false;
    pan_enabled = false;
    toolTipEnabled = false;

    grid = new Grid_XY(preferences, plotTitle, this->width(), this->height(), fontMgr);

    render_zoom_area = false;
    zoom_buffer.resize(28);  //7 data per vertex, 4 vertexes
    topRight.setX(1.0); topRight.setY(1.0);
    btmLeft.setX(-1.0); btmLeft.setY(-1.0);

    //Create the cursors
    QSize cursSize(24, 24);
    zoomXYCursor = new QCursor(QPixmap(":/Icons/Icons/zoomXY.ico").scaled(cursSize, Qt::AspectRatioMode::KeepAspectRatio,Qt::TransformationMode::SmoothTransformation));
    zoomXCursor = new QCursor(QPixmap(":/Icons/Icons/zoomX.ico").scaled(cursSize, Qt::AspectRatioMode::KeepAspectRatio,Qt::TransformationMode::SmoothTransformation));
    zoomYCursor = new QCursor(QPixmap(":/Icons/Icons/zoomY.ico").scaled(cursSize, Qt::AspectRatioMode::KeepAspectRatio,Qt::TransformationMode::SmoothTransformation));
    panCursor = new QCursor(QPixmap(":/Icons/Icons/panXY.ico").scaled(cursSize, Qt::AspectRatioMode::KeepAspectRatio,Qt::TransformationMode::SmoothTransformation));

    legend = new legendCreator(pref->legend_font, fontMgr);
    legendEnabled = true;
    panLegend = false;
    x_leg = 0.95f;  //topLeft default position
    y_leg = 0.95f;

    tooltip = new toolTipCreator(preferences->toolTip_font, fontMgr);

    this->setCursor(Qt::ArrowCursor);
}

XY_GLWindow::~XY_GLWindow()
{
    cleanup();
}

void XY_GLWindow::setGridMinY(double minY)
{
    grid->set_min_y(static_cast<float>(minY));
}

void XY_GLWindow::setGridMaxY(double maxY)
{
    grid->set_max_y(static_cast<float>(maxY));
}

void XY_GLWindow::setGridMinX(double minX)
{
    grid->set_min_x(static_cast<float>(minX));
}

void XY_GLWindow::setGridMaxX(double maxX)
{
    grid->set_max_x(static_cast<float>(maxX));
}

void XY_GLWindow::setGridTimeBase(double TimeBase)
{
    grid->set_base_time(static_cast<float>(TimeBase));
}

void XY_GLWindow::initializeGL()
{
    makeCurrent();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    initializeOpenGLFunctions();
    backgroundCol = preferences->backGround_color;
    backgroundCol.getRgbF(&backR, &backG, &backB, &backA);
    f->glClearColor(static_cast<float>(backR), static_cast<float>(backG), static_cast<float>(backB), static_cast<float>(backA));

    transfMatrix.setToIdentity();  //identity matrix
    identityMatrix.setToIdentity();

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &XY_GLWindow::cleanup);

    //Line Program Declaration
    m_program = new QOpenGLShaderProgram;
    if (m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/Shaders/Line.vsh") == false)
        qDebug("Error compiling Vertex Shader");
    if (m_program->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/Shaders/Line.gsh") == false)
        qDebug("Error compiling Geometry Shader");
    if (m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/Shaders/Line.fsh") == false)
        qDebug("Error compiling Fragment Shader");
    if (m_program->link() == false)
        qDebug("Shaders were not linked correctly\n");
    if (m_program->bind() == false)
        qDebug("Shader not correctly bound\n");

    m_in_colorLoc = m_program->uniformLocation("in_color");
    m_in_paramsLoc = m_program->uniformLocation("in_params");
    m_transfMatrixLoc = m_program->uniformLocation("transfMatrix");
    m_screenSizeLoc = m_program->uniformLocation("screen_size");

    m_vao = new QOpenGLVertexArrayObject;
    if (m_vao->create() == false)
        qDebug("VAO error!\n");
    m_vao->bind();

    m_gridVbo = new QOpenGLBuffer;
    if (m_gridVbo->create() == false)
        qDebug("VBO error!\n");
    if (m_gridVbo->bind() == false)
        qDebug("VBO not bounded!!\n");
    m_gridVbo->allocate(grid->Data(), grid->count() * static_cast<int>(sizeof(GLfloat)));
    m_gridVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);

    m_tTcrossVbo = new QOpenGLBuffer;
    m_tTcrossVbo->create();

    m_triggerVbo = new QOpenGLBuffer;
    m_triggerVbo->create();

    m_vao->release();

    m_program->release();

    //XY-Line Program Declaration
    m_xy_program = new QOpenGLShaderProgram;
    if (m_xy_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/Shaders/XYLine.vsh") == false)
        qDebug("Error compiling Vertex Shader");
    if (m_xy_program->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/shaders/Shaders/Line.gsh") == false)
        qDebug("Error compiling Geometry Shader");
    if (m_xy_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/Shaders/Line.fsh") == false)
        qDebug("Error compiling Fragment Shader");
    if (m_xy_program->link() == false)
        qDebug("Shaders were not linked correctly\n");
    if (m_xy_program->bind() == false)
        qDebug("Shader not correctly bound\n");

    m_xy_conv_factLoc = m_xy_program->uniformLocation("conv_fact");
    m_xy_in_colorLoc = m_xy_program->uniformLocation("in_color");
    m_xy_in_paramsLoc = m_xy_program->uniformLocation("in_params");
    m_xy_transfMatrixLoc = m_xy_program->uniformLocation("transfMatrix");
    m_xy_screenSizeLoc = m_xy_program->uniformLocation("screen_size");
    m_xy_pastValueGainLoc = m_xy_program->uniformLocation("pastValueGain");
    m_xy_n_samplesLoc = m_xy_program->uniformLocation("n_samples");
    m_xy_instanceLoc = m_xy_program->uniformLocation("instance");

    m_xy_vao = new QOpenGLVertexArrayObject;
    if (m_xy_vao->create() == false)
        qDebug("VAO error!\n");
    m_xy_vao->bind();

    m_x_signalVbo = new QOpenGLBuffer;
    if (m_x_signalVbo->create() == false)
        qDebug("VBO error!\n");
    if (m_x_signalVbo->bind() == false)
        qDebug("VBO not bounded!!\n");

    m_y_signalVbo = new QOpenGLBuffer;
    if (m_y_signalVbo->create() == false)
        qDebug("VBO error!\n");
    if (m_y_signalVbo->bind() == false)
        qDebug("VBO not bounded!!\n");

    m_y_signalVbo->release();
    m_x_signalVbo->release();
    m_xy_vao->release();
    m_xy_program->release();

    m_program_dot = new QOpenGLShaderProgram;
    if (m_program_dot->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/Shaders/XYDot.vsh") == false)
        qDebug("Error compiling Vertex Shader");
    if (m_program_dot->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/Shaders/Dot.fsh") == false)
        qDebug("Error compiling Fragment Shader");
    if (m_program_dot->link() == false)
        qDebug("Shaders were not linked correctly\n");
    if (m_program_dot->bind() == false)
        qDebug("Shader not correctly bound\n");
    m_in_convFactDotLoc = m_program_dot->uniformLocation("conv_fact");
    m_in_colorLocDot = m_program_dot->uniformLocation("in_color");
    m_in_paramsLocDot = m_program_dot->uniformLocation("in_params");
    m_transfMatrixLocDot = m_program_dot->uniformLocation("transfMatrix");
    m_xy_pastValueGainDotLoc = m_program_dot->uniformLocation("pastValueGain");
    m_xy_n_samplesDotLoc = m_program_dot->uniformLocation("n_samples");
    m_xy_instanceDotLoc = m_program_dot->uniformLocation("instance");

    m_program_dot->release();

    m_program_zoom = new QOpenGLShaderProgram;
    if (m_program_zoom->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/Shaders/Zoom.vsh") == false)
        qDebug("Error compiling Zoom Vertex Shader");
    if (m_program_zoom->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/Shaders/Zoom.fsh") == false)
        qDebug("Error compiling Zoom Geometry Shader");
    if (m_program_zoom->link() == false)
        qDebug("Zoom shaders were not linked correctly\n");
    if (m_program_zoom->bind() == false)
        qDebug("Zoom shader not correctly bound\n");

    m_zoom_vao = new QOpenGLVertexArrayObject;
    if (m_zoom_vao->create() == false)
        qDebug("Zoom VAO error!\n");
    m_zoom_vao->bind();

    m_zoomVbo = new QOpenGLBuffer;
    if (m_zoomVbo->create() == false)
        qDebug("Zoom VBO error!\n");

    m_zoom_vao->release();
    m_program_zoom->release();

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
    if (m_texVbo->create() == false)
        qDebug("Texture VBO error!\n");

    m_tex_vao->release();
    m_program_tex->release();

    gridTex = new QOpenGLTexture(grid->getFontTexture().mirrored());
    legendTex = new QOpenGLTexture(legend->getLegendTexture().mirrored());  //creates the legend texture
    toolTipTex = new QOpenGLTexture(tooltip->getToolTipTexture().mirrored());
    doneCurrent();
}

void XY_GLWindow::paintGL()
{
    int i;

    if ((N_signals != sig_properties.count()) && (N_signals != indexes.count()))  //we check that the signals vector are coherent
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    zoom_area_color = preferences->zoom_color;
    legend->setBackGroundColor(preferences->legend_background_color);
    tooltip->setCharSize(preferences->toolTip_char_size);
    tooltip->setLineSpace(preferences->toolTip_line_space);
    toolTipColor = preferences->toolTip_color;
    backgroundCol = preferences->backGround_color;
    backgroundCol.getRgbF(&backR, &backG, &backB, &backA);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(static_cast<float>(backR), static_cast<float>(backG), static_cast<float>(backB), static_cast<float>(backA));

    //DRAW THE GRID
    //First Axis
    m_gridVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
    m_gridVbo = new QOpenGLBuffer;
    if (m_gridVbo->create() == false)
        qDebug("VBO grid error!\n");
    if (m_gridVbo->bind() == false)
        qDebug("VBO grid not bounded!!\n");
    m_gridVbo->allocate(grid->Data(), grid->count() * static_cast<int>(sizeof(GLfloat)));
    m_gridVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
    m_program->bind();
    m_vao->bind();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_gridVbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), reinterpret_cast<void*>(0));
    m_program->setUniformValue(m_transfMatrixLoc, transfMatrix);
    m_program->setUniformValue(m_screenSizeLoc, QVector2D(static_cast<float>(this->width()), static_cast<float>(this->height())));
    m_program->setUniformValue(m_in_colorLoc, preferences->axis_color);
    m_program->setUniformValue(m_in_paramsLoc, QVector3D(preferences->axis_linewidth, 1.0f, 0.0f));
    glDrawArrays(GL_LINES, 0, 4);
    if (grid->getDrawGrid() == true)
    {
        m_program->setUniformValue(m_in_colorLoc, preferences->grid_color);
        m_program->setUniformValue(m_in_paramsLoc, QVector3D(preferences->grid_linewidth, 1.0f, 0.0f));
        glDrawArrays(GL_LINES, 4, grid->vertexCount() - 4);
    }
    m_vao->release();
    m_program->release();

    //DRAW SIGNALS
    m_xy_program->bind();
    m_xy_vao->bind();
    m_x_signalVbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), reinterpret_cast<void*>(0));
    m_y_signalVbo->bind();
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), reinterpret_cast<void*>(0));
    m_xy_program->setUniformValue(m_xy_transfMatrixLoc, transfMatrix);
    m_xy_program->setUniformValue(m_xy_screenSizeLoc, QVector2D(static_cast<float>(this->width()), static_cast<float>(this->height())));
    for (i = 0; i < N_signals; i++)
    {
        if (i < sig_properties.count())  //Makes sure that there is an element in sig_properties
            if (sig_properties[i].lineRendering == true)
            {
                m_xy_program->setUniformValue(m_xy_conv_factLoc, QVector2D(grid->get_ConvFact_X(), grid->get_ConvFact_Y()));
                m_xy_program->setUniformValue(m_xy_in_colorLoc, sig_properties[i].color);
                m_xy_program->setUniformValue(m_xy_in_paramsLoc, QVector3D(sig_properties[i].line_width, grid->get_Y_Axis(), grid->get_X_Axis()));
                m_xy_program->setUniformValue(m_xy_instanceLoc, i);
                m_xy_program->setUniformValue(m_xy_n_samplesLoc, Number_of_Points);
                m_xy_program->setUniformValue(m_xy_pastValueGainLoc, pastValueGain);
                glDrawArrays(GL_LINE_STRIP, indexes[i], Number_of_Points);
            }
    }

    m_x_signalVbo->release();
    m_y_signalVbo->release();
    m_xy_vao->release();
    m_xy_program->release();

    glEnable(GL_PROGRAM_POINT_SIZE);
    m_program_dot->bind();
    m_vao->bind();
    m_x_signalVbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), reinterpret_cast<void*>(0));
    m_y_signalVbo->bind();
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), reinterpret_cast<void*>(0));
    m_program_dot->setUniformValue(m_transfMatrixLocDot, transfMatrix);
    for (i = 0; i < N_signals; i++)
    {
        if (i < sig_properties.count())
            if (sig_properties[i].dotRendering == true)
            {
                m_program_dot->setUniformValue(m_in_convFactDotLoc, QVector2D(grid->get_ConvFact_X(), grid->get_ConvFact_Y()));
                m_program_dot->setUniformValue(m_in_colorLocDot, sig_properties[i].color);
                m_program_dot->setUniformValue(m_in_paramsLocDot, QVector3D(sig_properties[i].line_width, grid->get_Y_Axis(), grid->get_X_Axis()));
                m_program_dot->setUniformValue(m_xy_instanceDotLoc, i);
                m_program_dot->setUniformValue(m_xy_n_samplesDotLoc, Number_of_Points);
                m_program_dot->setUniformValue(m_xy_pastValueGainDotLoc, pastValueGain);
                glDrawArrays(GL_POINTS, indexes[i], Number_of_Points);
            }
    }
    m_x_signalVbo->release();
    m_y_signalVbo->release();
    m_vao->release();
    m_program->release();

    //DRAW GRID TEXT
    grid->update_Text(this->width(), this->height());
    m_texVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
    m_program_tex->bind();
    m_tex_vao->bind();
    m_texVbo = new QOpenGLBuffer;
    if (m_texVbo->create() == false)
        qDebug("Texture VBO signal error!\n");
    if (m_texVbo->bind() == false)
        qDebug("Texture VBO signal not bounded!!\n");
    m_texVbo->allocate(grid->getTextData()->data(), grid->getTextData()->length() * static_cast<int>(sizeof(float)));
    m_texVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
    m_texVbo->release();
    m_tex_vao->release();
    m_program_tex->release();
    m_program_tex->bind();
    m_tex_vao->bind();
    m_texVbo->bind();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(0));
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(2 * sizeof(GLfloat)));
    f->glEnableVertexAttribArray(2);
    f->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(4 * sizeof(GLfloat)));
    gridTex->bind();
    glDrawArrays(GL_TRIANGLES, 0, grid->getTextData()->length() / 48 * 6);
    m_texVbo->release();
    m_tex_vao->release();
    m_program_tex->release();

    if (render_zoom_area == true)
    {
        prepare_zoom_area();
        m_program_zoom->bind();
        m_zoom_vao->bind();
        m_zoomVbo->bind();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        m_zoomVbo->release();
        m_zoom_vao->release();
        m_program_zoom->release();
    }

    if ((legendEnabled == true) && (sig_properties.count() > 0))  //this will check if legend visible is ON
    {
        //here we prepare the legend data
        legend->setCharSize(preferences->legend_char_size);
        legend->setLineSpace(preferences->legend_line_space);
        legend->setScreenSize(this->width(), this->height());
        legend->setSignals(convertSigProperties());
        legend->createLegend(x_leg, y_leg);  //set by default at top left

        //First we paint the background area
        m_zoomVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
        delete m_zoomVbo;
        m_program_zoom->bind();
        m_zoom_vao->bind();
        m_zoomVbo = new QOpenGLBuffer;
        if (m_zoomVbo->create() == false)
            qDebug("Zoom VBO signal error!\n");
        if (m_zoomVbo->bind() == false)
            qDebug("Zoom VBO signal not bounded!!\n");
        m_zoomVbo->allocate(legend->getLegendAreaData()->data(), 28 * static_cast<int>(sizeof(GLfloat)));
        m_zoomVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
        m_program_zoom->bind();
        m_zoom_vao->bind();
        m_zoomVbo->bind();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        m_zoomVbo->release();
        m_zoom_vao->release();
        m_program_zoom->release();

        //Now we paint the text
        m_texVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
        m_program_tex->bind();
        m_tex_vao->bind();
        m_texVbo = new QOpenGLBuffer;
        if (m_texVbo->create() == false)
            qDebug("Texture VBO signal error!\n");
        if (m_texVbo->bind() == false)
            qDebug("Texture VBO signal not bounded!!\n");
        m_texVbo->allocate(legend->getLegendData()->data(), legend->getLegendData()->length() * static_cast<int>(sizeof(float)));
        m_texVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
        m_texVbo->release();
        m_tex_vao->release();
        m_program_tex->release();

        m_program_tex->bind();
        m_tex_vao->bind();
        m_texVbo->bind();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(2 * sizeof(GLfloat)));
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(4 * sizeof(GLfloat)));
        legendTex->bind();
        glDrawArrays(GL_TRIANGLES, 0, legend->getLegendData()->length() / 48 * 6);
        m_texVbo->release();
        m_tex_vao->release();
        m_program_tex->release();
    }

    if (toolTipEnabled == true)
    {
        float coordX, coordY;
        //here we prepare the tooltip data
        tooltip->setScreenSize(this->width(), this->height());
        calculateCoordinateFromCursor(x_tTint, y_tTint, &coordX, &coordY);
        tooltip->setValues(coordX, coordY);
        tooltip->createToolTip(x_tooltip, y_tooltip, toolTipColor);  //set by default at top left

        m_texVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
        m_program_tex->bind();
        m_tex_vao->bind();
        m_texVbo = new QOpenGLBuffer;
        if (m_texVbo->create() == false)
            qDebug("Texture VBO signal error!\n");
        if (m_texVbo->bind() == false)
            qDebug("Texture VBO signal not bounded!!\n");
        m_texVbo->allocate(tooltip->getToolTipData()->data(), tooltip->getToolTipData()->length() * static_cast<int>(sizeof(float)));
        m_texVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
        m_texVbo->release();
        m_tex_vao->release();
        m_program_tex->release();

        m_program_tex->bind();
        m_tex_vao->bind();
        m_texVbo->bind();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(2 * sizeof(GLfloat)));
        f->glEnableVertexAttribArray(2);
        f->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(4 * sizeof(GLfloat)));
        toolTipTex->bind();
        glDrawArrays(GL_TRIANGLES, 0, tooltip->getToolTipData()->length() / 48 * 6);
        m_texVbo->release();
        m_tex_vao->release();
        m_program_tex->release();

        //Draws the indicator cross
        prepareToolTipCrossData();  //prepares the indicator cross buffer
        m_program->bind();
        m_vao->bind();
        m_tTcrossVbo->bind();
        f->glEnableVertexAttribArray(0);
        f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        m_program->setUniformValue(m_transfMatrixLoc, identityMatrix);
        m_program->setUniformValue(m_screenSizeLoc, QVector2D(static_cast<float>(this->width()), static_cast<float>(this->height())));
        m_program->setUniformValue(m_in_colorLoc, toolTipColor);
        m_program->setUniformValue(m_in_paramsLoc, QVector4D(5.0f, 1.0f, 0.0f, 0.0f));
        glDrawArrays(GL_LINES, 0, 4);  //4 vertexes
        m_tTcrossVbo->release();
        m_vao->release();
        m_program->release();
    }
}

void XY_GLWindow::resizeGL(int w, int h)
{
    (void) w;
    (void) h;

    if (toolTipEnabled == true)
        toolTipEnabled = false;
}

void XY_GLWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    float width, height;
    float screen_x, screen_y;
    int idx;
    int x, y;

    x = event->x(); y = event->y();

    width = static_cast<float>(this->size().width());
    height = static_cast<float>(this->size().height());

    //we verify first if we clicked on the legend
    screen_x = (static_cast<float>(x) - (width / 2.0f)) / (width / 2.0f);
    screen_y = ((static_cast<float>(y) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));
    idx = legend->verifyMousePosition(screen_x, screen_y);

    if (idx == -1)
    {
        if (toolTipEnabled == true)
        {
            toolTipEnabled = false;
        }
        else
        {
            toolTipEnabled = true;
            width = static_cast<float>(this->width());
            height = static_cast<float>(this->height());
            x_tTint = event->pos().x();
            y_tTint = event->pos().y();
            x_tooltip = (static_cast<float>(x_tTint) - (width / 2.0f)) / (width / 2.0f);
            y_tooltip = ((static_cast<float>(y_tTint) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));
        }
        update();
    }
}

void XY_GLWindow::cleanup()
{
    if (m_program == nullptr)
        return;
    makeCurrent();
    m_gridVbo->destroy();
    m_x_signalVbo->destroy();
    m_y_signalVbo->destroy();
    m_zoomVbo->destroy();
    m_vao->destroy();
    m_zoom_vao->destroy();
    legendTex->destroy();
    doneCurrent();
}

void XY_GLWindow::prepare_Signal_Buffer(int n_signals, int n_points, QVector<GLfloat> *x_buffer, QVector<GLfloat> *y_buffer, QVector<XY_SigProperty> prop, QVector<int> idx, int NSamples)
{
    (void) NSamples;

    if (n_signals < prop.count())
        return;  //in this case we would have an error

    int i;
    sig_properties.clear();
    sig_properties.resize(prop.count());
    for (i = 0; i < prop.count(); i++)
    {
        sig_properties[i].name = prop[i].name;
        sig_properties[i].color = prop[i].color;
        sig_properties[i].line_width = prop[i].line_width;
        sig_properties[i].dotRendering = prop[i].dotRendering;
        sig_properties[i].lineRendering = prop[i].lineRendering;
    }

    makeCurrent();

    //the buffer is now ready to plot
    //we can prepare the x vertex buffer object
    m_x_signalVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
    m_x_signalVbo = new QOpenGLBuffer;
    if (m_x_signalVbo->create() == false)
        qDebug("VBO signal error!\n");
    if (m_x_signalVbo->bind() == false)
        qDebug("VBO signal not bounded!!\n");
    m_x_signalVbo->allocate(x_buffer->data(), n_signals * n_points * static_cast<int>(sizeof(GLfloat)));
    m_x_signalVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
    m_x_signalVbo->release();

    //we can prepare the y vertex buffer object
    m_y_signalVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
    m_y_signalVbo = new QOpenGLBuffer;
    if (m_y_signalVbo->create() == false)
        qDebug("VBO signal error!\n");
    if (m_y_signalVbo->bind() == false)
        qDebug("VBO signal not bounded!!\n");
    m_y_signalVbo->allocate(y_buffer->data(), n_signals * n_points * static_cast<int>(sizeof(GLfloat)));
    m_y_signalVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
    m_y_signalVbo->release();

    N_signals = n_signals;
    Number_of_Points = n_points;
    indexes = idx;

    doneCurrent();
}

void XY_GLWindow::enable_zoom(bool enable) {
    zoom_enabled = enable;
    pan_enabled = false;
    this->setCursor(Qt::ArrowCursor);
}

void XY_GLWindow::set_autoscale_mode(bool enable) {
    if (enable == true)
    {
        transfMatrix.setToIdentity();
        grid->setTransformationMatrix(transfMatrix);
        topRight.setX(1.0); topRight.setY(1.0);
        btmLeft.setX(-1.0); btmLeft.setY(-1.0);
    }
}

QImage XY_GLWindow::getPlotFigure()
{
    return this->grabFramebuffer();
}

void XY_GLWindow::updateFonts()
{
    makeCurrent();

    delete gridTex;
    delete legendTex;
    delete toolTipTex;

    delete legend;
    delete tooltip;

    legend = new legendCreator(preferences->legend_font, fontMgr);
    tooltip = new toolTipCreator(preferences->toolTip_font, fontMgr);
    grid->updateFont();

    gridTex = new QOpenGLTexture(grid->getFontTexture().mirrored());
    legendTex = new QOpenGLTexture(legend->getLegendTexture().mirrored());  //creates the legend texture
    toolTipTex = new QOpenGLTexture(tooltip->getToolTipTexture().mirrored());

    doneCurrent();
}

void XY_GLWindow::calculate_visible_area()
{
    visible_width = static_cast<float>(fabs(topRight.x() - btmLeft.x()));
    visible_height = static_cast<float>(fabs(topRight.y() - btmLeft.y()));
}

void XY_GLWindow::prepare_zoom_area()
{
    //prepares the 4 vertixes to be used for rendering the zoom area
    qreal r, g, b, a;
    zoom_area_color.getRgbF(&r, &g, &b, &a);
    float width, height;
    float ax, ay, bx, by;

    a = 0.25;  //transparence

    //transform screen coordinates to opengl coordinates
    width = static_cast<float>(this->size().width());
    height = static_cast<float>(this->size().height());

    ax = (static_cast<float>(x1x) - (width / 2.0f)) / (width / 2.0f);
    ay = ((static_cast<float>(x1y) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));
    bx = (static_cast<float>(x2x) - (width / 2.0f)) / (width / 2.0f);
    by = ((static_cast<float>(x2y) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));

    //first vertex
    zoom_buffer[0] = ax; zoom_buffer[1] = ay; zoom_buffer[2] = 0.0f;
    zoom_buffer[3] = static_cast<float>(r); zoom_buffer[4] = static_cast<float>(g);
    zoom_buffer[5] = static_cast<float>(b); zoom_buffer[6] = static_cast<float>(a);

    //second vertex
    zoom_buffer[7] = bx; zoom_buffer[8] = ay; zoom_buffer[9] = 0.0f;
    zoom_buffer[10] = static_cast<float>(r); zoom_buffer[11] = static_cast<float>(g);
    zoom_buffer[12] = static_cast<float>(b); zoom_buffer[13] = static_cast<float>(a);

    //third vertex
    zoom_buffer[14] = bx; zoom_buffer[15] = by; zoom_buffer[16] = 0.0f;
    zoom_buffer[17] = static_cast<float>(r); zoom_buffer[18] = static_cast<float>(g);
    zoom_buffer[19] = static_cast<float>(b); zoom_buffer[20] = static_cast<float>(a);

    //fourth vertex
    zoom_buffer[21] = ax; zoom_buffer[22] = by; zoom_buffer[23] = 0.0f;
    zoom_buffer[24] = static_cast<float>(r); zoom_buffer[25] = static_cast<float>(g);
    zoom_buffer[26] = static_cast<float>(b); zoom_buffer[27] = static_cast<float>(a);

    //the buffer is now ready to plot
    //we can prepare the vertex buffer object
    m_zoomVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
    delete m_zoomVbo;
    m_program_zoom->bind();
    m_zoom_vao->bind();
    m_zoomVbo = new QOpenGLBuffer;
    if (m_zoomVbo->create() == false)
        qDebug("Zoom VBO signal error!\n");
    if (m_zoomVbo->bind() == false)
        qDebug("Zoom VBO signal not bounded!!\n");
    m_zoomVbo->allocate(zoom_buffer.data(), 28 * static_cast<int>(sizeof(GLfloat)));
    m_zoomVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
    m_zoomVbo->release();
    m_zoom_vao->release();
    m_program_zoom->release();
}

void XY_GLWindow::prepare_grid_buffer()
{
    makeCurrent();

    //the buffer is now ready to plot
    //we can prepare the vertex buffer object
    m_gridVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
    m_gridVbo = new QOpenGLBuffer;
    if (m_gridVbo->create() == false)
        qDebug("VBO grid error!\n");
    if (m_gridVbo->bind() == false)
        qDebug("VBO grid not bounded!!\n");
    m_gridVbo->allocate(grid->Data(), grid->count() * static_cast<int>(sizeof(GLfloat)));
    m_gridVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::StaticDraw);
    doneCurrent();
}

void XY_GLWindow::legendSignalToggle(int idx)
{
    int condition;

    if ((idx < sig_properties.count()) && (idx >= 0))
    {
        condition = 0;
        if ((sig_properties[idx].lineRendering == false) && (sig_properties[idx].dotRendering == false))
            condition = 0;
        if ((sig_properties[idx].lineRendering == false) && (sig_properties[idx].dotRendering == true))
            condition = 1;
        if ((sig_properties[idx].lineRendering == true) && (sig_properties[idx].dotRendering == false))
            condition = 2;
        if ((sig_properties[idx].lineRendering == true) && (sig_properties[idx].dotRendering == true))
            condition = 3;

        //increases now the condition
        condition++;
        if (condition > 3)
            condition = 0;

        if (condition == 0)
        {
            sig_properties[idx].lineRendering = false; sig_properties[idx].dotRendering = false;
        }
        if (condition == 1)
        {
            sig_properties[idx].lineRendering = false; sig_properties[idx].dotRendering = true;
        }
        if (condition == 2)
        {
            sig_properties[idx].lineRendering = true; sig_properties[idx].dotRendering = false;
        }
        if (condition == 3)
        {
            sig_properties[idx].lineRendering = true; sig_properties[idx].dotRendering = true;
        }

        emit updateSigProperties(sig_properties);
    }
}

void XY_GLWindow::calculateCoordinateFromCursor(int screen_x, int screen_y, float *coordX, float *coordY)
{
    float width, height;
    float x, y;

    width = static_cast<float>(this->size().width());
    height = static_cast<float>(this->size().height());

    //now we transform the coordinates
    calculate_visible_area();
    x = ((screen_x / width) * visible_width) + static_cast<float>(btmLeft.x());
    y = (((height - screen_y) / height) * visible_height) + static_cast<float>(btmLeft.y());

    *coordX = (x - grid->get_Y_Axis()) / grid->get_ConvFact_X();
    *coordY = (y - grid->get_X_Axis()) / grid->get_ConvFact_Y();
}

void XY_GLWindow::prepareToolTipCrossData()
{
    //prepares the 4 vertixes to be used for rendering the zoom area
    float lengthw;

    float width, height;
    float lenghth;

    width = static_cast<float>(this->width());
    height = static_cast<float>(this->height());
    lengthw = 30.0f * 2.0f / width;
    lenghth = lengthw * width / height;

    toolTipCrossData.clear();
    toolTipCrossData.resize(8);  //4 vertexes with 10 elements each

    //X  Y X_AXIS  R  G  B ALFA LW  CF  ORD
    //first vertex
    toolTipCrossData[0] = x_tooltip - (lengthw / 2.0f); toolTipCrossData[1] = y_tooltip;

    toolTipCrossData[2] = x_tooltip + (lengthw / 2.0f); toolTipCrossData[3] = y_tooltip;

    toolTipCrossData[4] = x_tooltip; toolTipCrossData[5] = y_tooltip + (lenghth / 2.0f);

    toolTipCrossData[6] = x_tooltip; toolTipCrossData[7] = y_tooltip - (lenghth / 2.0f);


    //the buffer is now ready to plot
    //we can prepare the vertex buffer object
    m_tTcrossVbo->destroy();  //it destroys the previous buffer object and then it creates the new one
    delete m_tTcrossVbo;
    m_program->bind();
    m_vao->bind();
    m_tTcrossVbo = new QOpenGLBuffer;
    if (m_tTcrossVbo->create() == false)
        qDebug("Cross VBO signal error!\n");
    if (m_tTcrossVbo->bind() == false)
        qDebug("Cross VBO signal not bounded!!\n");
    m_tTcrossVbo->allocate(toolTipCrossData.data(), 8 * static_cast<int>(sizeof(GLfloat)));
    m_tTcrossVbo->setUsagePattern(QOpenGLBuffer::UsagePattern::DynamicDraw);
    m_tTcrossVbo->release();
    m_vao->release();
    m_program->release();
}

QVector<SigProperty> XY_GLWindow::convertSigProperties()
{
    QVector<SigProperty> prop;
    prop.resize(sig_properties.count());
    for (int i = 0; i < prop.count(); i++)
    {
        prop[i].color = sig_properties[i].color;
        prop[i].dotRendering = sig_properties[i].dotRendering;
        prop[i].lineRendering = sig_properties[i].lineRendering;
        prop[i].index = sig_properties[i].x_index;
        prop[i].line_width = sig_properties[i].line_width;
        prop[i].name = sig_properties[i].name;
    }

    return prop;
}

void XY_GLWindow::mousePressEvent(QMouseEvent *event)
{
    //THE TRANSFORMATION NEED TO BE ADAPTED WHEN SCROLLING FOLLOWS ZOOM OR VICEVERSA
    float width, height;
    float screen_x, screen_y;
    int idx;

    //saves the screen coordinates
    x1x = event->pos().x();
    x1y = event->pos().y();

    //we verify first if we clicked on the legend
    width = static_cast<float>(this->size().width());
    height = static_cast<float>(this->size().height());
    screen_x = (static_cast<float>(x1x) - (width / 2.0f)) / (width / 2.0f);
    screen_y = ((static_cast<float>(x1y) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));
    idx = legend->verifyMousePosition(screen_x, screen_y);

    if ((idx != -1) && (legendEnabled == true))  //we are on the legend
    {
        if (pan_enabled == true)
        {
            panLegend = true;
        }
    }
    else
    {
        if (pan_enabled == true)
            panLegend = false;
    }

    if (event->button() == Qt::LeftButton)
    {
        if (zoom_enabled == true)  //we start rendering the zoom area
        {
            render_zoom_area = true;
            if (zoom_mode == 0)
                this->setCursor(*zoomXYCursor);
            if (zoom_mode == 1)
                this->setCursor(*zoomXCursor);
            if (zoom_mode == 2)
                this->setCursor(*zoomYCursor);
        }
        if (pan_enabled == true)
            this->setCursor(*panCursor);
    }
}

void XY_GLWindow::mouseReleaseEvent(QMouseEvent *event)
{
    //THE TRANSFORMATION NEED TO BE ADAPTED WHEN SCROLLING FOLLOWS ZOOM OR VICEVERSA
    //PROBABLY JUST NEED TO ADAPT THE COORDINATES FROM SCREEN ACCORDING TO THE ZOOMED AREA INSTEAD THAN THE ORIGINAL AREA
    float width, height;
    float ax, ay, bx, by;
    float screen_x, screen_y;
    int idx;

    this->setCursor(Qt::ArrowCursor);
    x2x = event->pos().x();
    x2y = event->pos().y();

    if (event->button()== Qt::LeftButton)
    {
        //widget size
        width = static_cast<float>(this->size().width());
        height = static_cast<float>(this->size().height());

        //we calculate the points needed for transformation based on the visible area
        //we determine the topRight and bottomLeft points
        if (x1x > x2x)
        {
            ax = x1x; bx = x2x;
        }
        else
        {
            ax = x2x; bx = x1x;
        }
        if (x1y < x2y)  //this is because the screen y coordinates go with a different sign
        {
            ay = x1y; by = x2y;
        }
        else
        {
            ay = x2y; by = x1y;
        }
        //now we transform the coordinates
        calculate_visible_area();
        ax = ((ax / width) * visible_width) + static_cast<float>(btmLeft.x());
        ay = (((height - ay) / height) * visible_height) + static_cast<float>(btmLeft.y());
        bx = ((bx / width) * visible_width) + static_cast<float>(btmLeft.x());
        by = (((height - by) / height) * visible_height) + static_cast<float>(btmLeft.y());

        //we verify first if we clicked on the legend
        screen_x = (static_cast<float>(x2x) - (width / 2.0f)) / (width / 2.0f);
        screen_y = ((static_cast<float>(x2y) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));
        idx = legend->verifyMousePosition(screen_x, screen_y);

        if (idx == -1)  //then we are not operating in the legend box
        {
            if (zoom_enabled == true)
            {
                render_zoom_area = false;
                //Apply zoom mode
                if (zoom_mode == 1) //horizontal zoom => reset y coordinates
                {
                    if (ay >= by)
                    {
                        ay = static_cast<float>(topRight.y());
                        by = static_cast<float>(btmLeft.y());
                    }
                    else
                    {
                        ay = static_cast<float>(btmLeft.y());
                        by = static_cast<float>(topRight.y());
                    }
                }
                if (zoom_mode == 2)  //vertical zoom => reset x coordinates
                {
                    if (ax >= bx)
                    {
                        ax = static_cast<float>(topRight.x());
                        bx = static_cast<float>(btmLeft.x());
                    }
                    else
                    {
                        ax = static_cast<float>(btmLeft.x());
                        bx = static_cast<float>(topRight.x());
                    }
                }

                //here we will prepare the transformation matrix for the change of coordinates
                //the matrix will be exaluated in the shader

                //only if the coordinates are different we take action
                if ((ax != bx) && (ay != by))
                {
                    width = ax - bx;
                    height = ay - by;

                    transfMatrix.setToIdentity();
                    transfMatrix.scale(2.0f / width, 2.0f / height);
                    transfMatrix.translate((ax + bx) / 2.0f * (-1.0f), (ay + by) / 2.0f * (-1.0f));
                    grid->setTransformationMatrix(transfMatrix);

                    //update the visible area points
                    topRight.setX(static_cast<double>(ax));
                    topRight.setY(static_cast<double>(ay));
                    btmLeft.setX(static_cast<double>(bx));
                    btmLeft.setY(static_cast<double>(by));
                }
            }
        }
        else  //we are operating on the legend box
            if (pan_enabled == false)  //not panning => we click on the signal to toggle it
                legendSignalToggle(idx);
    }
    if (event->button() == Qt::RightButton)
    {
        width = static_cast<float>(this->size().width());
        height = static_cast<float>(this->size().height());

        //we verify first if we clicked on the legend
        screen_x = (static_cast<float>(x2x) - (width / 2.0f)) / (width / 2.0f);
        screen_y = ((static_cast<float>(x2y) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));
        idx = legend->verifyMousePosition(screen_x, screen_y);

        if (idx == -1) //not on the legend
        {
            transfMatrix.setToIdentity();
            grid->setTransformationMatrix(transfMatrix);
            topRight.setX(1.0); topRight.setY(1.0);
            btmLeft.setX(-1.0); btmLeft.setY(-1.0);
        }
        else
        {
            //we reposition the legend to its default position topLeft
            x_leg = 0.95f;
            y_leg = 0.95f;
        }
    }
    update();  //forces a repaint
}

void XY_GLWindow::mouseMoveEvent(QMouseEvent *event)
{
    //if we are in scroll mode then it's going to calculate the transfMatrix and apply it
    //we have initialized movetracking to false so this event occurs only when a mouse button is kept clicked
    float width, height;
    float ax, ay, bx, by;
    float screen_x, screen_y;

    width = static_cast<float>(this->size().width());
    height = static_cast<float>(this->size().height());

    //we verify first if we clicked on the legend
    screen_x = (static_cast<float>(x2x) - (width / 2.0f)) / (width / 2.0f);
    screen_y = ((static_cast<float>(x2y) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));

    if (pan_enabled == true)
    {
        width = static_cast<float>(this->size().width());
        height = static_cast<float>(this->size().height());
        x2x = event->pos().x();
        x2y = event->pos().y();

        if (panLegend == false)
        {
            //now we transform the coordinates
            calculate_visible_area();
            ax = x1x; ay = x1y; bx = x2x; by = x2y;
            ax = ((ax / width) * visible_width) + static_cast<float>(btmLeft.x());
            ay = (((height - ay) / height) * visible_height) + static_cast<float>(btmLeft.y());
            bx = ((bx / width) * visible_width) + static_cast<float>(btmLeft.x());
            by = (((height - by) / height) * visible_height) + static_cast<float>(btmLeft.y());

            transfMatrix.translate(bx - ax, by - ay);
            grid->setTransformationMatrix(transfMatrix);

            //updates the coordinates
            x1x = x2x; x1y = x2y;

            //updates the visible area
            topRight.setX(topRight.x() - static_cast<double>(bx - ax));
            btmLeft.setX(btmLeft.x() - static_cast<double>(bx - ax));
            topRight.setY(topRight.y() - static_cast<double>(by - ay));
            btmLeft.setY(btmLeft.y() - static_cast<double>(by - ay));
        }
        else
        {
            screen_x = (static_cast<float>(x2x - x1x) / (width / 2.0f));
            screen_y = ((static_cast<float>(x2y - x1y) / (height / 2.0f) * (-1.0f)));
            x_leg += screen_x;
            y_leg += screen_y;
            //updates the coordinates
            x1x = x2x; x1y = x2y;
        }
        update();
    }
    if (zoom_enabled == true)
    {
        x2x = event->pos().x();
        x2y = event->pos().y();

        update();
    }
}

void XY_GLWindow::wheelEvent(QWheelEvent *event)
{
    float width, height;
    float screen_x, screen_y;
    int idx;
    int x, y;

    const float step = 0.05f;
    float delta;  //a value of 120 (15 degrees) corresponds to 0.05 variation
    int alfa;

    x = event->x(); y = event->y();

    width = static_cast<float>(this->size().width());
    height = static_cast<float>(this->size().height());

    //we verify first if we clicked on the legend
    screen_x = (static_cast<float>(x) - (width / 2.0f)) / (width / 2.0f);
    screen_y = ((static_cast<float>(y) - (height / 2.0f)) / (height / 2.0f) * (-1.0f));
    idx = legend->verifyMousePosition(screen_x, screen_y);

    if (idx != -1)  // we scrolled on a signal name
    {
        if (idx < sig_properties.count())  //it is reliable
        {
            alfa = sig_properties[idx].color.alpha();
            delta = event->angleDelta().y() / 120.0f * step * 255.0f;  //brings
            alfa = alfa + static_cast<int>(delta);
            if (alfa > 255)
                alfa = 255;
            if (alfa < 0)
                alfa = 0;
            sig_properties[idx].color.setAlphaF(static_cast<float>(alfa) / 255.0f);
        }
    }

    emit updateSigProperties(sig_properties);
}
