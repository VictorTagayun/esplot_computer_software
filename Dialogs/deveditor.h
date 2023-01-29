/**
  *********************************************************************************************************************************************************
  @file     :deveditor.h
  @brief    :Header for device editor class (send data commands)
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

#ifndef DEVEDITOR_H
#define DEVEDITOR_H

#include <QWidget>
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableView>
#include <QCheckBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QVector>
#include <QPushButton>
#include <QStringList>
#include <QModelIndex>
#include <QLineEdit>
#include <QDebug>


typedef struct _devEditType
{
    QString sig_name;
    int index;
    int value;
} devEditType;

class devEditor : public QWidget
{
    Q_OBJECT

public:
    devEditor();
    ~devEditor();

    void Add_Signal(QString name, int index, int initialValue);
    QVector<int> get_Data();
    QString get_Command();

signals:
    void dataReady();

private slots:
    void itemClicked(const QModelIndex &index);

    void updateCmdTxt();
    void updateFromHex();
    void updateFromBin();
    void updateFromDec();

    void sendPushed();

private:
    QVector<devEditType> Sig_Pool;

    QTableView *dataView;
    QStandardItemModel *dataModel;

    QString command_string;

    QLabel *nameLBL;
    QLineEdit *txtCommandLE;
    QLineEdit *decEntryLE;
    QLineEdit *hexEntry1LE;
    QLineEdit *hexEntry2LE;
    QLineEdit *hexEntry3LE;
    QLineEdit *hexEntry4LE;
    QLineEdit *binEntry1LE;
    QLineEdit *binEntry2LE;
    QLineEdit *binEntry3LE;
    QLineEdit *binEntry4LE;
    QLineEdit *binEntry5LE;
    QLineEdit *binEntry6LE;
    QLineEdit *binEntry7LE;
    QLineEdit *binEntry8LE;
    QCheckBox *sendCB;

    QPushButton *sendBut;

    int activeRowIndex;

    void prepareViewModel();
    void prepareWindow();

    QString get_Hex(int value);
    QString get_Bin(int value);

    void viewEntries(int value);
    void updateEntries();
    void updateDataModel();
};

#endif // DEVEDITOR_H
