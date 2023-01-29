/**
  *********************************************************************************************************************************************************
  @file     :sgnalplottermanager.cpp
  @brief    :Functions of the Signal Plotter Manager Class
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

#include "sgnalplottermanager.h"

SgnalPlotterManager::SgnalPlotterManager(appPreferencesStruct *pref, fontManager *font, filenameGenerator *gen)
{
    N_Plots = 0;
    N_Signals = 0;
    maxNData = 1000;  //by default
    command_Rec = false;

    fftMgr = new fftManager(pref, font);  //we create the FFT manager
    connect(fftMgr, &fftManager::gridActTriggered, this, &SgnalPlotterManager::gridFFTChanged);

    fileGen = gen;

    preferences = pref;

    //Loads now all the fonts
    fontMgr = font;

    //Prepares the signal list widget
    sigView = new QTableView(this);
    QLabel *sigLabel = new QLabel("Signals received (selected signals will be included when exporting):");
    sigView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    sigView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    //Prepares the thumbnail widget and layout
    thumbnailPlots = new QListWidget(this);
    QLabel *plotterLabel = new QLabel("Plotter windows:");

    //Prepares pushbuttons
    addPlotPB = new QPushButton;
    remPlotPB = new QPushButton;
    sigSettPB = new QPushButton;
    associatePB = new QPushButton;
    deassociatePB = new QPushButton;
    associateXYPB = new QPushButton;
    deassociateXYPB = new QPushButton;
    titleChangePB = new QPushButton;
    addPlotPB->setIcon(QIcon(":/Icons/Icons/Add.ico"));
    addPlotPB->setToolTip("Add a new plot");
    remPlotPB->setIcon(QIcon(":/Icons/Icons/Remove.ico"));
    remPlotPB->setToolTip("Remove a plot");
    sigSettPB->setIcon(QIcon(":/Icons/Icons/Sig_Settings.ico"));
    sigSettPB->setToolTip("Change signal settings");
    associatePB->setIcon(QIcon(":/Icons/Icons/Add.ico"));
    associatePB->setToolTip("Associate a signal to a plot");
    deassociatePB->setIcon(QIcon(":/Icons/Icons/Remove.ico"));
    deassociatePB->setToolTip("Remove the association between a signal and a plot");
    associateXYPB->setIcon(QIcon(":/Icons/Icons/AddXY.ico"));
    associateXYPB->setToolTip("Associate a signal to a XY plot");
    deassociateXYPB->setIcon(QIcon(":/Icons/Icons/RemoveXY.ico"));
    deassociateXYPB->setToolTip("Remove the association between a signal and a XY plot");
    titleChangePB->setIcon(QIcon(":/Icons/Icons/titleChange.ico"));
    titleChangePB->setToolTip("Change plot title");
    addPlotPB->setFixedSize(addPlotPB->sizeHint());
    remPlotPB->setFixedSize(remPlotPB->sizeHint());
    sigSettPB->setFixedSize(sigSettPB->sizeHint());
    associatePB->setFixedSize(associatePB->sizeHint());
    deassociatePB->setFixedSize(deassociatePB->sizeHint());
    titleChangePB->setFixedSize(titleChangePB->sizeHint());
    connect(addPlotPB, &QPushButton::clicked, this, &SgnalPlotterManager::addPlotTriggered);
    connect(remPlotPB, &QPushButton::clicked, this, &SgnalPlotterManager::remPlotTriggered);
    connect(sigSettPB, &QPushButton::clicked, this, &SgnalPlotterManager::sigSetTriggered);
    connect(associatePB, &QPushButton::clicked, this, &SgnalPlotterManager::associateTriggered);
    connect(deassociatePB, &QPushButton::clicked, this, &SgnalPlotterManager::deassociateTriggered);
    connect(associateXYPB, &QPushButton::clicked, this, &SgnalPlotterManager::associateXYTriggered);
    connect(deassociateXYPB, &QPushButton::clicked, this, &SgnalPlotterManager::deassociateXYTriggered);
    connect(titleChangePB, &QPushButton::clicked, this, &SgnalPlotterManager::changeTitleTriggered);

    QSplitter *split = new QSplitter(Qt::Vertical);
    QVBoxLayout *layout1 = new QVBoxLayout;
    QVBoxLayout *layout2 = new QVBoxLayout;
    QHBoxLayout *layout3 = new QHBoxLayout;
    QHBoxLayout *layout4 = new QHBoxLayout;

    layout4->addWidget(sigSettPB); layout4->addWidget(associatePB); layout4->addWidget(deassociatePB);
    layout4->addWidget(associateXYPB); layout4->addWidget(deassociateXYPB); layout4->addStretch();
    layout3->addWidget(addPlotPB); layout3->addWidget(remPlotPB); layout3->addWidget(titleChangePB); layout3->addStretch();

    layout1->addWidget(sigLabel);
    layout1->addWidget(sigView);
    layout1->addLayout(layout4);
    layout2->addWidget(plotterLabel);
    layout2->addWidget(thumbnailPlots);
    layout2->addLayout(layout3);
    layout2->addWidget(fftMgr);

    QWidget *topW = new QWidget;
    QWidget *btmW = new QWidget;

    topW->setLayout(layout1);
    btmW->setLayout(layout2);
    split->addWidget(topW); split->addWidget(btmW);

    thumbnailPlots->setIconSize(QSize(200, 200));
    thumbnailPlots->setResizeMode(QListWidget::Adjust);
    thumbnailPlots->setViewMode(QListWidget::IconMode);
    thumbnailPlots->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
    thumbnailPlots->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    thumbnailPlots->resize(sizeHint());
    connect(thumbnailPlots, &QListWidget::itemDoubleClicked, this, &SgnalPlotterManager::thumbDoubleClick);

    sigViewModel = new QStandardItemModel;
    prepareSigViewModel();

    sigView->setModel(sigViewModel);

    thumbnailPlots->setSizePolicy(QSizePolicy::MinimumExpanding , QSizePolicy::MinimumExpanding);
    sigView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(split);
    setLayout(layout);

    setMinimumSize(150, 100);
    resize(sizeHint());

    thumbTimer = new QTimer;
    thumbTimer->setInterval(2000);  //2 seconds
    connect(thumbTimer, &QTimer::timeout, this, &SgnalPlotterManager::updateThumbs);
    thumbTimer->start();
}

SgnalPlotterManager::~SgnalPlotterManager()
{
    int i;

    qDebug() << "Deleting Signal Plotter Manager \n";

    //here we destroy all the elements
    for (i = 0; i < static_cast<int>(N_Plots); i++)
    {
        Plot_Pool[i].plot->close();
        delete Plot_Pool[i].plot;
    }
    for (i = 0; i < static_cast<int>(N_XY_Plots); i++)
    {
        XY_Plot_Pool[i].plot->close();
        delete XY_Plot_Pool[i].plot;
    }

    delete fftMgr;
}

uint32_t SgnalPlotterManager::Add_Signal(QString signal_name, int type, float scaling)
{
    uint32_t index;
    QList<QStandardItem*> newrow;

    index = get_new_signal_index();
    Signal_Data *sig = new Signal_Data(signal_name, index, type, scaling);

    sig->set_Record(false);  //by default

    Signal_Pool.append(sig);

    N_Signals = static_cast<uint32_t>(Signal_Pool.count());

    QStandardItem *item;

    //we add it to the signal list view
    item = new QStandardItem(signal_name);
    item->setCheckable(true);
    item->setCheckState(Qt::Checked);
    newrow.append(item);
    newrow.append(new QStandardItem("None"));
    newrow.append(new QStandardItem(get_type_text(type)));
    newrow.append(new QStandardItem(QString::number(static_cast<double>(scaling))));
    sigViewModel->appendRow(newrow);

    viewSigAss.insert(item, index);

    return index;  //this is also the index associated to the Signal
}

void SgnalPlotterManager::Remove_Signal(uint32_t index)
{
    int i = find_signal_by_index(index);

    if (i != -1)
    {
        delete Signal_Pool[i];  //deallocate the signal before removing it
        Signal_Pool.remove(i);
    }
}

void SgnalPlotterManager::Pass_Data_to_Signal(uint32_t index, float *data, int N_Data)
{
    int i = find_signal_by_index(index);
    int pos;

    if (i != -1)
        Signal_Pool[i]->Add_Data(data, N_Data, maxNData);

    int j;
    //we check if the fftManager is free and in that case we update the fftmanager data as well
    if (fftMgr->getStatus() == false)  //the fftManager is free
    {
        for (i = 0; i < fftWdwList.count(); i++)
        {
            //per each FFT plot, we update the signals data
            QVector<int> sigIdx = fftMgr->getSignalIndexPerWindow(fftWdwList[i]);
            for (j = 0; j < sigIdx.count(); j++)
            {
                pos = find_signal_by_index(static_cast<uint32_t>(sigIdx[j]));
                fftMgr->updateSigData(fftWdwList[i], sigIdx[j], Signal_Pool[pos]->retrieve_Data_Pointer(), static_cast<int>(Signal_Pool[pos]->Count_Data()));
            }
        }
    }
}

void SgnalPlotterManager::Clear_Signal_Data(uint32_t index)
{
    int i = find_signal_by_index(index);

    if (i != -1)
        Signal_Pool[i]->Clean_Data();
}

void SgnalPlotterManager::Pass_Cmd_to_Pool(uint8_t *cmd, int N_Data)
{
    unsigned long long N, old_N;
    unsigned long long size = sizeof(uint8_t);

    //if N_data is <= 0 we exit
    if (N_Data <= 0)
        return;

    old_N = static_cast<unsigned long long>(command_Pool.size());
    //we resize with the new dimension
    command_Pool.resize(static_cast<int>(old_N + static_cast<unsigned long long>(N_Data)));

    memcpy(command_Pool.data() + old_N, cmd, static_cast<unsigned long long>(N_Data) * size);

    if (command_Rec == false)
    {
        //if the new data overcome the maxData allowed, we cut
        N = static_cast<unsigned long long>(command_Pool.size());
        if (N > maxNData)
            command_Pool.erase(command_Pool.begin(), command_Pool.begin() + static_cast<int>((N - maxNData)));
    }
}

uint32_t SgnalPlotterManager::Add_Plot(QString plot_name, double frequency)
{
    uint32_t index;
    plot_Window *p;

    index = get_new_plot_index();
    Plot_Structure plot;
    p = new plot_Window(plot_name, static_cast<int>(index), frequency, preferences, fontMgr);

    plot.index = index;
    plot.title = plot_name;
    plot.plot = p;
    plot.closed = false;  //because it is about to be shown

    Plot_Pool.append(plot);

    N_Plots = static_cast<uint32_t>(Plot_Pool.count());

    p->resize(640,480);
    p->setWindowTitle(plot_name);
    p->show();

    connect(p, &plot_Window::closing, this, &SgnalPlotterManager::plotClose);
    connect(p, SIGNAL (replot(int)), this, SLOT (replot(int)));
    connect(p, SIGNAL (gridNSamplesChanged(int)), this, SLOT (gridNSampChanged(int)));

    //add a new thumbnail
    QListWidgetItem* item;
    item = new QListWidgetItem(QIcon(QPixmap::fromImage(p->get_Plot_Pointer()->getPlotFigure().scaled(150, 150))), plot_name);
    thumbnailPlots->addItem(item);
    thumbnailPlots->item(thumbnailPlots->count()-1)->setTextColor(QColor("Black"));
    thumbnailPlots->item(thumbnailPlots->count()-1)->setBackgroundColor(QColor("Green"));

    thumbPlotAss.insert(item, index);
    thumbPlotAssInv.insert(index, item);

    return index;  //this is also the index associated to the plot
}

void SgnalPlotterManager::Remove_Plot(uint32_t index)
{
    int idx, type;

    idx = find_plot_by_index(index, &type);
    if (idx == -1)
        return;

    disconnect(Plot_Pool[idx].plot, &plot_Window::closing, this, &SgnalPlotterManager::plotClose);
    disconnect(Plot_Pool[idx].plot, SIGNAL (replot(int)), this, SLOT (replot(int)));

    Plot_Pool[idx].plot->close();
    delete Plot_Pool[idx].plot;

    Plot_Pool.removeAt(idx);
    N_Plots = static_cast<uint32_t>(Plot_Pool.count());

    thumbPlotAss.remove(thumbPlotAssInv.value(index));
    thumbPlotAssInv.remove(index);
}

uint32_t SgnalPlotterManager::Add_XY_Plot(QString plot_name, double frequency)
{
    uint32_t index;
    xy_plot_Window *p;

    index = get_new_plot_index();
    XY_Plot_Structure plot;
    p = new xy_plot_Window(plot_name, static_cast<int>(index), frequency, preferences, fontMgr);

    plot.index = index;
    plot.title = plot_name;
    plot.plot = p;
    plot.closed = false;  //because it is about to be shown

    XY_Plot_Pool.append(plot);

    N_XY_Plots = static_cast<uint32_t>(XY_Plot_Pool.count());

    p->resize(640,480);
    p->setWindowTitle(plot_name);
    p->show();

    connect(p, &xy_plot_Window::closing, this, &SgnalPlotterManager::xy_plotClose);
    connect(p, SIGNAL (replot(int)), this, SLOT (replot(int)));
    connect(p, SIGNAL (gridNSamplesChanged(int)), this, SLOT (gridNSampChanged(int)));

    //add a new thumbnail
    QListWidgetItem* item;
    item = new QListWidgetItem(QIcon(QPixmap::fromImage(p->get_Plot_Pointer()->getPlotFigure().scaled(150, 150))), plot_name);
    thumbnailPlots->addItem(item);
    thumbnailPlots->item(thumbnailPlots->count()-1)->setTextColor(QColor("Black"));
    thumbnailPlots->item(thumbnailPlots->count()-1)->setBackgroundColor(QColor("Green"));

    thumbPlotAss.insert(item, index);
    thumbPlotAssInv.insert(index, item);

    return index;  //this is also the index associated to the plot
}

void SgnalPlotterManager::Remove_XY_Plot(uint32_t index)
{
    int idx, type;

    idx = find_plot_by_index(index, &type);
    if (idx == -1)
        return;

    disconnect(XY_Plot_Pool[idx].plot, &xy_plot_Window::closing, this, &SgnalPlotterManager::xy_plotClose);
    disconnect(XY_Plot_Pool[idx].plot, SIGNAL (replot(int)), this, SLOT (replot(int)));

    XY_Plot_Pool[idx].plot->close();
    delete XY_Plot_Pool[idx].plot;

    XY_Plot_Pool.removeAt(idx);
    N_XY_Plots = static_cast<uint32_t>(XY_Plot_Pool.count());

    thumbPlotAss.remove(thumbPlotAssInv.value(index));
    thumbPlotAssInv.remove(index);
}

uint32_t SgnalPlotterManager::get_new_signal_index()
{
    int i;
    uint32_t max = 0;

    for (i = 0; i < Signal_Pool.count(); i++)
        if (Signal_Pool[i]->get_Index() > max)
            max = Signal_Pool[i]->get_Index();

    return (max + 1);
}

int SgnalPlotterManager::find_signal_by_index(uint32_t index)
{
    int i;

    for (i = 0; i < Signal_Pool.count(); i++)
        if (Signal_Pool[i]->get_Index() == index)
            return i;
    return -1;
}

uint32_t SgnalPlotterManager::get_new_plot_index()
{
    int i;
    uint32_t max = 0;

    //we search among all plots (normal and XY plots) and find the maximum index. then, we create a new one
    for (i = 0; i < Plot_Pool.count(); i++)
        if (Plot_Pool[i].index > max)
            max = Plot_Pool[i].index;

    for (i = 0; i < XY_Plot_Pool.count(); i++)
        if (XY_Plot_Pool[i].index > max)
            max = XY_Plot_Pool[i].index;

    return (max + 1);
}

int SgnalPlotterManager::find_plot_by_index(uint32_t index, int *type)
{
    //we look for the plot first in the normal plots and then in the xy_plots
    int i;

    for (i = 0; i < Plot_Pool.count(); i++)
        if (Plot_Pool[i].index == index)
        {
            *type = 0;
            return i;
        }
    //we look for it in the xy_plots
    for (i = 0; i < XY_Plot_Pool.count(); i++)
        if (XY_Plot_Pool[i].index == index)
        {
            *type = 1;
            return i;
        }

    //if we are here then nothing was found
    *type = -1;

    return -1;
}

int SgnalPlotterManager::get_Min(int *buff, int N)
{
    int i, j, swap;

    for (i = 0; i < N - 1; i++)
        for (j = i + 1; j < N; j++)
            if (buff[i] > buff[j])
            {
                swap = buff[j];
                buff[j] = buff[i];
                buff[i] = swap;
            }
    return buff[0];
}

int SgnalPlotterManager::get_Min_Vector(QVector<int> vect)
{
    int i;
    int min;

    min = vect[0];

    for (i = 1; i < vect.count(); i++)
        if (vect[i] < min)
            min = vect[i];

    return min;
}

void SgnalPlotterManager::prepareSigViewModel()
{
    QStringList headers;

    sigViewModel->insertColumns(0, 4);  //inserts 5 columns

    headers.append("Signal");
    headers.append("Value");
    headers.append("Data Type");
    headers.append("Scaling Factor");

    sigViewModel->setHorizontalHeaderLabels(headers);
    sigView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

int SgnalPlotterManager::Prepare_and_Plot_Individual(int i)
{
    int j, min, res;
    float** data; QColor* colors; float* line_width;
    int N_sig; int idx; int* n_p;

    N_sig = Plot_Pool[i].signals_associated.count();
    data = new float*[N_sig]; colors = new QColor[N_sig]; line_width = new float[N_sig];
    n_p = new int[N_sig];

    for (j = 0; j < N_sig; j++)
    {
        idx = find_signal_by_index(Plot_Pool[i].signals_associated[j].signal_ID);
        if (idx != -1)
            data[j] = Signal_Pool[idx]->retrieve_Data_Pointer();
        else
            return -1;
        colors[j] = Plot_Pool[i].signals_associated[j].signal_color;
        line_width[j] = Plot_Pool[i].signals_associated[j].line_width;
        n_p[j] = static_cast<int>(Signal_Pool[idx]->Count_Data());
    }
    if (N_sig > 0)
        min = get_Min(n_p, N_sig);  //finds the minimum number of points available among all signals
    else
        min = 0;

    res = Plot_Pool[i].plot->parallel_prepare_Signal_Data(N_sig, min, data, colors, line_width);

    if (res == 0)  //preparation of data has been successful => order a rewrite of the plot buffer
        Plot_Pool[i].plot->update();

    delete data; delete colors; delete line_width; delete n_p;

    return min;
}

int SgnalPlotterManager::Prepare_and_Plot_XY_Individual(int i)
{
    int j, min, res;
    float** x_data; float** y_data;
    QColor* colors; float* line_width;
    int N_sig; int x_idx, y_idx; int* n_p;

    N_sig = XY_Plot_Pool[i].x_signals_associated.count();
    x_data = new float*[N_sig]; y_data = new float*[N_sig];
    colors = new QColor[N_sig]; line_width = new float[N_sig];
    n_p = new int[2*N_sig];

    for (j = 0; j < N_sig; j++)
    {
        x_idx = find_signal_by_index(XY_Plot_Pool[i].x_signals_associated[j].signal_ID);
        y_idx = find_signal_by_index(XY_Plot_Pool[i].y_signals_associated[j].signal_ID);
        if ((x_idx != -1) && (y_idx != -1))  //signals are found
        {
            x_data[j] = Signal_Pool[x_idx]->retrieve_Data_Pointer();
            y_data[j] = Signal_Pool[y_idx]->retrieve_Data_Pointer();
        }
        else
            return -1;
        colors[j] = XY_Plot_Pool[i].x_signals_associated[j].signal_color;
        line_width[j] = XY_Plot_Pool[i].x_signals_associated[j].line_width;
        n_p[2 * j] = static_cast<int>(Signal_Pool[x_idx]->Count_Data());
        n_p[(2 * j) + 1] = static_cast<int>(Signal_Pool[y_idx]->Count_Data());
    }
    if (N_sig > 0)
        min = get_Min(n_p, 2 * N_sig);
    else
        min = 0;

    res = XY_Plot_Pool[i].plot->prepare_Signal_Data(N_sig, min, x_data, y_data, colors, line_width);

    if (res == 0)  //preparation of data has been successful => order a rewrite of the plot buffer
        XY_Plot_Pool[i].plot->update();

    delete x_data; delete y_data; delete colors; delete line_width; delete n_p;

    return min;
}

QString SgnalPlotterManager::get_type_text(int type)
{
    QString st;

    switch (type)
    {
    case 0:
        st = "8-bit Int";
        break;

    case 1:
        st = "8-bit UInt";
        break;

    case 2:
        st = "16-bit UInt";
        break;

    case 3:
        st = "16-bit Int";
        break;

    case 4:
        st = "32-bit UInt";
        break;

    case 5:
        st = "32-bit Int";
        break;

    case 6:
        st = "Float";
        break;

    default:
        st = "None";
    }

    return st;
}

void SgnalPlotterManager::Associate(uint32_t signal_index, uint32_t plot_index, QColor color, float line_width)
{
    int type;
    int i = find_signal_by_index(signal_index);
    int j = find_plot_by_index(plot_index, &type);
    int k;
    bool found;

    if (type != 0)  //not a signal plot
        return;

    if (j == -1)  //plot not found
        return;

    found = false;

    if ((i != -1) && (j != -1))
    {
        //we check if the association between signal and plot has already been made
        for (k = 0; k < Plot_Pool[j].signals_associated.count(); k++)
            if (Plot_Pool[j].signals_associated[k].signal_ID == signal_index)  //the association has already been made
                found = true;
        if (found == false)  //we can now add the new association
        {
            SignalInfo s_i; s_i.signal_ID = signal_index; s_i.signal_color = color; s_i.line_width = line_width; s_i.visible = true;  //by defaults it is visible
            Plot_Pool[j].signals_associated.append(s_i);
            Plot_Pool[j].plot->addSignal(Signal_Pool[i]->get_Index(), Signal_Pool[i]->get_Name(), color);
        }
    }
}

bool SgnalPlotterManager::DeAssociate(uint32_t signal_index, uint32_t plot_index)
{
    int type;
    int i = find_signal_by_index(signal_index);
    int j = find_plot_by_index(plot_index, &type);
    int k;

    if ((i == -1) || (j == -1))  //either the signal or the plot have not been found
        return false;

    //we check that the association exists and in case we remove it
    for (k = 0; k < Plot_Pool[j].signals_associated.count(); k++)
        if (Plot_Pool[j].signals_associated[k].signal_ID == signal_index)  //the association has already been made
        {
            Plot_Pool[j].plot->removeSignal(signal_index);
            Plot_Pool[j].signals_associated.remove(k);
            //At this point we need to update the plot by redrawing
            Plot_Pool[j].plot->update();
            return true;
        }
    return false;
}

void SgnalPlotterManager::Associate_XY(uint32_t x_signal_index, uint32_t y_signal_index, uint32_t plot_index, QColor color, float line_width)
{
    int type;
    int i = find_signal_by_index(x_signal_index);
    int j = find_signal_by_index(y_signal_index);
    int k = find_plot_by_index(plot_index, &type);
    int t;
    bool found;

    if (type != 1)  //not a xy_plot
        return;

    if (j == -1)  //plot not found
        return;

    found = false;

    if ((i != -1) && (j != -1))
    {
        //we check if the association between signals and plot has already been made
        for (t = 0; t < XY_Plot_Pool[k].x_signals_associated.count(); t++)
            if ((XY_Plot_Pool[k].x_signals_associated[t].signal_ID == x_signal_index) && (XY_Plot_Pool[k].y_signals_associated[t].signal_ID == y_signal_index))
                found = true;

        if (found == false)  //we can make the association
        {
            SignalInfo s_x; s_x.signal_ID = x_signal_index; s_x.signal_color = color; s_x.line_width = line_width; s_x.visible = true;
            SignalInfo s_y; s_y.signal_ID = y_signal_index; s_y.signal_color = color; s_y.line_width = line_width; s_y.visible = true;
            XY_Plot_Pool[k].x_signals_associated.append(s_x);
            XY_Plot_Pool[k].y_signals_associated.append(s_y);

            XY_Plot_Pool[k].plot->addSignal(x_signal_index, y_signal_index, Signal_Pool[i]->get_Name(), Signal_Pool[j]->get_Name(), color);
        }
    }
}

bool SgnalPlotterManager::DeAssociate_XY(uint32_t x_signal_index, uint32_t y_signal_index, uint32_t plot_index)
{
    int type;
    int i = find_signal_by_index(x_signal_index);
    int j = find_signal_by_index(y_signal_index);
    int k = find_plot_by_index(plot_index, &type);
    int t;

    if ((i == -1) || (j == -1))  //either the signal or the plot have not been found
        return false;

    if (k == -1)  //plot not found
        return false;

    if (type != 1)  //not a xy_plot
        return false;

    //we check that the association exists and in case we remove it
    for (t = 0; t < XY_Plot_Pool[k].x_signals_associated.count(); t++)
        if ((XY_Plot_Pool[k].x_signals_associated[t].signal_ID == x_signal_index) && (XY_Plot_Pool[k].y_signals_associated[t].signal_ID == y_signal_index))  //the association has already been made
        {
            XY_Plot_Pool[k].plot->removeSignal(x_signal_index, y_signal_index);

            XY_Plot_Pool[k].x_signals_associated.remove(k);
            XY_Plot_Pool[k].y_signals_associated.remove(k);
            //At this point we need to update the plot by redrawing
            XY_Plot_Pool[k].plot->update();
            return true;
        }
    return false;
}

void SgnalPlotterManager::Enable_Record(uint32_t signal_index, bool record)
{
    int i = find_signal_by_index(signal_index);

    if (i != -1)
        Signal_Pool[i]->set_Record(record);
}

void SgnalPlotterManager::Enable_Record_All(bool record)
{
    int i;

    command_Rec = record;

    for (i = 0; i < static_cast<int>(N_Signals); i++)
        Signal_Pool[i]->set_Record(record);
}

void SgnalPlotterManager::Prepare_and_Plot()
{
    int i;
//    QElapsedTimer timer;

//    timer.start();

    //We start the FFT calculations if the fftManager is free
    if (fftMgr->getStatus() == false)  //the manager is free
        fftMgr->startFFTCalculation();  //now we don't care, the fftManager will update the FFT windows when calculation is done, not our concern here

    for (i = 0; i < static_cast<int>(Plot_Pool.count()); i++)
    {
        Prepare_and_Plot_Individual(i);
    }

    for (i = 0; i < static_cast<int>(XY_Plot_Pool.count()); i++)
    {
        Prepare_and_Plot_XY_Individual(i);
    }

    //We now update the value of each signal in the sigView
    for (i = 0; i < static_cast<int>(N_Signals); i++)
    {
        sigViewModel->setItem(i, 1, new QStandardItem(QString::number(Signal_Pool[i]->getLastSample())));
    }

 //   qDebug() << "Preparation time = " << (timer.nsecsElapsed() / 1000000) << " ms";

}

void SgnalPlotterManager::set_Plots_N_Points(int N)
{
    int i;

    for (i = 0; i < Plot_Pool.size(); i++)
        Plot_Pool[i].plot->setNPoints(N);
}

int SgnalPlotterManager::exportToFile(QString filename)
{
    int i, res;

    if (filename.isEmpty() == true)
        return -1;

    MatlabFileSaver saver;
    float** data;
    int N_sig, counter, N_samples;
    QVector<int> samples;

    N_sig = 0;
    for (i = 0; i < static_cast<int>(N_Signals); i++)
        if (sigViewModel->item(i, 0)->checkState() == Qt::Checked)
        {
            qDebug() << Signal_Pool[i]->get_Name();
            saver.AddSignalInfoToWrite(Signal_Pool[i]->get_Name());
            N_sig++;
        }

    if (N_sig == 0)
        return -1;  //no signals to be saved

    //prepares pointers to data to be saved
    data = new float*[N_sig];
    samples.resize(N_sig);
    counter = 0;
    for (i = 0; i < static_cast<int>(N_Signals); i++)
        if (sigViewModel->item(i, 0)->checkState() == Qt::Checked)
        {
            data[counter] = Signal_Pool[i]->retrieve_Data_Pointer();
            samples[counter] = static_cast<int>(Signal_Pool[i]->Count_Data());
            counter++;
        }

    N_samples = get_Min_Vector(samples);

    res = saver.Save_MATLAB_File(data, N_samples, filename);

    return res;
}

int SgnalPlotterManager::autoExportToFile()
{
    int i, k, res;
    uint8_t out;
    QString filename;

    res = -1;

    if (command_Pool.length() < 1)
        return -1;

    MatlabFileSaver *saver;
    float** data;
    int N_sig, counter, N_samples;

    N_sig = 0;
    saver = new MatlabFileSaver();

    for (i = 0; i < static_cast<int>(N_Signals); i++)
        if (sigViewModel->item(i, 0)->checkState() == Qt::Checked)
        {
            saver->AddSignalInfoToWrite(Signal_Pool[i]->get_Name());
            N_sig++;
        }

    if (N_sig == 0)
    {
        delete saver;
        return -1;  //no signals to be saved
    }

    //prepares pointers to data to be saved

    int start_idx; int end_idx; int state;
    bool command_save;

    //state = 0 => we are waiting for RECORD_ACT
    //state = 1 => we are waiting for NO_ACT


    state = command_Pool[0];
    if (state == RECORD_CMD)
        start_idx = 0;
    command_save = false;

    for (k = 1; k < command_Pool.length(); k++)
    {
        //two cases: we are in record or we are in no_cmd
        switch(state)
        {
        case NO_CMD:
            if (command_Pool[k] == RECORD_CMD)
            {
                start_idx = k;
                state = RECORD_CMD;
            }
            break;

        case RECORD_CMD:
            if (command_Pool[k] == NO_CMD)
            {
                end_idx = k;
                command_save = true;
                state = NO_CMD;
            }
            break;
        }

        if (command_save == true)
        {
            //first we generate a new valid filename
            filename = fileGen->generateAutoFileName(&out);

            if (out != 0)
            {
                delete saver;
                return -1;
            }

            N_samples = end_idx - start_idx;

            data = new float*[N_sig];
            counter = 0;
            for (i = 0; i < static_cast<int>(N_Signals); i++)
                if (sigViewModel->item(i, 0)->checkState() == Qt::Checked)
                {
                    data[counter] = &(Signal_Pool[i]->retrieve_Data_Pointer()[start_idx]);
                    counter++;
                }

            res = saver->Save_MATLAB_File(data, N_samples, filename);

//            delete data;
            command_save = false;
        }
    }
    delete saver;

    return res;
}

void SgnalPlotterManager::Organize_Windows()
{
    int N_plots;
    int n_columns, n_rows, n_surfaces;
    int scr_w, scr_h;
    int x_off, y_off;
    int fav_w;  //favourite window width
    int fav_h;  //favourite window height
    int i, j, k, t;

    N_plots = Plot_Pool.count() + fftMgr->getNWindows() + XY_Plot_Pool.count();

    bool orderingSignals;

    if (N_plots == 0)
        return;

    fav_w = Plot_Pool[0].plot->size().width();
    fav_h = Plot_Pool[0].plot->size().height();
    scr_w = this->window()->windowHandle()->screen()->size().width();
    scr_h = this->window()->windowHandle()->screen()->size().height();
    x_off = this->window()->windowHandle()->screen()->availableGeometry().x();
    y_off = this->window()->windowHandle()->screen()->availableGeometry().y();

    //calculates how many rows are necessary based on favourite dimensions
    n_columns = scr_w / fav_w;
    n_rows = scr_h / fav_h;
    n_surfaces = N_plots / (n_columns * n_rows);

    //we start by ordering the standart plot signals
    orderingSignals = true;
    t = 0;
    for (k = 0; k <= n_surfaces; k++)
        for (j = 0; j < n_rows; j++)
            for (i = 0; i < n_columns; i++)
            {
                if (orderingSignals == true)
                {
                    if (t < Plot_Pool.count())
                    {
                        Plot_Pool[t].plot->resize(fav_w, fav_h);
                        Plot_Pool[t].plot->move(x_off + (i * fav_w), y_off + (j * fav_h));
                        t++;
                    }
                    else //we pass now to order the FFT windows
                    {
                        if (t < (Plot_Pool.count() + XY_Plot_Pool.count()))
                        {
                            XY_Plot_Pool[t - Plot_Pool.count()].plot->resize(fav_w, fav_h);
                            XY_Plot_Pool[t - Plot_Pool.count()].plot->move(x_off + (i * fav_w), y_off + (j * fav_h));
                            t++;
                        }
                        else
                        {
                            orderingSignals = false;
                            t = 0;
                        }
                    }
                }
                if (orderingSignals  == false)
                {
                    if (t < fftMgr->getNWindows())
                    {
                        fftMgr->getWindowHandler(t)->resize(fav_w, fav_h);
                        fftMgr->getWindowHandler(t)->move(x_off + (i * fav_w), y_off + (j * fav_h));
                        t++;
                    }
                }
            }
}

qint64 SgnalPlotterManager::getSignalMemoryData()
{
    int i;
    qint64 bytes;
    unsigned int floatsize;

    floatsize = sizeof(float);
    bytes = 0;

    for (i = 0; i < Signal_Pool.count(); i++)
        bytes += (Signal_Pool[i]->Count_Data() * floatsize);

    return bytes;
}

void SgnalPlotterManager::clearAllData()
{
    int i;

    for (i = 0; i < Signal_Pool.count(); i++)
        Signal_Pool[i]->Clean_Data();
}

void SgnalPlotterManager::updateFonts()
{
    int i;

    for (i = 0; i < Plot_Pool.count(); i++)
    {
        Plot_Pool[i].plot->updateFonts();
        Plot_Pool[i].plot->update();
    }
    for (i = 0; i < XY_Plot_Pool.count(); i++)
    {
        XY_Plot_Pool[i].plot->updateFonts();
        XY_Plot_Pool[i].plot->update();
    }
}

bool SgnalPlotterManager::checkGrids()
{
    bool grid_stat;
    int n, m;
    int i;

    fftMgr->checkGrids(&n, &m);

    for (i = 0; i < Plot_Pool.count(); i++)
    {
        if (Plot_Pool[i].plot->isGridEnabled() == true)
        {
            n += 1;
        }
        else
        {
            m += 1;
        }
    }
    for (i = 0; i < XY_Plot_Pool.count(); i++)
    {
        if (XY_Plot_Pool[i].plot->isGridEnabled() == true)
        {
            n += 1;
        }
        else
        {
            m += 1;
        }
    }

    if (n >= m)
        grid_stat = true;
    else
        grid_stat = false;

    return grid_stat;
}

void SgnalPlotterManager::setAllGrid(bool en)
{
    int i;

    for (i = 0; i < Plot_Pool.count(); i++)
        Plot_Pool[i].plot->setGrid(en);
    for (i = 0; i < XY_Plot_Pool.count(); i++)
        XY_Plot_Pool[i].plot->setGrid(en);

    fftMgr->setAllGrids(en);
}

void SgnalPlotterManager::setPushButtonsEnable(bool en)
{
    addPlotPB->setEnabled(en);
    remPlotPB->setEnabled(en);
    sigSettPB->setEnabled(en);
    associatePB->setEnabled(en);
    deassociatePB->setEnabled(en);
    titleChangePB->setEnabled(en);
    associateXYPB->setEnabled(en);
    deassociateXYPB->setEnabled(en);
}

void SgnalPlotterManager::replot(int index)
{
    int type;

    int pos = find_plot_by_index(index, &type);
    if ((type == 0) && (pos != -1))
        Prepare_and_Plot_Individual(pos);
    if ((type == 1) && (pos != -1))
        Prepare_and_Plot_XY_Individual(pos);

    emit replotCalled();
}

void SgnalPlotterManager::updateThumbs()
{
    int i;

    for (i = 0; i < Plot_Pool.count(); i++)
        thumbPlotAssInv.value(Plot_Pool[i].index)->setIcon(QIcon(QPixmap::fromImage(Plot_Pool[i].plot->get_Plot_Pointer()->getPlotFigure().scaled(150, 150))));
    for (i = 0; i < XY_Plot_Pool.count(); i++)
        thumbPlotAssInv.value(XY_Plot_Pool[i].index)->setIcon(QIcon(QPixmap::fromImage(XY_Plot_Pool[i].plot->get_Plot_Pointer()->getPlotFigure().scaled(150, 150))));
}

void SgnalPlotterManager::plotClose(int index)
{
    int idx, type;

    idx = find_plot_by_index(static_cast<uint32_t>(index), &type);  //it's the same index of the thumbnails
    if (idx == -1)
        return;
    if (type == 0)
    {
        Plot_Pool[idx].closed = true;
        thumbPlotAssInv.value(static_cast<uint32_t>(index))->setTextColor(QColor("Black"));
        thumbPlotAssInv.value(static_cast<uint32_t>(index))->setBackgroundColor(QColor("Red"));
    }
}

void SgnalPlotterManager::xy_plotClose(int index)
{
    int idx, type;

    idx = find_plot_by_index(static_cast<uint32_t>(index), &type);
    if (idx == -1)
        return;
    if (type == 1)
    {
        XY_Plot_Pool[idx].closed = true;
        thumbPlotAssInv.value(static_cast<uint32_t>(index))->setTextColor(QColor("Black"));
        thumbPlotAssInv.value(static_cast<uint32_t>(index))->setBackgroundColor(QColor("Red"));
    }
}

void SgnalPlotterManager::thumbDoubleClick(QListWidgetItem *item)
{
    int type, pos;
    uint32_t index;

    index = thumbPlotAss.value(item);  //gets the plot index
    pos = find_plot_by_index(index, &type);
    if (pos == -1)
        return;
    if (type == 0)
    {
        if (Plot_Pool[pos].closed == true)
        {
            Plot_Pool[pos].closed = false;
            Plot_Pool[pos].plot->show();
            item->setBackgroundColor(QColor("Green"));
            item->setTextColor(QColor("Black"));
        }
        else
        {
            Plot_Pool[pos].plot->raise();
        }
    }
    if (type == 1)
    {
        if (XY_Plot_Pool[pos].closed == true)
        {
            XY_Plot_Pool[pos].closed = false;
            XY_Plot_Pool[pos].plot->show();
            item->setBackgroundColor(QColor("Green"));
            item->setTextColor(QColor("Black"));
        }
        else
        {
            XY_Plot_Pool[pos].plot->raise();
        }
    }
}

void SgnalPlotterManager::addPlotTriggered()
{
    bool ok;
    QString title = QInputDialog::getText(this, "Insert new plot title", "Plot title:", QLineEdit::Normal, "", &ok);

    if ((ok == false) || (title.isEmpty()))
        return;

    //Now we ask what kind of plot to add: Signal Plot or X-Y Plot
    QStringList items;
    items << QString("Signal Plot");
    items << QString("X-Y Plot");

    QString selected = QInputDialog::getItem(this, "Plot Window", "Choose the kind of plot to add", items, 0, false, &ok);

    if (ok == false)
        return;

    if (QString::compare(selected, "Signal Plot", Qt::CaseInsensitive) == 0)
        Add_Plot(title, plot_frequency);
    if (QString::compare(selected, "X-Y Plot", Qt::CaseInsensitive) == 0)
        Add_XY_Plot(title, plot_frequency);
}

void SgnalPlotterManager::remPlotTriggered()
{
    QList<QListWidgetItem*> list;
    int pos, type;

    list = thumbnailPlots->selectedItems();

    for (int i = 0; i < list.count(); i++)
    {
        pos = find_plot_by_index(thumbPlotAss.value(list[i]), &type);
        if (pos == -1) //not found
            return;
        if (type == 0)
        {
            Remove_Plot(thumbPlotAss.value(list[i]));
        }
        if (type == 1)
        {
            Remove_XY_Plot(thumbPlotAss.value(list[i]));
        }
        thumbnailPlots->removeItemWidget(list[i]);
        thumbPlotAss.remove(list[i]);
        delete list[i];
    }
}

void SgnalPlotterManager::sigSetTriggered()
{
    uint32_t index;
    int i, j;
    int sig_pos;

    QItemSelectionModel *select = sigView->selectionModel();

    if (select->selectedRows().isEmpty() == true)
        return;

    index = viewSigAss.value(sigViewModel->item(select->selectedRows()[0].row()));

    sigAssDlg *sD = new sigAssDlg(index, Plot_Pool);

    this->setWindowModality(Qt::WindowModal);
    sD->setModal(true);
    sD->exec();
    if (sD->getAcceptFlag() == true)
    {
        Plot_Pool = sD->getPlotStruct();  //plot_pool updated
        //we need now to change all settings in the plots
        for (i = 0; i < Plot_Pool.count(); i++)
        {
            Plot_Pool[i].plot->clearAllSignal();
            for (j = 0; j < Plot_Pool[i].signals_associated.count(); j++)
            {
                sig_pos = find_signal_by_index(Plot_Pool[i].signals_associated[j].signal_ID);
                Plot_Pool[i].plot->addSignal(Plot_Pool[i].signals_associated[j].signal_ID, Signal_Pool[sig_pos]->get_Name(), Plot_Pool[i].signals_associated[j].signal_color);
            }
        }
    }

    delete sD;
}

void SgnalPlotterManager::associateTriggered()
{
    QMessageBox msgBox;
    uint32_t index_sig, index_plot;
    int plot_pos;
    int type;

    QItemSelectionModel *sig_select = sigView->selectionModel();

    if (sig_select->selectedRows().isEmpty() == true)
    {
        msgBox.setText("No signal has been selected!");
        msgBox.exec();
        return;
    }
    index_sig = viewSigAss.value(sigViewModel->item(sig_select->selectedRows()[0].row()));

    if (thumbnailPlots->selectedItems().isEmpty() == true)
    {
        msgBox.setText("No plot has been selected!");
        msgBox.exec();
        return;
    }

    index_plot = thumbPlotAss.value(thumbnailPlots->selectedItems().first());
    plot_pos = find_plot_by_index(index_plot, &type);

    if (type != 0)  //in this case it's not a signal plot, we can return
        return;

    //We ask for confirmation
    msgBox.setText("Do you want to associate " + Signal_Pool[find_signal_by_index(index_sig)]->get_Name() + " with plot " + Plot_Pool[plot_pos].title + "?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Cancel)
        return;

    //We ask for the line width
    bool ok;

    int lw = QInputDialog::getInt(this, "Insert line width", "Line width (1 - 100 pixels): ", 4, 1, 100, 1, &ok);

    //We ask for the color
    QColor color_ret;

    color_ret = QColorDialog::getColor(QColor("Black"), this, "Color for associating the signal to the plot");

    //We create the new association
    Associate(index_sig, index_plot, color_ret, static_cast<float>(lw));
}

void SgnalPlotterManager::deassociateTriggered()
{
    QMessageBox msgBox;
    uint32_t index_sig, index_plot;
    int plot_pos, type;

    QItemSelectionModel *sig_select = sigView->selectionModel();

    if (sig_select->selectedRows().isEmpty() == true)
    {
        msgBox.setText("No signal has been selected!");
        msgBox.exec();
        return;
    }
    index_sig = viewSigAss.value(sigViewModel->item(sig_select->selectedRows()[0].row()));

    if (thumbnailPlots->selectedItems().isEmpty() == true)
    {
        msgBox.setText("No plot has been selected!");
        msgBox.exec();
        return;
    }
    index_plot = thumbPlotAss.value(thumbnailPlots->selectedItems().first());
    plot_pos = find_plot_by_index(index_plot, &type);

    if (type != 0) //it's not a signal plot
        return;

    //We check if the association exists
    int i;  bool found; found = false;
    for (i = 0; i < Plot_Pool[plot_pos].signals_associated.count(); i++)
        if (Plot_Pool[plot_pos].signals_associated[i].signal_ID == index_sig)
            found = true;

    if (found == false)
    {
        msgBox.setText("The selected signal is not associated to the chosen plot!");
        msgBox.exec();
        return;
    }

    //We ask for confirmation
    msgBox.setText("Do you want to remove the association of  " + Signal_Pool[find_signal_by_index(index_sig)]->get_Name() + " to plot " + Plot_Pool[static_cast<int>(plot_pos)].title + "?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Cancel)
        return;

    //We delete the association
    if(DeAssociate(index_sig, index_plot) == false)
    {
        msgBox.setText("Error during eliminating the association!");
        msgBox.exec();
    }

    Prepare_and_Plot();
}

void SgnalPlotterManager::associateXYTriggered()
{
    QMessageBox msgBox;
    uint32_t x_index_sig, y_index_sig, index_plot;
    int i, plot_pos, type;
    bool ok, found;

    //first we check that the selected plot is a XY_Plot
    if (thumbnailPlots->selectedItems().isEmpty() == true)
    {
        msgBox.setText("No plot has been selected!");
        msgBox.exec();
        return;
    }

    index_plot = thumbPlotAss.value(thumbnailPlots->selectedItems().first());
    plot_pos = find_plot_by_index(index_plot, &type);

    if (type != 1)  //in this case it's not a xy plot
    {
        msgBox.setText("The selected plot is not a X-Y plot");
        msgBox.exec();
        return;
    }

    //Now we ask for what is going to be the x_signal
    QStringList items;
    for (i = 0; i < Signal_Pool.count(); i++)
        items << Signal_Pool[i]->get_Name();

    if (items.count() == 0)
        return;

    QString selected = QInputDialog::getItem(this, "Select a signal", "Choose the signal to associate to the X-Axis", items, 0, false, &ok);
    if (ok == false)
        return;
    found = false;
    i = 0;
    while((found == false) && (i < Signal_Pool.count()))
    {
        if (QString::compare(selected, Signal_Pool[i]->get_Name()) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == false)
    {
        msgBox.setText("No signal has been selected!");
        msgBox.exec();
        return;
    }
    x_index_sig = static_cast<int>(Signal_Pool[i]->get_Index());

    //Now we ask for what is going to be the y_signal
    items.clear();
    for (i = 0; i < Signal_Pool.count(); i++)
        items << Signal_Pool[i]->get_Name();

    if (items.count() == 0)
        return;

    selected = QInputDialog::getItem(this, "Select a signal", "Choose the signal to associate to the Y-Axis", items, 0, false, &ok);
    if (ok == false)
        return;
    found = false;
    i = 0;
    while((found == false) && (i < Signal_Pool.count()))
    {
        if (QString::compare(selected, Signal_Pool[i]->get_Name()) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == false)
    {
        msgBox.setText("No signal has been selected!");
        msgBox.exec();
        return;
    }
    y_index_sig = static_cast<int>(Signal_Pool[i]->get_Index());


    //We ask for confirmation
    QString text;
    text = "Do you want to associate " + Signal_Pool[find_signal_by_index(x_index_sig)]->get_Name();
    text += " and " + Signal_Pool[find_signal_by_index(y_index_sig)]->get_Name() + " with plot ";
    text += XY_Plot_Pool[plot_pos].title + "?";
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Cancel)
        return;

    //We ask for the line width
    int lw = QInputDialog::getInt(this, "Insert line width", "Line width (1 - 100 pixels): ", 4, 1, 100, 1, &ok);

    //We ask for the color
    QColor color_ret;

    color_ret = QColorDialog::getColor(QColor("Black"), this, "Color for associating the signal to the plot");

    //We create the new association
    Associate_XY(x_index_sig, y_index_sig, index_plot, color_ret, static_cast<float>(lw));
}

void SgnalPlotterManager::deassociateXYTriggered()
{
    QMessageBox msgBox;
    uint32_t x_index_sig, y_index_sig, index_plot;
    int i, plot_pos, type, x_pos, y_pos;
    bool ok, found;

    //first we check that the selected plot is a XY_Plot
    if (thumbnailPlots->selectedItems().isEmpty() == true)
    {
        msgBox.setText("No plot has been selected!");
        msgBox.exec();
        return;
    }

    index_plot = thumbPlotAss.value(thumbnailPlots->selectedItems().first());
    plot_pos = find_plot_by_index(index_plot, &type);

    if (type != 1)  //in this case it's not a xy plot
    {
        msgBox.setText("The selected plot is not a X-Y plot");
        msgBox.exec();
        return;
    }

    //Now we prepare a vector of associations
    QString text;
    QVector<QString> assoc;
    QStringList items;

    if (XY_Plot_Pool[plot_pos].x_signals_associated.count() == 0)
    {
        msgBox.setText("No signals associated to this plot!");
        msgBox.exec();
        return;
    }

    assoc.resize(XY_Plot_Pool[plot_pos].x_signals_associated.count());

    for (i = 0; i < XY_Plot_Pool[plot_pos].x_signals_associated.count(); i++)
    {
        x_pos = find_signal_by_index(XY_Plot_Pool[plot_pos].x_signals_associated[i].signal_ID);
        y_pos = find_signal_by_index(XY_Plot_Pool[plot_pos].y_signals_associated[i].signal_ID);
        if ((x_pos == -1) || (y_pos == -1))
        {
            qDebug() << "Error in the signal list!";
            msgBox.setText("Error in the signal list!");
            msgBox.exec();
            return;
        }
        text = "X: " + Signal_Pool[x_pos]->get_Name();
        text += "; Y: " + Signal_Pool[y_pos]->get_Name();
        assoc[i] = text;
        items << text;
    }

    QString selected = QInputDialog::getItem(this, "Select a signal couple", "Choose the signal couple to remove from the selected plot:", items, 0, false, &ok);

    if (ok == false)
        return;

    found = false;
    i = 0;
    while((found == false) && (i < assoc.count()))
    {
        if (QString::compare(selected, assoc[i]) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == false)
    {
        msgBox.setText("No signal couple has been selected!");
        msgBox.exec();
        return;
    }

    x_index_sig = XY_Plot_Pool[plot_pos].x_signals_associated[i].signal_ID;
    y_index_sig = XY_Plot_Pool[plot_pos].y_signals_associated[i].signal_ID;

    //We ask for confirmation
    msgBox.setText("Do you want to remove the association of  " + Signal_Pool[find_signal_by_index(x_index_sig)]->get_Name() + " and " + Signal_Pool[find_signal_by_index(y_index_sig)]->get_Name() + " to plot " + XY_Plot_Pool[plot_pos].title + "?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Cancel)
        return;

    //We delete the association
    if(DeAssociate_XY(x_index_sig, y_index_sig, index_plot) == false)
    {
        msgBox.setText("Error during eliminating the association!");
        msgBox.exec();
    }

    Prepare_and_Plot();
}

void SgnalPlotterManager::changeTitleTriggered()
{
    int type;
    bool ok;

    uint32_t plot_index = thumbPlotAss.value(thumbnailPlots->selectedItems().first());
    int pos = find_plot_by_index(plot_index, &type);

    if (pos == -1)
        return;

    if (type == 0)
    {
        QString title = QInputDialog::getText(this, "Modify plot title", "Plot title:", QLineEdit::Normal, Plot_Pool[pos].title, &ok);
        if (ok == true)
        {
            Plot_Pool[pos].title = title;
            thumbnailPlots->selectedItems().first()->setText(title);
            Plot_Pool[pos].plot->setTitle(title);
        }
    }
    else
    {
        QString title = QInputDialog::getText(this, "Modify plot title", "Plot title:", QLineEdit::Normal, XY_Plot_Pool[pos].title, &ok);
        if (ok == true)
        {
            XY_Plot_Pool[pos].title = title;
            thumbnailPlots->selectedItems().first()->setText(title);
            XY_Plot_Pool[pos].plot->setTitle(title);
        }
    }
}

void SgnalPlotterManager::gridFFTChanged()
{
    emit replotCalled();
}

void SgnalPlotterManager::gridNSampChanged(int npoints)
{
    emit gridChanged(npoints);
}

void SgnalPlotterManager::newFFTWindow()
{
    //waits for the fftmanager to be free
    while (fftMgr->getStatus() == true);

    //We add a new FFT window
    bool ok, found;
    QString title = QInputDialog::getText(this, "Insert new FFT title", "Plot title:", QLineEdit::Normal, "FFT Window " + QString::number(fftWdwList.count() + 1), &ok);

    if ((ok == false) || (title.isEmpty()))
        return;

    //We check that the title has not been assigned yet
    found = false;
    for (int i = 0; i < fftWdwList.count(); i++)
        if (QString::compare(title, fftMgr->getWindowName(fftWdwList[i], &ok)) == 0)
            found = true;

    if (found == true)
    {
        QMessageBox msgBox;
        msgBox.setText("This title has already been assigned");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    int n_samples = QInputDialog::getInt(this, "Insert the number of samples (multiple of 2):", "FFT dimension:", QLineEdit::Normal, 32, std::numeric_limits<int>::max(), 1, &ok);

    if ((n_samples == 0) || (ok == false))
        return;

    if ((n_samples % 2) != 0)  //not a multiple of 2
    {
        QMessageBox msgBox;
        msgBox.setText("The number of samples is not a multiple of 2");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    QStringList items;
    items << QString("Amplitude");
    items << QString("Power");
    items << QString("Power [dB]");

    int mode;
    QString selected = QInputDialog::getItem(this, "FFT calculation mode", "Choose the kind of FFT to be performed", items, 0, false, &ok);

    if (ok == false)
        return;

    mode = 0;
    if (QString::compare(selected, "Amplitude", Qt::CaseInsensitive) == 0)
        mode = 0;
    if (QString::compare(selected, "Power", Qt::CaseInsensitive) == 0)
        mode = 1;
    if (QString::compare(selected, "Power [dB]", Qt::CaseInsensitive) == 0)
        mode = 2;

    fftWdwList.append(fftMgr->createFFTWindow(title, n_samples, mode));
}

void SgnalPlotterManager::remFFTWindow()
{
    int i;
    QStringList items;
    bool ok;
    QString ret;
    bool found;

    for (i = 0; i < fftWdwList.count(); i++)
    {
        ret = fftMgr->getWindowName(fftWdwList[i], &ok);
        if (ok == true)
            items << ret;
    }

    if (items.count() == 0)
        return;

    QString selected = QInputDialog::getItem(this, "FFT windows list", "Choose the FFT window to be removed", items, 0, false, &ok);
    found = false;
    i = 0;
    while((found == false) && (i < fftWdwList.count()))
    {
        if (QString::compare(selected, fftMgr->getWindowName(fftWdwList[i], &ok)) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == true)
    {
        //before removing we wait for the fftManager to finish opened FFT calculation
        while(fftMgr->getStatus() == true);  //do nothing until the FFT calculator has finished
        fftMgr->removeFFTWindow(fftWdwList[i]);
    }
}

void SgnalPlotterManager::assSigFFT()
{
    int i;
    QStringList items;
    bool ok;
    QString ret;
    bool found;
    QString selected;
    QMessageBox msgBox;
    int index_sig, index_fftWdw;

    //waits for the fftmanager to be free
    while (fftMgr->getStatus() == true);

    for (i = 0; i < Signal_Pool.count(); i++)
        items << Signal_Pool[i]->get_Name();

    if (items.count() == 0)
        return;
    selected = QInputDialog::getItem(this, "Signals list", "Choose the signal for FFT calculation", items, 0, false, &ok);
    found = false;
    i = 0;
    while((found == false) && (i < Signal_Pool.count()))
    {
        if (QString::compare(selected, Signal_Pool[i]->get_Name()) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == false)
    {
        msgBox.setText("No signal has been selected!");
        msgBox.exec();
        return;
    }
    index_sig = static_cast<int>(Signal_Pool[i]->get_Index());

    items.clear();
    for (i = 0; i < fftWdwList.count(); i++)
    {
        ret = fftMgr->getWindowName(fftWdwList[i], &ok);
        if (ok == true)
            items << ret;
    }

    if (items.count() == 0)
        return;

    selected = QInputDialog::getItem(this, "FFT windows list", "Choose the FFT window", items, 0, false, &ok);
    found = false;
    i = 0;
    while((found == false) && (i < fftWdwList.count()))
    {
        if (QString::compare(selected, fftMgr->getWindowName(fftWdwList[i], &ok)) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == false)
    {
        msgBox.setText("No valid FFT window has been selected!");
        msgBox.exec();
        return;
    }
    index_fftWdw = fftWdwList[i];

    //We ask for the line width
    int lw = QInputDialog::getInt(this, "Insert line width", "Line width (1 - 100 pixels): ", 4, 1, 100, 1, &ok);

    //We ask for the color
    QColor color_ret;

    color_ret = QColorDialog::getColor(QColor("Black"), this, "Color for associating the signal to the plot");

    //we make the association
    fftMgr->addSigFFT(index_fftWdw, index_sig, Signal_Pool[find_signal_by_index(index_sig)]->get_Name(), lw, color_ret);
}

void SgnalPlotterManager::remSigFFT()
{
    int i;
    QStringList items;
    bool ok;
    QString ret;
    bool found;
    QString selected;
    QMessageBox msgBox;
    int index_sig, index_fftWdw;

    //waits for the fftmanager to be free
    while (fftMgr->getStatus() == true);

    for (i = 0; i < Signal_Pool.count(); i++)
        items << Signal_Pool[i]->get_Name();

    if (items.count() == 0)
        return;
    selected = QInputDialog::getItem(this, "Signals list", "Choose the signal to remove from FFT calculation", items, 0, false, &ok);
    found = false;
    i = 0;
    while((found == false) && (i < Signal_Pool.count()))
    {
        if (QString::compare(selected, Signal_Pool[i]->get_Name()) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == false)
    {
        msgBox.setText("No signal has been selected!");
        msgBox.exec();
        return;
    }
    index_sig = static_cast<int>(Signal_Pool[i]->get_Index());

    items.clear();
    for (i = 0; i < fftWdwList.count(); i++)
    {
        ret = fftMgr->getWindowName(fftWdwList[i], &ok);
        if (ok == true)
            items << ret;
    }

    if (items.count() == 0)
        return;

    selected = QInputDialog::getItem(this, "FFT windows list", "Choose the FFT window", items, 0, false, &ok);
    found = false;
    i = 0;
    while((found == false) && (i < fftWdwList.count()))
    {
        if (QString::compare(selected, fftMgr->getWindowName(fftWdwList[i], &ok)) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == false)
    {
        msgBox.setText("No valid FFT window has been selected!");
        msgBox.exec();
        return;
    }
    index_fftWdw = fftWdwList[i];

    //we check now if that signals is associated to that
    int r = fftMgr->remSigFFT(index_fftWdw, index_sig);
    if (r != 0)
    {
        msgBox.setText("This signal is not associated to this FFT window!");
        msgBox.exec();
    }
}

void SgnalPlotterManager::exportFFT()
{
    int i;
    QStringList items;
    bool ok;
    QString ret;
    bool found;
    QString selected;
    QMessageBox msgBox;
    int index_fftWdw;

    //waits for the fftmanager to be free
    while (fftMgr->getStatus() == true);

    //we ask for which window the datas will be saved
    items.clear();
    for (i = 0; i < fftWdwList.count(); i++)
    {
        ret = fftMgr->getWindowName(fftWdwList[i], &ok);
        if (ok == true)
            items << ret;
    }

    if (items.count() == 0)
        return;

    selected = QInputDialog::getItem(this, "FFT windows list", "Choose the FFT window", items, 0, false, &ok);
    found = false;
    i = 0;
    while((found == false) && (i < fftWdwList.count()))
    {
        if (QString::compare(selected, fftMgr->getWindowName(fftWdwList[i], &ok)) == 0)
        {
            found = true;
        }
        i++;
    }
    i--;
    if (found == false)
    {
        msgBox.setText("No valid FFT window has been selected!");
        msgBox.exec();
        return;
    }
    index_fftWdw = fftWdwList[i];

    //Then we get the savefile
    QString filename = QFileDialog::getSaveFileName(this, "Export data", "", "Matlab File (*.mat)");

    if (filename.isEmpty() == false)
    {
        int r = fftMgr->exportToFile(filename, index_fftWdw);
        if (r != 0)
        {
            QMessageBox msgBox;
            msgBox.setText("Error during export of data");
            msgBox.exec();
        }
    }
}
