/**
  *********************************************************************************************************************************************************
  @file     :ft4222_dev.cpp
  @brief    :Functions of the FT4222 communication class
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

#include "ft4222_dev.h"

#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

ft4222_dev::ft4222_dev()
{
    connection_status = NOT_FOUND; 
    ftHandle_SPI = nullptr;
    ftHandle_GPIO = nullptr;
    ftStatus = 0;
    ft4222Status = FT4222_OK;
    internal_buffer_size = 65535;
}

ft4222_dev::~ft4222_dev()
{
    if (connection_status != DISCONNECTED)
    {
        disconnect();
    }

    connection_status = NOT_FOUND;
    ftHandle_SPI = nullptr;
    ftHandle_GPIO = nullptr;
    ftStatus = 0;
    ft4222Status = FT4222_OK;
    internal_buffer_size = 0;
}

vector<ft4222_dev::device_description_t> ft4222_dev::get_list_of_devices()
{
    DWORD numOfDevices = 0;
    ftStatus = FT_CreateDeviceInfoList(&numOfDevices);
    devInfo.resize(numOfDevices);
    list_of_devices.resize(0);

    for(DWORD i=0; i<numOfDevices; ++i)
    {
        memset(&devInfo[i], 0, sizeof(devInfo[i]));

        ftStatus = FT_GetDeviceInfoDetail(i, &devInfo[i].Flags, &devInfo[i].Type, &devInfo[i].ID, &devInfo[i].LocId, devInfo[i].SerialNumber, devInfo[i].Description, &devInfo[i].ftHandle);

        if (ftStatus == FT_OK)
        {
            string name(devInfo[i].Description);
            if(name.find("FT4222") != std::string::npos)
            {
                device_description_t temp;
                temp.idx = i;
                temp.name = name;
                temp.description = name; //No real description included, therefore use name again
                string serial_number(devInfo[i].SerialNumber);
                temp.serial_number = std::to_string(devInfo[i].LocId);
                list_of_devices.push_back(temp);
            }
        }
    }

    return list_of_devices;
}

ft4222_dev::connection_status_t ft4222_dev::connect(unsigned int idx)
{  
    if (static_cast<int>(idx) > (static_cast<int>(devInfo.size())-1))   //-1 because no GPIO device should be opened as first device
    {
        connection_status = NOT_FOUND;
        return connection_status;
    }

    ftStatus = FT_OK;

    try {
        ftStatus |= FT_OpenEx((PVOID)devInfo[idx].LocId, FT_OPEN_BY_LOCATION, &ftHandle_SPI); //from example application, warnings cannot be solved
        ftStatus |= FT_OpenEx((PVOID)devInfo[idx+1].LocId, FT_OPEN_BY_LOCATION, &ftHandle_GPIO);
    } catch (...)
    {
        connection_status = COMM_ERROR;
        return connection_status;
    }



    if ((ftStatus != FT_OK) || (ftHandle_SPI == nullptr) || (ftHandle_GPIO == nullptr))
    {
        connection_status = COMM_ERROR;
        return connection_status;
    }

    ft4222Status = FT4222_SetClock(ftHandle_SPI, SYS_CLK_80);
    if (ft4222Status != FT4222_OK)
    {
        connection_status = UNINITIALIZED;
        return connection_status;
    }
    ft4222Status = FT4222_SPISlave_InitEx(ftHandle_SPI, SPI_SLAVE_NO_PROTOCOL);
    if (ft4222Status != FT4222_OK)
    {
        connection_status = UNINITIALIZED;
        return connection_status;
    }
    ft4222Status = FT4222_SetSuspendOut(ftHandle_GPIO, false);
    if (ft4222Status != FT4222_OK)
    {
        connection_status = UNINITIALIZED;
        return connection_status;
    }
    GPIO_Dir gpioDir[4] = {GPIO_OUTPUT, GPIO_OUTPUT, GPIO_OUTPUT, GPIO_OUTPUT};
    if (ft4222Status != FT4222_OK)
    {
        connection_status = UNINITIALIZED;
        return connection_status;
    }
    ft4222Status = FT4222_GPIO_Init(ftHandle_GPIO, gpioDir);  //all GPIOs are set to output
    if (ft4222Status != FT4222_OK)
    {
        connection_status = UNINITIALIZED;
        return connection_status;
    }
    set_GPIO(false);

    connection_status = CONNECTED;
    return connection_status;
}

ft4222_dev::connection_status_t ft4222_dev::disconnect()
{
    set_GPIO(false);

    FT4222_SPI_Reset(ftHandle_SPI);

    FT4222_UnInitialize(ftHandle_SPI);
    FT4222_UnInitialize(ftHandle_GPIO);

    ftStatus = 0;
    ftStatus = FT_Close(ftHandle_SPI);
    ftStatus |= FT_Close(ftHandle_GPIO);

    if (ftStatus != FT_OK)
    {
        connection_status = COMM_ERROR;
        return connection_status;
    }
    else
    {
        connection_status = DISCONNECTED;
        return connection_status;
    }
}

void ft4222_dev::purge_buffers()
{
    vector<byte> rx_purge_buffer(internal_buffer_size, 0);
    receive_all(rx_purge_buffer);
}

int ft4222_dev::send_buffer(vector<byte> &tx_buff)
{
    if (connection_status != CONNECTED)
        return EXIT_FAILURE;

    uint16_t size_transferred = 0;
    ft4222Status = FT4222_SPISlave_Write(ftHandle_SPI, &tx_buff[0], static_cast<uint16_t>(tx_buff.size()), &size_transferred);

    if (ft4222Status == FT4222_OK)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

unsigned int ft4222_dev::get_rx_available_size()
{
    if (connection_status != CONNECTED)
        return 0;

    uint16_t rx_buff_size = 0;
    ft4222Status = FT4222_SPISlave_GetRxStatus(ftHandle_SPI, &rx_buff_size);

    if (ft4222Status == FT4222_OK)
        return static_cast<unsigned int>(rx_buff_size);
    else
        return 0;
}

int ft4222_dev::receive_buffer(vector<byte> &rx_buff)
{
    if (connection_status != CONNECTED)
        return EXIT_FAILURE;

    uint16_t size_transferred = 0;
    ft4222Status = FT4222_SPISlave_Read(ftHandle_SPI, &rx_buff[0], static_cast<uint16_t>(rx_buff.size()), &size_transferred);

    if (ft4222Status == FT4222_OK)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

int ft4222_dev::receive_all(vector<byte> &rx_buff)
{
    if (connection_status != CONNECTED)
        return EXIT_FAILURE;

    unsigned int rx_buff_size = get_rx_available_size();
    rx_buff.resize(rx_buff_size);
    return receive_buffer(rx_buff);
}
void ft4222_dev::set_GPIO(bool state)
{
    if (connection_status == CONNECTED)
    {
        FT4222_GPIO_Write(ftHandle_GPIO, GPIO_PORT0, state);
        FT4222_GPIO_Write(ftHandle_GPIO, GPIO_PORT1, state);
    }
}
