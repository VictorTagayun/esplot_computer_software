/**
  *********************************************************************************************************************************************************
  @file     :comm_prot.h
  @brief    :Definitions of the Communication Protocol Library
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

#ifndef COMM_PROT_H
#define COMM_PROT_H

#include "comm_dev.h"
#include <utility>
#include <algorithm>
#include <string>
#include <cstring>

/**
 * Version of the used protocol
 */
#define COMM_PROT_VERSION 90

/**
 * Defines for signals types
 */
#define TYPE_FLOAT		6
#define TYPE_INT32		5
#define TYPE_UINT32		4
#define TYPE_INT16		3
#define	TYPE_UINT16		2
#define TYPE_UINT8		1
#define TYPE_INT8		0

/**
 * Defines if conversion factor is to be applied
 */
#define SCALING_FACTOR_APPLIED		1
#define SCALING_FACTOR_NOT_APPLIED 	0

/**
 * Defines for the record functionality
 */
#define NO_CMD		0
#define RECORD_CMD	1

class comm_prot{

public:
    comm_prot();
    ~comm_prot();

    typedef enum {UNCONNECTED, UNINITIALIZED, INITIALIZED, ESTABLISHED} prot_status_t;
    typedef enum {NOT_READY, NO_DATA_AVAILABLE, COMM_ERROR, PARSE_ERROR, DECODE_ERROR, WRONG_VERSION, SUCCESS} error_t;
    typedef struct{
        unsigned int idx;
        uint8_t scaling_factor_applied;
        string signal_name;
        uint8_t type;
        uint16_t representation;
        float scaling_factor;
        uint8_t line_width;			//line width of the plot

        uint8_t alpha;				//color of the plot
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } comm_data_descriptor_t;

    unsigned int n_rx_errors;
    unsigned int n_rx_completetion_errors;
    unsigned int n_rx_corrupted_errors;
    unsigned int n_rx_decode_errors;

    int connect(comm_dev* comm_dev_h);
    int disconnect();
    error_t request_descriptor_frame_and_initialize_comm_prot();
    int parse_data();
    vector<float> decode_frame(vector<byte> data_frame);
    int decode_data();
    unsigned int get_recommended_trigger_time();

    unsigned int get_n_tx_data() {return n_tx_data;}
    unsigned int get_n_rx_data() {return n_rx_data;}
    unsigned int get_prot_status() {return prot_status;}
    unsigned int get_buff_dimension() {return buff_dimension;}
    unsigned int get_process_freq() {return process_freq;}
    unsigned int get_n_rx_errors() {return n_rx_errors;}
    vector<comm_data_descriptor_t> get_rx_data_descriptor_list() {return rx_data_descriptor_list;}
    vector<comm_data_descriptor_t> get_tx_data_descriptor_list() {return tx_data_descriptor_list;}

    void reset_buffers();

    error_t comm_manager();

    void set_terminal_command(string cmd);
    int set_tx_data(vector<int> tx_data);
    vector<vector<float>> get_rx_data();
    vector<uint8_t> get_cmd();

private:
    comm_dev* comm_dev_handle;

    vector<comm_data_descriptor_t> tx_data_descriptor_list;
    vector<comm_data_descriptor_t> rx_data_descriptor_list;
    unsigned int n_tx_data;
    unsigned int n_rx_data;
    unsigned int buff_dimension;
    unsigned int process_freq;
    float process_time_step;
    unsigned int recommended_trigger_time;


    prot_status_t prot_status;

    vector<byte> rx_actu_buff;
    vector<byte> tx_actu_buff;
    vector<byte> rx_remain_buff;
    vector<vector<byte>> rx_decode_buff;

    vector<vector<float>> decoded_rx_data;
    vector<uint8_t> decoded_cmd;

    bool first_run;

    string terminal_command;
};

#endif
