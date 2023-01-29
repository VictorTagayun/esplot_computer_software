/**
  *********************************************************************************************************************************************************
  @file     :sigassdlg.h
  @brief    :Header for Signal Assignment Dialog class
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

#ifndef SIGASSDLG_H
#define SIGASSDLG_H

#include <QDialog>
#include <QVector>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QMap>
#include <QColor>
#include <QColorDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QItemSelectionModel>
#include <QMap>
#include <QMessageBox>

#include "definitions.h"
#include "math.h"

class sigAssDlg : public QDialog
{
    Q_OBJECT

public:
    sigAssDlg(uint32_t index, QVector<Plot_Structure> pool);
    bool getAcceptFlag() { return acceptChanges; }
    QVector<Plot_Structure> getPlotStruct() { return Plot_Pool; }

private slots:
    void acceptandclose();
    void colorChange();
    void lineWidthChange();
    void plotChange(const QItemSelection &selected);

private:
    bool acceptChanges;

    QVector<Plot_Structure> Plot_Pool;  //this is our pool of Plots
    uint32_t N_Plots;  //number of plots in the pool

    uint32_t sig_idx;  //index of the signal

    QMap<QStandardItem*, uint32_t> plotIdxMap;

    QListView *plotView;
    QItemSelectionModel *selectModel;
    QStandardItemModel *plotViewModel;
    QSpinBox *lwSB;
    QPushButton *colorPB;
    QDialogButtonBox *buttonBox;

    int activeIdx;

    int populate();
    int findAssSigFromPlot(int plot_idx);
    QColor invertColor(QColor in);
};

#endif // SIGASSDLG_H
