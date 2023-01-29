/**
  *********************************************************************************************************************************************************
  @file     :fft_plot_window.cpp
  @brief    :Functions for FFT plot class
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

#include "fft_plot_window.h"
#include "glwindow.h"

fft_plot_Window::fft_plot_Window(QString title, int index, double frequency, int NPoints, appPreferencesStruct *pref, fontManager *font)
{
    plotIndex = index;

    Autoscale = false;
    StatsEnabled = true;

    preferences = pref;
    fontMgr = font;

    stepFrequency = static_cast<float>(frequency);
    autoscaleType = 0;
    MaxNPoints = NPoints;

    createActions();

    layout = new QVBoxLayout;

    glPlot = new GLWindow(this, title, pref, fontMgr);
    glPlot->setGridTimeBase(static_cast<double>(stepFrequency));
    connect(glPlot, &GLWindow::updateSigProperties, this, &fft_plot_Window::updateSigProperties);

    createSpinBoxes();

    layout->addWidget(statLabel);
    layout->addWidget(glPlot);
    layout->addWidget(bottomFill);

    glPlot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bottomFill->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QWidget *wid = new QWidget;

    wid->setLayout(layout);

    this->setCentralWidget(wid);

    connect(glPlot, SIGNAL (gridChanged()), this, SLOT(gridChanged()));
    connect(minYSB, SIGNAL (editingFinished()), this, SLOT (minYSBchanged()));
    connect(maxYSB, SIGNAL (editingFinished()), this, SLOT (maxYSBchanged()));
    connect(nSamplesSB, SIGNAL (editingFinished()), this, SLOT (nsamplesSBchanged()));
    connect(nSamplesSlider, SIGNAL(valueChanged(int)), this, SLOT(nsamplesSliderchanged(int)));
    connect(nSamplesSlider, SIGNAL(sliderReleased()), this, SLOT(nsamplesSliderreleased()));
    connect(xGridSlider, SIGNAL(valueChanged(int)), this, SLOT (xGridRatioChanged(int)));
    connect(yGridSlider, SIGNAL(valueChanged(int)), this, SLOT (yGridRatioChanged(int)));
    connect(autoscaleSlider, SIGNAL(valueChanged(int)), this, SLOT(autoscaleCenterChanged(int)));

    QLabel *statusBarLbl = new QLabel("CTRL-D for expanding the plot and CTRL-F for fullscreen.");
    statusBar()->addPermanentWidget(statusBarLbl);
}

void fft_plot_Window::createSpinBoxes()
{
    QLocale locale;
    float freqWidth;
    QString text;

    minYSB = new QDoubleSpinBox(this);
    maxYSB = new QDoubleSpinBox(this);
    nSamplesSB = new QSpinBox(this);
    xGridSlider = new QSlider(this);
    yGridSlider = new QSlider(this);
    autoscaleSlider = new QSlider(this);
    nSamplesSlider = new QSlider(this);

    float min, max;

    min = glPlot->get_Grid()->get_min_y();
    max = glPlot->get_Grid()->get_max_y();
    connect(nSamplesSlider, SIGNAL(valueChanged(int)), this, SLOT(nsamplesSliderchanged(int)));
    connect(nSamplesSlider, SIGNAL(sliderReleased()), this, SLOT(nsamplesSliderreleased()));

    minYSB->setMinimum(static_cast<double>(std::numeric_limits<float>::max() * -1));
    minYSB->setMaximum(static_cast<double>(std::numeric_limits<float>::max()));
    minYSB->setValue(static_cast<double>(min));
    minYSB->setSingleStep(1);
    minYSB->resize(minYSB->sizeHint());

    maxYSB->setMinimum(static_cast<double>(std::numeric_limits<float>::max() * -1.0f));
    maxYSB->setMaximum(static_cast<double>(std::numeric_limits<float>::max()));
    maxYSB->setValue(static_cast<double>(max));
    maxYSB->setSingleStep(1);
    maxYSB->resize(maxYSB->sizeHint());

    xGridSlider->setOrientation(Qt::Horizontal); xGridSlider->setTickPosition(QSlider::TicksBelow);
    xGridSlider->setMinimum(0); xGridSlider->setMaximum(5); xGridSlider->setSingleStep(1); xGridSlider->setValue(2);
    xGridSlider->resize(xGridSlider->sizeHint());

    yGridSlider->setOrientation(Qt::Horizontal); yGridSlider->setTickPosition(QSlider::TicksBelow);
    yGridSlider->setMinimum(0); yGridSlider->setMaximum(5); yGridSlider->setSingleStep(1); yGridSlider->setValue(2);
    yGridSlider->resize(yGridSlider->sizeHint());

    autoscaleSlider->setOrientation(Qt::Horizontal); autoscaleSlider->setTickPosition(QSlider::TicksBelow);
    autoscaleSlider->setMinimum(0); autoscaleSlider->setMaximum(100); autoscaleSlider->setSingleStep(1); autoscaleSlider->setValue(0);
    autoscaleSlider->resize(autoscaleSlider->sizeHint());

    freqWidth = static_cast<float>(MaxNPoints) * stepFrequency;
    text = "This FFT is calculated up to " + locale.toString(freqWidth, 'g', 3) + " Hz. The discretized step frequency is " + locale.toString(stepFrequency, 'g', 3) + " Hz.";
    statLabel = new QLabel(text);

    nSamplesSB->setMinimum(2);
    nSamplesSB->setMaximum(MaxNPoints);
    nSamplesSB->setValue(MaxNPoints);
    nSamplesSB->setSingleStep(1);
    nSamplesSB->resize(nSamplesSB->sizeHint());

    nSamplesSlider->setOrientation(Qt::Horizontal); nSamplesSlider->setTickPosition(QSlider::NoTicks);
    nSamplesSlider->setMinimum(2); nSamplesSlider->setMaximum(MaxNPoints); nSamplesSlider->setSingleStep(1); nSamplesSlider->setValue(MaxNPoints);
    nSamplesSlider->resize(nSamplesSlider->sizeHint()); nSamplesSlider->setTracking(true);

    QLabel *yminLabel = new QLabel("Y minimum value:");
    QLabel *ymaxLabel = new QLabel("Y maximum value:");
    QLabel *nsamplesLabel = new QLabel("Number of samples:");
    QLabel *gridyLabel = new QLabel("Y-grid fill:");
    QLabel *gridxLabel = new QLabel("X-grid fill:");
    QLabel *autoscaleLabel = new QLabel("Autoscale centering:");

    QVBoxLayout *vert1 = new QVBoxLayout;
    QVBoxLayout *vert2 = new QVBoxLayout;
    QVBoxLayout *vert3 = new QVBoxLayout;
    QVBoxLayout *vert4 = new QVBoxLayout;
    QVBoxLayout *vert5 = new QVBoxLayout;
    QVBoxLayout *vert6 = new QVBoxLayout;

    vert1->addWidget(yminLabel); vert1->addWidget(minYSB);
    vert2->addWidget(ymaxLabel); vert2->addWidget(maxYSB);
    vert3->addWidget(nsamplesLabel); vert3->addWidget(nSamplesSB); vert3->addWidget(nSamplesSlider);
    vert4->addWidget(gridyLabel); vert4->addWidget(yGridSlider);
    vert5->addWidget(gridxLabel); vert5->addWidget(xGridSlider);
    vert6->addWidget(autoscaleLabel); vert6->addWidget(autoscaleSlider);

    QVBoxLayout *vv1 = new QVBoxLayout;
    QVBoxLayout *vv2 = new QVBoxLayout;
    QVBoxLayout *vv3 = new QVBoxLayout;

    vv1->addLayout(vert1); vv1->addLayout(vert2);
    vv2->addLayout(vert4); vv2->addLayout(vert5); vv2->addLayout(vert6);
    vv3->addLayout(vert3); vv3->addStretch();

    QHBoxLayout *layoutSB = new QHBoxLayout;
    layoutSB->addLayout(vv1); layoutSB->addLayout(vv2); layoutSB->addLayout(vv3);

    bottomFill = new QWidget;
    bottomFill->setLayout(layoutSB);
}

void fft_plot_Window::thread_prepare_signal(int i, QVector<int> points, QVector<float> floats, QVector<void *> pointers)
{
    (void) floats;

    int start = points[0];
    int end = points[1];
    int n_p;
    float** buff_ptr = static_cast<float**>(pointers[0]);
    float* line_widths = static_cast<float*>(pointers[2]);

    int k;

    sig_properties[i].line_width = line_widths[i];

    //calculates statistics
    try   //insert to catch out of buffer accesses and/or divisions by zero
    {
        if (StatsEnabled == true)
        {
            sig_properties[i].stats.n_samples = 0;
            sig_properties[i].stats.mean = 0;

            if (StatsEnabled == true)  //we can calculate statistics
            {
                for (k = start; k <= end; k++)
                {
                    if (buff_ptr[i][k] > sig_properties[i].stats.max)
                        sig_properties[i].stats.max = buff_ptr[i][k];
                    if (buff_ptr[i][k] < sig_properties[i].stats.min)
                        sig_properties[i].stats.min = buff_ptr[i][k];
                    sig_properties[i].stats.n_samples += 1;
                    if (sig_properties[i].stats.n_samples != 0)
                        sig_properties[i].stats.mean += ((buff_ptr[i][k] - sig_properties[i].stats.mean) / static_cast<float>(sig_properties[i].stats.n_samples));  //updates average
                }
            }
        }
    } catch (...)
    {

    }

#ifdef USE_VERTEX_ID
    n_p = end - start + 1;
    memcpy(signal_buffer.data() + (i * n_p), buff_ptr[i] + start, n_p * sizeof(float));
#else
    index = -0.95f;
    k = 0;
    n_p = end - start + 1;
    for (j = start; j <= end; j++)
    {
        signal_buffer[(i * n_p * 2) + (k * 2)] = index; signal_buffer[(i * n_p * 2) + (k * 2)+1] = buff_ptr[i][j];
        index += step_x;
        k++;
    }
#endif

    signal_buffer_count = signal_buffer.count();
}

void fft_plot_Window::find_min_max_signals(int n_signals, int start, int end, float **buff_ptr, float *Min, float *Max)
{
    float min, max;
    int i, j;
    bool found;

    min = 0;
    max = 1;
    found = false;
    //we initialize min and max to the first valid value of the first signal
    for (i = 0; i < n_signals; i++)
        for (j = start; j <= end; j++)
        {
            if ((std::isnan(buff_ptr[i][j]) == false) && (std::isinf(buff_ptr[i][j]) == false))
            {
                found = true;
                min = buff_ptr[i][j];
                max = min;
            }
        }

    if (found == true)
    {
        for (i = 0; i < n_signals; i++)
            for (j = start; j <= end; j++)
            {
                if ((std::isnan(buff_ptr[i][j]) == false) && (std::isinf(buff_ptr[i][j]) == false))
                {
                    if (buff_ptr[i][j] < min)
                        min = buff_ptr[i][j];
                    if (buff_ptr[i][j] > max)
                        max = buff_ptr[i][j];
                }
            }
    }
    else {
        min = 0;
        max = 1;
    }

    *Min = static_cast<float>(min);
    *Max = static_cast<float>(max);
}

fft_plot_Window::~fft_plot_Window()
{
    delete glPlot;
}

void fft_plot_Window::addSignal(uint32_t index, QString name, QColor color)
{
    SigProperty p;

    p.index = index;
    p.name = name;
    p.color = color;
    p.dotRendering = false;
    p.lineRendering = true;
    p.stats.initialized = false;
    p.stats.max = 0.0;
    p.stats.min = 0.0;
    p.stats.n_samples = 0;
    p.stats.mean = 0.0;
    p.triggerAct = new QAction(this);
    p.triggerAct->setText(p.name);
    p.triggerAct->setCheckable(true);
    sig_properties.append(p);

    glPlot->setSigProperties(sig_properties);

    glPlot->setSigProperties(sig_properties);
}

void fft_plot_Window::removeSignal(uint32_t index)
{
    int i; int idx;

    idx = -1;
    for (i = 0; i < sig_properties.count(); i++)
        if (sig_properties[i].index == index)
            idx = i;

    qDebug() << "Idx = " << idx;

    if (idx == -1)
        return;

    //Remove the trigger action
    sig_properties.remove(idx);
    glPlot->setSigProperties(sig_properties);
}

void fft_plot_Window::setNPoints(int N)
{
    if (N > 1)
    {
        nSamplesSB->setValue(N);
        nsamplesSBchanged();
    }
}

void fft_plot_Window::setTitle(QString title)
{
    glPlot->get_Grid()->setTitle(title);
    glPlot->get_Grid()->recreate_Grid();
    this->setWindowTitle(title);
}

int fft_plot_Window::parallel_prepare_Signal_Data(int n_signals, int n_points, float **buff_ptr, QColor *colors, float *line_widths, QString *names)
{
    (void) names;

    int i; int n_p;
    float step_x;
    float x_axis;
    bool finished;
    float min, max;  //used for autoscale
    int start, end;
    float distance;

    if (n_signals != sig_properties.count())  //we have a mismatch between the number of signals passed and the number of signals declared in the plot => do not plot anything
        return -1;

    signal_buffer.clear();  //deletes the whole buffer

    if (n_points < 2)
        return -1;

    //we prepare the buffer by using resize (faster than using append)
    if (n_points <= glPlot->get_Grid()->get_N_points()) //n_points tells us how many points are contained in the signal buffers
    {
        n_p = n_points;
        start = 0;
        end = n_points - 1;
    }
    else
    {
        n_p = glPlot->get_Grid()->get_N_points();  //the plot will be filled
        start = 0;
        end = n_p - 1;
    }

    if (Autoscale == true)  //in this case, we need to rewrite the grid according to the min and max values of the signals to be plotted
    {
        find_min_max_signals(n_signals, start, end, buff_ptr, &min, &max);

        //we adjust the value according to the autoscaleType percentage
        distance = (abs(max - min) * static_cast<float>(autoscaleType) / 100.0f) / 2.0f;
        max += distance;
        min -= distance;

        //we can now prepare the grid and load the vertex buffer for the next paint operation
        glPlot->get_Grid()->recreate_Grid(max, min);
        emit gridChanged();  //signals a change in the grid
    }
    else
    {
        glPlot->get_Grid()->recreate_Grid();
    }

    x_axis = glPlot->get_Grid()->get_X_Axis();

#ifdef USE_VERTEX_ID
    signal_buffer.resize(n_signals * (end - start + 1));
#else
    signal_buffer.resize(n_signals * (end - start + 1) * 2);
#endif

    Number_of_Points = end - start + 1;
    n_p = Number_of_Points;

    indexes.clear();
    indexes.resize(n_signals);  //indexes to the locations of each signal in the buffer

    QVector<QFuture<void>> res;

    res.resize(n_signals);

    for (i = 0; i < n_signals; i++)
        indexes[i] = i * (end - start + 1);

    step_x = glPlot->get_Grid()->get_StepX();

    //Create arguments
    QVector<int> points;
    QVector<float> floats;
    QVector<void*> pointers;

    points.push_back(start); points.push_back(end);
    floats.push_back(step_x); floats.push_back(x_axis);
    pointers.push_back(static_cast<void*>(buff_ptr));
    pointers.push_back(static_cast<void*>(colors)); pointers.push_back(static_cast<void*>(line_widths));

    //I load the data in the right format from buff_ptr
    for (i = 0; i < n_signals; i++)  //Launches the parallel threads
    {
        res[i] = QtConcurrent::run(this, &fft_plot_Window::thread_prepare_signal, i, points, floats, pointers);
    }

    //waits for threads to finish
    finished = false;
    while (finished == false)
    {
        finished = true;
        for (i = 0; i < n_signals; i++)
            finished = finished && res[i].isFinished();
    }

    glPlot->parallel_prepare_Signal_Buffer(n_signals, n_p, &signal_buffer, sig_properties, indexes);

    return 0;
}

void fft_plot_Window::update()
{
    glPlot->update();
}

void fft_plot_Window::setGrid(bool en)
{
    gridAct->setChecked(en);
    glPlot->enable_grid(en);
    yTicksAct->setEnabled(en);
    xTicksAct->setEnabled(en);
}

void fft_plot_Window::closeEvent(QCloseEvent *event)
{
    (void) event;

    emit closing(plotIndex);
}

void fft_plot_Window::keyPressEvent(QKeyEvent *ev)
{
    bool state;

    if ((ev->key() == Qt::Key_D) && (ev->modifiers() == Qt::ControlModifier))
    {
        state = menuBar()->isVisible();
        state = !state;
        menuBar()->setVisible(state);
        this->toolBar->setVisible(state);
        statLabel->setVisible(state);
        bottomFill->setVisible(state);
        statusBar()->setVisible(state);
    }
    if ((ev->key() == Qt::Key_F) && (ev->modifiers() == Qt::ControlModifier))
    {
        state = isFullScreen();
        if (state == false)
        {
            this->showFullScreen();
        }
        else
        {
            this->showNormal();
        }
    }
    if (ev->key() == Qt::Key_Escape)
    {
        state = isFullScreen();
        if (state == true)
        {
            this->showNormal();
        }
    }
    if ((ev->key() == Qt::Key_S) && (ev->modifiers() == Qt::ControlModifier))
        exportFigure();
    if ((ev->key() == Qt::Key_G) && (ev->modifiers() == Qt::ControlModifier))
        gridAct->trigger();
    if ((ev->key() == Qt::Key_H) && (ev->modifiers() == Qt::ControlModifier))
        autoscaleAct->trigger();
    if ((ev->key() == Qt::Key_L) && (ev->modifiers() == Qt::ControlModifier))
        legendAct->trigger();
}

void fft_plot_Window::exportFigure()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Plot Figure", "", "Bitmap (*.bmp);; JPEG (*.jpg)");

    if (filename.isEmpty() == false)
        glPlot->getPlotFigure().save(filename);
}

void fft_plot_Window::closeWindow()
{
    close();
}

void fft_plot_Window::triggerAutoscale()
{
    if (autoscaleAct->isChecked() == true)
    {
        Autoscale = true;
        glPlot->set_autoscale_mode(true);
        minYSB->setEnabled(false);
        maxYSB->setEnabled(false);
        nSamplesSB->setEnabled(false);
        nSamplesSlider->setEnabled(false);
    }
    else
    {
        Autoscale = false;
        glPlot->set_autoscale_mode(false);
        minYSB->setEnabled(true);
        maxYSB->setEnabled(true);
        nSamplesSB->setEnabled(true);
        nSamplesSlider->setEnabled(true);
    }
}

void fft_plot_Window::triggerZoomXY()
{
    if (zoomXYAct->isChecked() == true)
    {
        zoomXAct->setChecked(false); zoomYAct->setChecked(false); panAct->setChecked(false);
        glPlot->set_zoom_mode(0);  //XY Zoom
        glPlot->enable_zoom(true);
    }
    else
        glPlot->enable_zoom(false);
}

void fft_plot_Window::triggerZoomX()
{
    if (zoomXAct->isChecked() == true)
    {
        zoomXYAct->setChecked(false); zoomYAct->setChecked(false); panAct->setChecked(false);
        glPlot->set_zoom_mode(1);  //X Zoom
        glPlot->enable_zoom(true);
    }
    else
        glPlot->enable_zoom(false);
}

void fft_plot_Window::triggerZoomY()
{
    if (zoomYAct->isChecked() == true)
    {
        zoomXAct->setChecked(false); zoomXYAct->setChecked(false); panAct->setChecked(false);
        glPlot->set_zoom_mode(2);  //Y Zoom
        glPlot->enable_zoom(true);
    }
    else
        glPlot->enable_zoom(false);
}

void fft_plot_Window::triggerPan()
{
    if (panAct->isChecked() == true)
    {
        zoomXAct->setChecked(false); zoomXYAct->setChecked(false); zoomYAct->setChecked(false);
        glPlot->enable_pan(true);
    }
    else
        glPlot->enable_pan(false);
}

void fft_plot_Window::triggerLegend()
{
    if (legendAct->isChecked() == true)
    {
        glPlot->enable_legend(true);
    }
    else
        glPlot->enable_legend(false);
}

void fft_plot_Window::triggerStats()
{
    if (statAct->isChecked() == true)
    {
        StatsEnabled = true;
        glPlot->enable_stats(true);
    }
    else
    {
        StatsEnabled = false;
        glPlot->enable_stats(false);
    }
}

void fft_plot_Window::triggerResetStats()
{
    int i;

    for (i = 0; i < sig_properties.count(); i++)
    {
        sig_properties[i].stats.initialized = false;
        sig_properties[i].stats.mean = 0.0;
        sig_properties[i].stats.min = 0.0;
        sig_properties[i].stats.max = 0.0;
    }
}

void fft_plot_Window::triggerGrid()
{
    if (gridAct->isChecked() == true)
    {
        glPlot->enable_grid(true);
        yTicksAct->setEnabled(true);
        xTicksAct->setEnabled(true);
    }
    else
    {
        glPlot->enable_grid(false);
        yTicksAct->setEnabled(false);
        xTicksAct->setEnabled(false);
    }
    emit replot(plotIndex);
    emit gridTriggered();
}

void fft_plot_Window::triggerYTicks()
{
    if (yTicksAct->isChecked() == true)
        glPlot->enable_yLabels(true);
    else
        glPlot->enable_yLabels(false);
    emit replot(plotIndex);
}

void fft_plot_Window::triggerXTicks()
{
    if (xTicksAct->isChecked() == true)
        glPlot->enable_xLabels(true);
    else
        glPlot->enable_xLabels(false);
    emit replot(plotIndex);
}

void fft_plot_Window::triggerTitle()
{
    if (titleAct->isChecked() == true)
        glPlot->enable_title(true);
    else
        glPlot->enable_title(false);
    emit replot(plotIndex);
}

void fft_plot_Window::minYSBchanged()
{
    if (minYSB->isEnabled() == true)
    {
        //we set the new value
        glPlot->setGridMinY(minYSB->value());
        emit replot(plotIndex);
    }
}

void fft_plot_Window::maxYSBchanged()
{
    if (maxYSB->isEnabled() == true)
    {
        //we set the new value
        glPlot->setGridMaxY(maxYSB->value());
        emit replot(plotIndex);
    }
}

void fft_plot_Window::nsamplesSBchanged()
{
    float freqWidth;
    QLocale locale;

    if (nSamplesSB->isEnabled() == true)
    {
        //we set the new value
        glPlot->setGridNSamples(nSamplesSB->value());
        nSamplesSlider->blockSignals(true);
        nSamplesSlider->setValue(nSamplesSB->value());
        nSamplesSlider->blockSignals(false);
        freqWidth = static_cast<float>(glPlot->get_Grid()->get_N_points()) * stepFrequency;
        statLabel->setText("This FFT is calculated up to " + locale.toString(freqWidth, 'g', 3) + " Hz. The discretized step frequency is " + locale.toString(stepFrequency, 'g', 3) + " Hz.");
        emit replot(plotIndex);
    }
}

void fft_plot_Window::nsamplesSliderchanged(int)
{
    float freqWidth;
    QLocale locale;

    if (nSamplesSlider->isEnabled() == true)
    {
        //we set the new value
        glPlot->setGridNSamples(nSamplesSlider->value());
        nSamplesSB->blockSignals(true);
        nSamplesSB->setValue(nSamplesSlider->value());
        nSamplesSB->blockSignals(false);
        freqWidth = static_cast<float>(glPlot->get_Grid()->get_N_points()) * stepFrequency;
        statLabel->setText("This FFT is calculated up to " + locale.toString(freqWidth, 'g', 3) + " Hz. The discretized step frequency is " + locale.toString(stepFrequency, 'g', 3) + " Hz.");

        emit replot(plotIndex);
    }
}

void fft_plot_Window::nsamplesSliderreleased()
{

}

void fft_plot_Window::xGridRatioChanged(int value)
{
    double v;
    v = 5.0 * pow(2, static_cast<double>(value));
    glPlot->get_Grid()->set_x_grid_ratio(static_cast<int>(v));
    emit replot(plotIndex);
}

void fft_plot_Window::yGridRatioChanged(int value)
{
    double v;
    v = 5.0 * pow(2, static_cast<double>(value));
    glPlot->get_Grid()->set_y_grid_ratio(static_cast<int>(v));
    emit replot(plotIndex);
}

void fft_plot_Window::autoscaleCenterChanged(int value)
{
    autoscaleType = value;
    emit replot(plotIndex);
}

void fft_plot_Window::updateSigProperties(QVector<SigProperty> properties)
{
    sig_properties = properties;
}

void fft_plot_Window::gridChanged()
{
    //the grid has changed, thus we update the spinBoxes
    minYSB->setValue(static_cast<double>(glPlot->get_Grid()->get_min_y()));
    maxYSB->setValue(static_cast<double>(glPlot->get_Grid()->get_max_y()));
    nSamplesSB->setValue(static_cast<int>(glPlot->get_Grid()->get_N_points()));
}

void fft_plot_Window::createActions()
{
    QSize iconSize(32, 32);

    triggerSignals.clear();

    triggerGroup = new QActionGroup(this);
    triggerGroup->setExclusive(true);

    exportFigAct = new QAction(this);
    exportFigAct->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    exportFigAct->setCheckable(false);
    exportFigAct->setToolTip("Export plot figure (CTRL+S)");
    exportFigAct->setText("&Export Plot Figure");
    connect(exportFigAct, &QAction::triggered, this, &fft_plot_Window::exportFigure);

    closeAct = new QAction(this);
    closeAct->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    closeAct->setCheckable(false);
    closeAct->setToolTip("Close Plot");
    closeAct->setText("&Close");
    connect(closeAct, &QAction::triggered, this, &fft_plot_Window::closeWindow);

    gridAct = new QAction(this);
    gridAct->setIcon(QIcon(":/Icons/Icons/grid.ico"));
    gridAct->setCheckable(true);
    gridAct->setChecked(false);
    gridAct->setToolTip("Grid (CTRL+G)");
    gridAct->setText("&Grid");
    connect(gridAct, &QAction::triggered, this, &fft_plot_Window::triggerGrid);

    yTicksAct = new QAction(this);
    yTicksAct->setIcon(QIcon(":/Icons/Icons/yTicks.ico"));
    yTicksAct->setCheckable(true);
    yTicksAct->setChecked(true);
    yTicksAct->setEnabled(false);  //default
    yTicksAct->setToolTip("Shows (hides) y-axis labels");
    yTicksAct->setText("&Y-Grid Labels");
    connect(yTicksAct, &QAction::triggered, this, &fft_plot_Window::triggerYTicks);

    xTicksAct = new QAction(this);
    xTicksAct->setIcon(QIcon(":/Icons/Icons/xTicks.ico"));
    xTicksAct->setCheckable(true);
    xTicksAct->setChecked(true);
    xTicksAct->setEnabled(false);  //default
    xTicksAct->setToolTip("Shows (hides) x-axis labels");
    xTicksAct->setText("&X-Grid Labels");
    connect(xTicksAct, &QAction::triggered, this, &fft_plot_Window::triggerXTicks);

    titleAct = new QAction(this);
    titleAct->setIcon(QIcon(":/Icons/Icons/title.ico"));
    titleAct->setCheckable(true);
    titleAct->setChecked(false);
    titleAct->setToolTip("Toggle the plot title");;
    titleAct->setText("&Title");
    connect(titleAct, &QAction::triggered, this, &fft_plot_Window::triggerTitle);

    autoscaleAct = new QAction(this);
    autoscaleAct->setIcon(QIcon(":/Icons/Icons/autoscale.ico"));
    autoscaleAct->setCheckable(true);
    autoscaleAct->setChecked(false);
    autoscaleAct->setToolTip("Autoscale (CTRL+H)");
    autoscaleAct->setText("&Autoscale");
    connect(autoscaleAct, &QAction::triggered, this, &fft_plot_Window::triggerAutoscale);

    zoomXYAct = new QAction(this);
    zoomXYAct->setIcon(QIcon(":/Icons/Icons/zoomXY.ico"));
    zoomXYAct->setCheckable(true);
    zoomXYAct->setChecked(false);
    zoomXYAct->setToolTip("Zoom In");
    zoomXYAct->setText("&Zoom");
    connect(zoomXYAct, &QAction::triggered, this, &fft_plot_Window::triggerZoomXY);

    zoomXAct = new QAction(this);
    zoomXAct->setIcon(QIcon(":/Icons/Icons/zoomX.ico"));
    zoomXAct->setCheckable(true);
    zoomXAct->setChecked(false);
    zoomXAct->setToolTip("Zoom Horizontally");
    zoomXAct->setText("Zoom &Horizontally");
    connect(zoomXAct, &QAction::triggered, this, &fft_plot_Window::triggerZoomX);

    zoomYAct = new QAction(this);
    zoomYAct->setIcon(QIcon(":/Icons/Icons/zoomY.ico"));
    zoomYAct->setCheckable(true);
    zoomYAct->setChecked(false);
    zoomYAct->setToolTip("Zoom Vertically");
    zoomYAct->setText("Zoom &Vertically");
    connect(zoomYAct, &QAction::triggered, this, &fft_plot_Window::triggerZoomY);

    panAct = new QAction(this);
    panAct->setIcon(QIcon(":/Icons/Icons/panXY.ico"));
    panAct->setCheckable(true);
    panAct->setChecked(false);
    panAct->setToolTip("Pan");
    panAct->setText("&Pan");
    connect(panAct, &QAction::triggered, this, &fft_plot_Window::triggerPan);

    legendAct = new QAction(this);
    legendAct->setIcon(QIcon(":/Icons/Icons/legend.ico"));
    legendAct->setCheckable(true);
    legendAct->setChecked(true);  //checked by default
    legendAct->setToolTip("Show legend (CTRL+L)");
    legendAct->setText("&Legend");
    connect(legendAct, &QAction::triggered, this, &fft_plot_Window::triggerLegend);

    statAct = new QAction(this);
    statAct->setIcon(QIcon(":/Icons/Icons/Stats.ico"));
    statAct->setCheckable(true);
    statAct->setChecked(true);  //checked by default
    statAct->setToolTip("Show statistics");
    statAct->setText("&Statistics");
    connect(statAct, &QAction::triggered, this, &fft_plot_Window::triggerStats);

    resetStatAct = new QAction(this);
    resetStatAct->setIcon(QIcon(":/Icons/Icons/resetStats.ico"));
    resetStatAct->setCheckable(false);
    resetStatAct->setChecked(false);  //not checked by default
    resetStatAct->setToolTip("Reset statistics");
    resetStatAct->setText("&Reset statistics");
    connect(resetStatAct, &QAction::triggered, this, &fft_plot_Window::triggerResetStats);

    fileMenu = menuBar()->addMenu("&File");
    viewMenu = menuBar()->addMenu("&View");
    toolMenu = menuBar()->addMenu("&Tool");

    toolBar = this->addToolBar("File");
    toolBar->setIconSize(iconSize);

    //Prepares the Tool Bar
    toolBar->addAction(exportFigAct);
    toolBar->addSeparator();
    toolBar->addAction(gridAct);
    toolBar->addAction(yTicksAct);
    toolBar->addAction(xTicksAct);
    toolBar->addAction(titleAct);
    toolBar->addSeparator();
    toolBar->addAction(autoscaleAct);
    toolBar->addSeparator();
    toolBar->addSeparator();
    toolBar->addAction(zoomXYAct);
    toolBar->addAction(zoomXAct);
    toolBar->addAction(zoomYAct);
    toolBar->addSeparator();
    toolBar->addAction(panAct);
    toolBar->addSeparator();
    toolBar->addAction(legendAct);
    toolBar->addAction(statAct);
    toolBar->addAction(resetStatAct);
    toolBar->addSeparator();
    toolBar->addAction(closeAct);

    //Prepares the Menu Bar
    fileMenu->addAction(exportFigAct);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);

    viewMenu->addAction(autoscaleAct);
    viewMenu->addSeparator();
    viewMenu->addAction(zoomXYAct);
    viewMenu->addAction(zoomXAct);
    viewMenu->addAction(zoomYAct);
    viewMenu->addSeparator();
    viewMenu->addAction(panAct);

    toolMenu->addSeparator();
    toolMenu->addAction(gridAct);
    toolMenu->addAction(yTicksAct);
    toolMenu->addAction(xTicksAct);
    toolMenu->addAction(titleAct);
    toolMenu->addSeparator();
    toolMenu->addAction(legendAct);
    toolMenu->addAction(statAct);
    toolMenu->addAction(resetStatAct);
}
