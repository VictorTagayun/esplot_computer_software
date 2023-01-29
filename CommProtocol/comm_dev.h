/**
  *********************************************************************************************************************************************************
  @file     :comm_dev.h
  @brief    :Definitions of a generic communication device
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

#ifndef COMM_DEV
#define COMM_DEV

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>

using namespace std;

#define byte uint8_t

class comm_dev{

     public:
        comm_dev() {connection_status = NOT_FOUND;}
        virtual ~comm_dev() {connection_status = NOT_FOUND;}

        typedef enum {NOT_FOUND, UNINITIALIZED, INITIALIZED, CONNECTED, DISCONNECTED, COMM_ERROR} connection_status_t;
        typedef struct{
            unsigned int idx;
            string name;
            string description;
            string serial_number;
        } device_description_t;

        connection_status_t get_status() {return connection_status;}
        unsigned int get_internal_buffer_size() {return internal_buffer_size;}
        virtual vector<device_description_t> get_list_of_devices() = 0;
        virtual connection_status_t connect(unsigned int idx) = 0;
        virtual connection_status_t disconnect() = 0;

        virtual void purge_buffers() = 0;
        virtual int send_buffer(vector<byte> &tx_buff) = 0;
        virtual unsigned int get_rx_available_size() = 0;
        virtual int  receive_buffer(vector<byte> &rx_buff) = 0;
        virtual int  receive_all(vector<byte> &rx_buff) = 0;

    protected:
        connection_status_t connection_status;
        vector<device_description_t> list_of_devices;
        unsigned int internal_buffer_size;


};




#endif
