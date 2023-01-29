/**
  *********************************************************************************************************************************************************
  @file     :sigassdlg.cpp
  @brief    :Functions for Signal Assignment Dialog class
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

#include "sigassdlg.h"

#include <QDebug>

sigAssDlg::sigAssDlg(uint32_t index, QVector<Plot_Structure> pool)
{
    acceptChanges = false;
    activeIdx = -1;
    sig_idx = index;  //this is the signal index on which the properties are set
    Plot_Pool = pool;  //contains all info about plots and associated signals

    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *hlay = new QHBoxLayout;

    plotView = new QListView;
    plotViewModel = new QStandardItemModel;
    plotView->setModel(plotViewModel);
    lwSB = new QSpinBox(this);
    colorPB = new QPushButton(this);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    lwSB->setMinimum(1); lwSB->setMaximum(100);

    plotView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    plotView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    selectModel = plotView->selectionModel();

    connect(buttonBox, &QDialogButtonBox::accepted, this, &sigAssDlg::acceptandclose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &sigAssDlg::close);
    connect(colorPB, &QPushButton::clicked, this, &sigAssDlg::colorChange);
    connect(lwSB, &QSpinBox::editingFinished, this, &sigAssDlg::lineWidthChange);
    connect(selectModel, &QItemSelectionModel::selectionChanged, this, &sigAssDlg::plotChange);

    hlay->addWidget(lwSB); hlay->addWidget(colorPB);

    layout->addWidget(new QLabel("Associated plots:"));
    layout->addWidget(plotView);
    layout->addWidget(new QLabel("Line width and color:"));
    layout->addLayout(hlay);
    layout->addWidget(buttonBox);

    this->setLayout(layout);

    int res = populate();
    if (res == -1)
    {
        QMessageBox msgBox;
        msgBox.setText("This signal is not associated to any plot!");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        QMetaObject::invokeMethod( this, "close", Qt::QueuedConnection );
    }
    else
    {
        QModelIndex idx = plotViewModel->index(0, 0);
        plotView->setCurrentIndex(idx);
    }
}

void sigAssDlg::acceptandclose()
{
    acceptChanges = true;
    this->close();
}

void sigAssDlg::colorChange()
{
    if (activeIdx == -1)
        return;
    int plot_idx;
    int s_idx;

    plot_idx = static_cast<int>(plotIdxMap.value(plotViewModel->item(activeIdx, 0)));
    s_idx = findAssSigFromPlot(plot_idx);
    if (s_idx == -1)
        return;

    QColor color = QColorDialog::getColor(Plot_Pool[plot_idx].signals_associated[s_idx].signal_color, this);
    if (color.isValid())
    {
        Plot_Pool[plot_idx].signals_associated[s_idx].signal_color = color;
    }
    colorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(color.name()).arg(invertColor(color).name()));
}

void sigAssDlg::lineWidthChange()
{
    if (activeIdx == -1)
        return;
    int plot_idx;
    int s_idx;

    plot_idx = static_cast<int>(plotIdxMap.value(plotViewModel->item(activeIdx, 0)));
    s_idx = findAssSigFromPlot(plot_idx);
    if (s_idx == -1)
        return;

    Plot_Pool[plot_idx].signals_associated[s_idx].line_width = lwSB->value();
}

void sigAssDlg::plotChange(const QItemSelection &selected)
{
    if (selected.indexes().isEmpty() == true)
        return;

    activeIdx = selected.indexes().first().row();
    if (activeIdx == -1)
        return;

    int plot_idx;
    int s_idx;

    plot_idx = static_cast<int>(plotIdxMap.value(plotViewModel->item(activeIdx, 0)));
    s_idx = findAssSigFromPlot(plot_idx);
    if (s_idx == -1)
        return;

    QColor col = Plot_Pool[plot_idx].signals_associated[s_idx].signal_color;

    //Now we can update the values
    colorPB->setStyleSheet(QString("background-color: %1; color: %2").arg(col.name()).arg(invertColor(col).name()));
    lwSB->setValue(static_cast<int>(Plot_Pool[plot_idx].signals_associated[s_idx].line_width));
}

int sigAssDlg::populate()
{
    //we look in the plot_pool for the plots that contain the given signal
    int i, j;
    bool found;
    QList<QStandardItem*> newrow;
    QStandardItem* item;
    int counter;

    counter = 0;

    for (i = 0; i < Plot_Pool.count(); i++)
    {
        found = false;
        newrow.clear();
        for (j = 0; j < Plot_Pool[i].signals_associated.count(); j++)
        {
            if (Plot_Pool[i].signals_associated[j].signal_ID == sig_idx)
                found = true;
        }
        if (found == true)  //then we can add this plot to the list
        {
            item = new QStandardItem(Plot_Pool[i].title);
            item->setCheckable(false);
            newrow.append(item);
            plotViewModel->appendRow(newrow);
            plotIdxMap.insert(item, static_cast<uint32_t>(i));
            counter++;
        }
    }

    if (counter == 0)
        return -1;
    else
        return 0;
}

int sigAssDlg::findAssSigFromPlot(int plot_idx)
{
    int i, idx;

    idx = -1;
    for (i = 0; i < Plot_Pool[plot_idx].signals_associated.count(); i++)
        if (Plot_Pool[plot_idx].signals_associated[i].signal_ID == sig_idx)
            idx = i;

    return idx;
}

QColor sigAssDlg::invertColor(QColor in)
{
    double r, g, b, a;
    QColor out;

    in.getRgbF(&r, &g, &b, &a);
    out.setRedF(fabs(1.0 - r));
    out.setGreenF(fabs(1.0 - g));
    out.setBlueF(fabs(1.0 - b));
    out.setAlphaF(a);

    return out;
}
