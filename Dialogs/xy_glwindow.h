/**
  *********************************************************************************************************************************************************
  @file     :xy_glwindow.h
  @brief    :Header for xy GLwindow class (window containing XY OpenGL plot)
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

#ifndef XY_GLWIDGET_H
#define XY_GLWIDGET_H

#include "Creators/grid_xy.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QVector2D>
#include <QVector4D>
#include <QImage>
#include <QCursor>
#include <QPixmap>
#include <QTimer>
#include <QOpenGLShaderProgram>

#include "FontManager/fontmanager.h"
#include "Managers/prefmanager.h"
#include "Creators/tooltipcreator.h"
#include "Creators/textrenderer.h"
#include "Creators/legendCreator.h"
#include "Creators/statcreator.h"
#include "definitions.h"

class XY_GLWindow : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    XY_GLWindow(QWidget *parent = 0, QString title = "No title defined", appPreferencesStruct *pref = 0, fontManager *font = 0);
    ~XY_GLWindow() override;

    void setSigProperties(QVector<XY_SigProperty> properties) { sig_properties = properties; }
    Grid_XY *get_Grid() { return grid; }
    void setGridMinY(double minY);
    void setGridMaxY(double maxY);
    void setGridMinX(double minX);
    void setGridMaxX(double maxX);
    void setGridTimeBase(double TimeBase);
    void prepare_Signal_Buffer(int n_signals, int n_points, QVector<GLfloat> *x_buffer, QVector<GLfloat> *y_buffer, QVector<XY_SigProperty> prop, QVector<int> idx, int NSamples);
    void set_zoom_mode(int mode) { if ((mode >= 0) && (mode <= 2)) zoom_mode = mode; }
    void enable_zoom(bool enable);
    void enable_pan(bool enable) { pan_enabled = enable; zoom_enabled = false; }
    void enable_legend(bool enable) { legendEnabled = enable; }
    void enable_grid(bool enable) { grid->setDrawGrid(enable); grid->recreate_Grid(); prepare_grid_buffer(); }
    void enable_yLabels(bool enable) { grid->setDrawYLabels(enable); grid->recreate_Grid(); prepare_grid_buffer(); }
    void enable_xLabels(bool enable) { grid->setDrawXLabels(enable); grid->recreate_Grid(); prepare_grid_buffer(); }
    void enable_title(bool enable) { grid->setDrawTitle(enable); }
    bool get_zoom_status(void) { return zoom_enabled; }
    void set_past_value_gain(float value) { pastValueGain = value; }
    void set_autoscale_mode(bool enable);
    QImage getPlotFigure(void);
    void updateFonts();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

signals:
    void gridChanged();
    void updateSigProperties(QVector<XY_SigProperty> properties);

public slots:
    void cleanup();

private:
    appPreferencesStruct *preferences;
    fontManager *fontMgr;

    QOpenGLBuffer *m_x_signalVbo;  //Buffer used for plotting lines => processed by m_program
    QOpenGLBuffer *m_y_signalVbo;  //Buffer used for plotting lines => processed by m_program
    QOpenGLVertexArrayObject *m_vao;
    QOpenGLVertexArrayObject *m_xy_vao;
    QOpenGLVertexArrayObject *m_zoom_vao;
    QOpenGLBuffer *m_gridVbo;  //Buffer used for plotting grid => processed by m_program
    QOpenGLBuffer *m_tTcrossVbo;  //Buffer used for plotting the tooltip indicator cross
    QOpenGLBuffer *m_zoomVbo;  //Buffer used for plotting the zoom area => processed by m_program_zoom
    QOpenGLBuffer *m_triggerVbo;  //Buffer used for plotting the trigger line => processed by m_program
    QOpenGLShaderProgram *m_program;
    QOpenGLShaderProgram *m_xy_program;
    QOpenGLShaderProgram *m_program_dot;
    QOpenGLShaderProgram *m_program_zoom;
    QSurface *m_surf;
    QOpenGLContext *m_context;
    //User for legend and other writing
    QOpenGLTexture *gridTex;
    QOpenGLTexture *legendTex;
    QOpenGLTexture *toolTipTex;
    QOpenGLShaderProgram *m_program_tex;
    QOpenGLVertexArrayObject *m_tex_vao;
    QOpenGLBuffer *m_texVbo;
    bool dotRendering;

    Grid_XY *grid;

    QMatrix4x4 transfMatrix;  //used for zoom, scroll and autoscale operation => initially set to identity
    QMatrix4x4 identityMatrix;  //just an identity matrix

    int m_in_colorLoc;
    int m_in_paramsLoc;
    int m_transfMatrixLoc;  //address of the uniform transfMatrix
    int m_in_convFactDotLoc;
    int m_in_colorLocDot;
    int m_in_paramsLocDot;
    int m_transfMatrixLocDot;
    int m_xy_pastValueGainDotLoc;
    int m_xy_n_samplesDotLoc;
    int m_xy_instanceDotLoc;
    int m_screenSizeLoc;

    int m_xy_conv_factLoc;
    int m_xy_in_paramsLoc;
    int m_xy_in_colorLoc;
    int m_xy_transfMatrixLoc;
    int m_xy_screenSizeLoc;
    int m_xy_pastValueGainLoc;
    int m_xy_n_samplesLoc;
    int m_xy_instanceLoc;

    QString plotTitle;

    QColor backgroundCol;
    double backR, backG, backB, backA;

    float pastValueGain;

    //Used for zoom and scroll operation
    QPointF topRight, btmLeft;  //define the actually visible area
    float visible_width, visible_height;  //width and height of the visible area
    float x1x, x1y;  //screen coordinates of the mouse pointer
    float x2x, x2y;
    int zoom_mode;  //0 => standard zoom; 1 => horizontal zoom; 2 => vertical zoom
    bool zoom_enabled;  //false => no zoom; true => zoom
    bool pan_enabled;  //false => no pan; true => pan

    QVector<float> zoom_buffer;
    bool render_zoom_area;
    QColor zoom_area_color;

    float trigger_level;

    //Used for texture writing
    QVector<float> texture_buffer;

    //Legend and Stats and Tooltip
    legendCreator *legend;
    bool legendEnabled;
    float x_leg, y_leg;  //position of the legend
    bool panLegend;
    bool toolTipEnabled;
    toolTipCreator *tooltip;
    float x_tooltip, y_tooltip;  //writing coordinate
    int x_tTint, y_tTint;  //screen coordinates
    QColor toolTipColor;
    QVector<float> toolTipCrossData;

    int N_signals;  //indicates the number of signals that need to be displayed
    int Number_of_Points;  //number of points for the signals to be plotted
    QVector<int> indexes;  //all the signals are contained into one single buffer. in this vector we store the location at which each signal starts

    //Mouse Cursors during pan and zoom operation
    QCursor *zoomXYCursor;
    QCursor *zoomXCursor;
    QCursor *zoomYCursor;
    QCursor *panCursor;

    QVector<XY_SigProperty> sig_properties;

    void calculate_visible_area(void);
    void prepare_zoom_area(void);
    void prepare_grid_buffer(void);  //used when in autoscale
    void legendSignalToggle(int idx);
    void calculateCoordinateFromCursor(int screen_x, int screen_y, float *coordX, float *coordY);
    void prepareToolTipCrossData();
    QVector<SigProperty> convertSigProperties();
};

#endif
