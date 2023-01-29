/**
  *********************************************************************************************************************************************************
  @file     :fftmanager.cpp
  @brief    :Functions for FFT manager class
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

#include "fftmanager.h"

fftManager::fftManager(appPreferencesStruct *pref, fontManager *font)
{
    preferences = pref;
    fontMgr = font;

    status = false;

    connect(&jobWatch, SIGNAL(finished()), this, SLOT(jobHasFinished()));

    //Creates the GUI part
    plotList = new QTableView(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("FFT windows list:"));
    layout->addWidget(plotList);

    plotList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    plotList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    plotListModel = new QStandardItemModel;
    prepareViewModel();

    plotList->setModel(plotListModel);
    connect(plotList, &QTableView::doubleClicked, this, &fftManager::plotListDoubleClicked);

    setLayout(layout);

    setVisible(false);  //not visible when no windows have been created
}

fftManager::~fftManager()
{
    //here we destroy everything that has been allocated
    int i;
    for (i = 0; i < windowPool.count(); i++)
    {
        windowPool[i].fftWnd->close();
        delete windowPool[i].fftWnd;
    }
}

int fftManager::createFFTWindow(QString title, int n_Samples, int type)
{
    int idx;

    if (n_Samples == 0)
        return -1;

    idx = getNewIndex();  //gets a new index
    fftWindow wdw;
    wdw.N_sig = 0;
    wdw.idx = idx;
    wdw.n_Samples = n_Samples;
    wdw.stepFrequency = frequency / n_Samples;
    wdw.title = title;
    wdw.fftType = type;
    wdw.nullVector.resize(n_Samples);
    wdw.nullVector.fill(0.0f);

    wdw.fftWnd = new fft_plot_Window(title, idx, wdw.stepFrequency, (n_Samples >> 1) + 1, preferences, fontMgr);
    wdw.fftWnd->resize(640,480);
    wdw.fftWnd->setWindowTitle(title);
    wdw.fftWnd->setStepFrequency(wdw.stepFrequency);
    wdw.fftWnd->show();

    connect(wdw.fftWnd, &fft_plot_Window::gridTriggered, this, &fftManager::gridActTriggered);

    //More code is needed to set the plot window to behave as a FFT window => To be added

    windowPool.append(wdw);

    QList<QStandardItem*> newrow;

    QStandardItem *item;

    //we add it to the signal list view
    item = new QStandardItem(title);
    item->setCheckable(false);
    newrow.append(item);
    newrow.append(new QStandardItem(getTypeText(type)));
    double range = static_cast<double>((n_Samples >> 1) + 1) * static_cast<double>(wdw.stepFrequency);
    newrow.append(new QStandardItem(QString::number(range) + "Hz"));
    newrow.append(new QStandardItem(QString::number(static_cast<double>(wdw.stepFrequency)) + "Hz"));
    plotListModel->appendRow(newrow);

    plotListAss.insert(item, wdw.idx);

    if (windowPool.count() == 0)
        setVisible(false);
    else
        setVisible(true);

    return idx;
}

void fftManager::removeFFTWindow(int index)
{
    int i = findWindow(index);
    int idx;

    if (i != -1)
    {
        //we can now delete this window
        windowPool[i].fftWnd->close();
        idx = windowPool[i].idx;
        delete windowPool[i].fftWnd;
        windowPool.removeAt(i);

        QStandardItem *item; bool found = false; int row;
        //find the item that corresponds to the index
        for (i = 0; i < plotListModel->rowCount(); i++)
            if (plotListAss.value(plotListModel->item(i)) == idx)
            {
                item = plotListModel->item(i);
                found = true; row = i;
            }
        if (found == true)
        {
            plotListModel->removeRows(row, 1);
            plotListAss.remove(item);
        }

        if (windowPool.count() == 0)
            setVisible(false);
        else
            setVisible(true);
    }
}

void fftManager::addSigFFT(int windowIdx, int sigIdx, QString name, float line_width, QColor col)
{
    int i = findWindow(windowIdx);

    if (i != -1)
    {
        windowPool[i].sig_name.append(name);
        windowPool[i].line_width.append(line_width);
        windowPool[i].sigIdx.append(sigIdx);
        windowPool[i].sigCol.append(col);
        QVector<float> d;
        d.resize(windowPool[i].n_Samples);
        QVector<float> f;
        f.resize((windowPool[i].n_Samples / 2) + 1);
        windowPool[i].sig_data.append(d);
        windowPool[i].fft_data.append(f);
        windowPool[i].N_sig = windowPool[i].sig_name.count();
        windowPool[i].fftWnd->addSignal(static_cast<uint32_t>(sigIdx), name, col);
    }
}

int fftManager::remSigFFT(int windowIdx, int sigIdx)
{
    int i, j;

    i = findWindow(windowIdx);
    if (i != -1)
    {
        j = findSig(windowIdx, sigIdx);
        if (j != -1)
        {
            windowPool[i].sig_name.removeAt(j);
            windowPool[i].line_width.removeAt(j);
            windowPool[i].sigIdx.removeAt(j);
            windowPool[i].sig_data.removeAt(j);
            windowPool[i].fft_data.removeAt(j);
            windowPool[i].N_sig = windowPool[i].sig_name.count();
            windowPool[i].fftWnd->removeSignal(static_cast<uint32_t>(sigIdx));

            return 0;
        }
    }

    return -1;
}

void fftManager::startFFTCalculation()
{
    status = true;  //the manager is busy

    QFuture<void> future = QtConcurrent::run(this, &fftManager::calculateFFTdata);
    jobWatch.setFuture(future);
}

QVector<int> fftManager::getSignalIndexPerWindow(int wdwIdx)
{
    int pos = findWindow(wdwIdx);
    if (pos != -1)
        return windowPool[pos].sigIdx;
    else
    {
        QVector<int> nullVector;
        nullVector.resize(0);
        return nullVector;
    }
}

void fftManager::updateSigData(int wdwIdx, int sigIdx, float *data, int N_data)
{
    int i, j, N;

    i = findWindow(wdwIdx);
    if (i == -1)
        return;
    j = findSig(wdwIdx, sigIdx);
    if (j == -1)
        return;

    //we consider only the last samples of the signal
    int start;
    start = N_data - windowPool[i].n_Samples;
    if (start < 0)
        start = 0;

    //first we copy the nullVector => done in case zero padding is necessary
    windowPool[i].sig_data[j].clear(); windowPool[i].sig_data[j].resize(windowPool[i].n_Samples);
    memcpy(windowPool[i].sig_data[j].data(), windowPool[i].nullVector.data(), windowPool[i].n_Samples * sizeof(float));
    //now we copy the data
    if (N_data >= windowPool[i].n_Samples)
        N = windowPool[i].n_Samples;
    else
        N = N_data;

    memcpy(windowPool[i].sig_data[j].data(), data + start, N * sizeof(float));
}

QString fftManager::getWindowName(int wdwIdx, bool *ok)
{
    QString ret;
    int i = findWindow(wdwIdx);
    ret = QString::Null();
    if (i == -1)
        *ok = false;
    else
    {
        ret = windowPool[i].title;
        *ok = true;
    }
    return ret;
}

fft_plot_Window *fftManager::getWindowHandler(int pos)
{
    if ((pos < 0) || (pos >= (windowPool.count())))
        return NULL;
    else
        return windowPool[pos].fftWnd;
}

int fftManager::exportToFile(QString filename, int index)
{
    int i;

    if (filename.isEmpty() == true)
        return -1;

    MatlabFileSaver saver;
    float** data;

    //we look for the FFT window containing the data
    int pos = findWindow(index);
    if (pos == -1)
        return -1;

    //now we create the FFT base vector
    QVector<float> baseFrequency; float value = 0.0f;
    baseFrequency.resize((windowPool[pos].n_Samples >> 1) + 1);
    for (i = 0; i < baseFrequency.count(); i++)
    {
        baseFrequency[i] = value;
        value += windowPool[pos].stepFrequency;
    }

    //now we create the data matrix containing all the information to be saved
    data = new float*[windowPool[pos].N_sig + 1];  //+1 because we will also save the baseFrequency vector
    for (i = 0; i < windowPool[pos].N_sig; i++)
        data[i] = windowPool[pos].fft_data[i].data();
    data[windowPool[pos].N_sig] = baseFrequency.data();

    for (i = 0; i < windowPool[pos].N_sig; i++)
        saver.AddSignalInfoToWrite(windowPool[pos].sig_name[i]);
    saver.AddSignalInfoToWrite("Base Frequency Vector");

    //now we can save the matrix onto the file
    return saver.Save_MATLAB_File(data, (windowPool[pos].n_Samples >> 1) + 1, filename);
}

void fftManager::checkGrids(int *n, int *m)
{
    int i;
    int a, b;

    a = 0; b = 0;

    for (i = 0; i < windowPool.count(); i++)
    {
        if (windowPool[i].fftWnd->isGridEnabled() == true)
        {
            a += 1;
        }
        else {
            b += 1;
        }
    }

    *n = a; *m = b;
}

void fftManager::setAllGrids(bool en)
{
    int i;

    for (i = 0; i < windowPool.count(); i++)
        windowPool[i].fftWnd->setGrid(en);
}

void fftManager::jobHasFinished()
{
    //the FFTs have been calculated, plot and set free

    //In order to plot, we prepare the data per each plot
    int i, j;
    float **data;

    for (i = 0; i < windowPool.count(); i++)
    {
        data = new float*[windowPool[i].N_sig];
        for (j = 0; j < windowPool[i].fft_data.count(); j++)
        {
            data[j] = windowPool[i].fft_data[j].data();
        }

        windowPool[i].fftWnd->parallel_prepare_Signal_Data(windowPool[i].N_sig, (windowPool[i].n_Samples >> 1) + 1, data, windowPool[i].sigCol.data(), windowPool[i].line_width.data(), windowPool[i].sig_name.data());

        windowPool[i].fftWnd->update();

        delete data;
    }

    status = false;  //the manager is now free
}

void fftManager::plotListDoubleClicked(const QModelIndex & index)
{
    int row;
    int idx;
    int pos;

    row = index.row();
    idx = plotListAss.value(plotListModel->item(row, 0));

    //we simply call the show function of the window
    pos = findWindow(idx);
    if (pos != -1)
        windowPool[pos].fftWnd->show();
}

void fftManager::gridTriggered()
{
    emit gridActTriggered();
}

int fftManager::getNewIndex()
{
    int i;
    int max;

    max = 0;
    for (i = 0; i < windowPool.count(); i++)
        if (windowPool[i].idx > max)
            max = windowPool[i].idx;
    return max + 1;

}

int fftManager::findWindow(int index)
{
    int i, pos;

    pos = -1;

    for (i = 0; i < windowPool.count(); i++)
        if (windowPool[i].idx == index)
            pos = i;

    return pos;
}

int fftManager::findSig(int wdwIdx, int sigIdx)
{
    int i, k, ret;

    ret = -1;

    i = findWindow(wdwIdx);
    if (i != -1)
    {
        for (k = 0; k < windowPool[i].sigIdx.count(); k++)
            if (windowPool[i].sigIdx[k] == sigIdx)
                ret = k;
    }

    return ret;
}

void fftManager::prepareViewModel()
{
    QStringList headers;

    plotListModel->insertColumns(0, 4);  //inserts 5 columns

    headers.append("Title");
    headers.append("Type");
    headers.append("Range");
    headers.append("Step");

    plotListModel->setHorizontalHeaderLabels(headers);
    plotList->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QString fftManager::getTypeText(int type)
{
    if (type == 0)
        return "Amplitude";
    if (type == 1)
        return "Power";
    if (type == 2)
        return "Power [dB]";
    if ((type < 0) || (type > 2))
        return "Unknown";

    return "Unknown";
}

void fftManager::calculateFFTdata()
{
    //Here we calculate all the selected FFTs
    int i, j, k;
    int N;
    float a_square, b_square;
    double log_10 = 2.30258509299;

    for (i = 0; i < windowPool.count(); i++)
    {
        N = windowPool[i].n_Samples;
        kiss_fftr_cfg cfg = kiss_fftr_alloc(N, 0, 0, 0);
        kiss_fft_scalar *cx_in = new kiss_fft_scalar[N];
        kiss_fft_cpx *cx_out = new kiss_fft_cpx[N];

        for (j = 0; j < windowPool[i].sig_data.count(); j++)
        {
            //fill the data
            for (k = 0; k < N; k++)
                cx_in[k] = windowPool[i].sig_data[j][k];

            //calculates FFT
            kiss_fftr(cfg, cx_in, cx_out);

            //prepares the output data
            for (k = 0; k < (N >> 1) + 1; k++)
            {
                a_square = cx_out[k].r * cx_out[k].r;
                b_square = cx_out[k].i * cx_out[k].i;
                if (windowPool[i].fftType == 0)  //amplitude
                {
                    windowPool[i].fft_data[j][k] = (sqrt((a_square / N / N)  + (b_square / N / N))) * 2.0f;
                    if (k == 0)
                        windowPool[i].fft_data[j][k] = windowPool[i].fft_data[j][k] / 2;
                }
                if (windowPool[i].fftType == 1)  //power
                {
                    windowPool[i].fft_data[j][k] = static_cast<float>(sqrt(a_square + b_square) * sqrt(a_square + b_square) / N);
                }
                if (windowPool[i].fftType == 2)  //power_db
                {
                    windowPool[i].fft_data[j][k] = 10.0f * static_cast<float>(log(sqrt(a_square + b_square) * sqrt(a_square + b_square) / N)) / log_10;
                }
            }

        }
        free(cfg);
        delete[] cx_in;
        delete[] cx_out;
    }
    //Job finished
}
