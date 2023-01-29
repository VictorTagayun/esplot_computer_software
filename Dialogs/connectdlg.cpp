/**
  *********************************************************************************************************************************************************
  @file     :connectdlg.cpp
  @brief    :Functions for Connect Dialog class
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

#include "connectdlg.h"

#include <QDebug>

using namespace std;

connectDlg::connectDlg(QMainWindow *parent, ft4222_dev *ft_device, serial_dev *serial_device)
{
    (void) parent;

    ftDevice = ft_device;
    serialDevice = serial_device;

    baudrate = 1000000;

    //Creates dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &connectDlg::applyandclose);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &connectDlg::close);

    ftListWidget = new QWidget;
    serialListWidget = new QWidget;
    fillListWidgets();

    //Creates the tab
    tabWidget = new QTabWidget;
    tabWidget->addTab(serialListWidget, "Serial COM Devices");
    tabWidget->addTab(ftListWidget, "FT4222 USB Devices");   

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    if ((ftDevice != 0) &&(serialDevice != 0))
    {
        populateDevices();
    }

    deviceSelected = false;

    //set focus on first element
    if (serialList.size() > 0)
    {
        serialListCB->setCurrentIndex(0);
        selectedSerialDeviceChanged(0);
    }
    if (ftList.size() > 0)
    {
        ftListCB->setCurrentIndex(0);
        selectedFTDeviceChanged(0);
    }
}

connectDlg::~connectDlg()
{
    delete ftListWidget;
    delete serialListWidget;
    delete tabWidget;
}

void connectDlg::showEvent(QShowEvent *event)
{
    (void) event;
    resize(260, 200);
}

void connectDlg::applyandclose()
{
    if ((ftList.size() > 0) || (serialList.size() > 0))
        deviceSelected = true;
    close();
}

void connectDlg::selectedFTDeviceChanged(int index)
{
    selectedDeviceType = 0;  //FT device chosen
    selectedDevice = ftList[static_cast<unsigned int>(index)];
}

void connectDlg::selectedSerialDeviceChanged(int index)
{
    selectedDeviceType = 1;  //Serial device chosen
    selectedDevice = serialList[static_cast<unsigned int>(index)];
}

void connectDlg::baudrateChanged()
{
    baudrate = baudrateSB->value();
}

void connectDlg::fillListWidgets()
{
    ftLabel = new QLabel("FT4222 devices:");
    ftListCB = new QComboBox;
    connect(ftListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedFTDeviceChanged(int)));
    QVBoxLayout *ftLayout = new QVBoxLayout;

    ftLayout->addWidget(ftLabel);
    ftLayout->addWidget(ftListCB);
    ftLayout->addStretch();
    ftListWidget->setLayout(ftLayout);

    serialLabel = new QLabel("Serial COM devices:");
    serialListCB = new QComboBox;
    connect(serialListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedSerialDeviceChanged(int)));
    QVBoxLayout *serialLayout = new QVBoxLayout;

    baudrateSB = new QSpinBox;
    baudrateSB->setMinimum(300);
    baudrateSB->setMaximum(10000000);
    baudrateSB->setValue(baudrate);
    connect(baudrateSB, &QSpinBox::editingFinished, this, &connectDlg::baudrateChanged);

    serialLayout->addWidget(serialLabel);
    serialLayout->addWidget(serialListCB);
    serialLayout->addWidget(new QLabel("Baudrate (baud/s):"));
    serialLayout->addWidget(baudrateSB);
    serialLayout->addStretch();
    serialListWidget->setLayout(serialLayout);
}

void connectDlg::populateDevices()
{
    unsigned int i;

    ftList = ftDevice->get_list_of_devices();

    for (i = 0; i < ftList.size(); i++)
        ftListCB->addItem(QString::fromStdString(ftList[i].name));

    serialList = serialDevice->get_list_of_devices();

    for (i = 0; i < serialList.size(); i++)
        serialListCB->addItem(QString::fromStdString(serialList[i].name));
}
