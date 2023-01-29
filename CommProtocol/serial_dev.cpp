/**
  *********************************************************************************************************************************************************
  @file     :serial_dev.cpp
  @brief    :Functions of the COM port communication class
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

#include "serial_dev.h"

serial_dev::serial_dev(QObject* parent): QObject(parent)
{
    connection_status = NOT_FOUND; 
    internal_buffer_size = 65535;

    baudrate = 1000000;  //by default
}

serial_dev::~serial_dev()
{
    if (connection_status != DISCONNECTED)
        disconnect();
    connection_status = NOT_FOUND;
    internal_buffer_size = 0;
}

vector<serial_dev::device_description_t> serial_dev::get_list_of_devices()
{
    const auto list_serial_port = QSerialPortInfo::availablePorts();
    list_of_devices.resize(list_serial_port.count());

    unsigned int i = 0;
    for(const QSerialPortInfo &serial_port_info : list_serial_port)
    {
        list_of_devices[i].idx = i;
        QString portName =serial_port_info.portName();
        list_of_devices[i].name = portName.toStdString(); 
        QString description =serial_port_info.description();
        list_of_devices[i].description = description.toStdString();
        QString serialNumber =serial_port_info.serialNumber();
        list_of_devices[i].serial_number = serialNumber.toStdString(); 
                    
        i++;
    }

    return list_of_devices;
}

serial_dev::connection_status_t serial_dev::connect(unsigned int idx)
{  
    if (static_cast<int>(idx) > (static_cast<int>(list_of_devices.size())) )
    {
        connection_status = NOT_FOUND;
        return connection_status;
    }

    QString name(list_of_devices[idx].name.c_str());
    serial_dev_handle.setPortName(name);
    serial_dev_handle.setBaudRate((qint64) baudrate);
    serial_dev_handle.setDataBits(QSerialPort::Data8);
    serial_dev_handle.setParity(QSerialPort::NoParity);
    serial_dev_handle.setStopBits(QSerialPort::OneStop);
    serial_dev_handle.setFlowControl(QSerialPort::NoFlowControl);
    
    if (serial_dev_handle.open(QIODevice::ReadWrite))
    {
        serial_dev_handle.setReadBufferSize((qint64) internal_buffer_size);
        connection_status = CONNECTED;
    }
    else
        connection_status = DISCONNECTED;
        
    return connection_status;
}

comm_dev::connection_status_t serial_dev::disconnect()
{
    if (serial_dev_handle.isOpen())
        serial_dev_handle.close();

    connection_status = DISCONNECTED;
    return connection_status;
}

void serial_dev::purge_buffers()
{
    serial_dev_handle.flush();
}

int serial_dev::send_buffer(vector<byte> &tx_buff)
{
    if (connection_status != CONNECTED)
        return EXIT_FAILURE;

    QByteArray temp;

    for (unsigned int i = 0; i < tx_buff.size(); i++)
       temp.append(tx_buff[i]);

    serial_dev_handle.write(temp);
    return EXIT_SUCCESS;
}

unsigned int serial_dev::get_rx_available_size()
{
    if (connection_status != CONNECTED)
        return 0;

    serial_dev_handle.waitForReadyRead(1);

    unsigned int test = (unsigned int) serial_dev_handle.bytesAvailable();

    return ((unsigned int) test);
}

int serial_dev::receive_buffer(vector<byte> &rx_buff)
{
    if (connection_status != CONNECTED)
        return EXIT_FAILURE;

    QByteArray temp = serial_dev_handle.read((qint64) rx_buff.size());

    for (unsigned int i = 0; i < static_cast<unsigned int>(temp.length()); i++)
        rx_buff[i]= static_cast<byte>(temp[i]);

    return EXIT_SUCCESS;
}

int serial_dev::receive_all(vector<byte> &rx_buff)
{
    if (connection_status != CONNECTED)
        return EXIT_FAILURE;

    QByteArray temp = serial_dev_handle.readAll();
    rx_buff.resize(temp.length());

    for (unsigned int i = 0; i < static_cast<unsigned int>(temp.length()); i++)
        rx_buff[i]= static_cast<byte>(temp[i]);

    return EXIT_SUCCESS;
}

void serial_dev::set_baudrate(int b_rate)
{
    baudrate = b_rate;
}
