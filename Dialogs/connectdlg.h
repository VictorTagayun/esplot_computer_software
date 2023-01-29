/**
  *********************************************************************************************************************************************************
  @file     :connectdlg.h
  @brief    :Header for Connect Dialog class
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

#ifndef CONNECTDLG_H
#define CONNECTDLG_H

#include <QMainWindow>
#include <QDialog>
#include <QWidget>
#include <QDialogButtonBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QString>
#include <QSpinBox>

#include "CommProtocol/ft4222_dev.h"
#include "CommProtocol/serial_dev.h"

class connectDlg : public QDialog
{
    Q_OBJECT

public:
    connectDlg(QMainWindow *parent = 0, ft4222_dev *ft_device = 0, serial_dev *serial_device = 0);
    ~connectDlg();

    bool isDeviceSelected() { return deviceSelected; }
    comm_dev::device_description_t get_SelectedDevice() { return selectedDevice; }
    int get_SelectedDeviceType() { return selectedDeviceType; }
    int get_BaudRate() { return baudrate; }

public slots:
    void showEvent(QShowEvent* event) override;

    void applyandclose();
    void selectedFTDeviceChanged(int index);
    void selectedSerialDeviceChanged(int index);
    void baudrateChanged();

private:
    ft4222_dev *ftDevice;
    serial_dev *serialDevice;

    int baudrate;

    vector<ft4222_dev::device_description_t> ftList;
    vector<serial_dev::device_description_t> serialList;

    comm_dev::device_description_t selectedDevice;
    int selectedDeviceType;  //0 => FT device; 1 => Serial device
    bool deviceSelected;

    QDialogButtonBox *buttonBox;
    QTabWidget *tabWidget;
    QWidget *ftListWidget;
    QWidget *serialListWidget;
    QComboBox *ftListCB;
    QComboBox *serialListCB;
    QLabel *ftLabel;
    QLabel *serialLabel;
    QSpinBox *baudrateSB;

    void fillListWidgets();
    void populateDevices();
};

#endif // CONNECTDLG_H
