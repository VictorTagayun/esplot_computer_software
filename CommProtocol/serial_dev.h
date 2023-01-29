/**
  *********************************************************************************************************************************************************
  @file     :serial_dev.h
  @brief    :Headers of the COM Port communication class
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

#ifndef SERIAL_DEV
#define SERIAL_DEV

#include "comm_dev.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QByteArray>

#include <iostream>
#include <vector>

#include <QObject>



class serial_dev: public QObject, public comm_dev {

    Q_OBJECT

    public:
        explicit serial_dev(QObject* parent = nullptr);
        ~serial_dev();

        vector<device_description_t> get_list_of_devices();
        connection_status_t connect(unsigned int idx);
        connection_status_t disconnect();

        void purge_buffers();
        int send_buffer(vector<byte> &tx_buff);
        unsigned int get_rx_available_size();
        int  receive_buffer(vector<byte> &rx_buff);
        int  receive_all(vector<byte> &rx_buff);

        void set_baudrate(int b_rate);

    private:
        void handleReadyRead(){};
        QSerialPort serial_dev_handle;

        int baudrate;

signals:
public slots:


};


#endif
