/**
  *********************************************************************************************************************************************************
  @file     :comm_prot.cpp
  @brief    :Functions of the Communication Protocol Library
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

#include "comm_prot.h"

#pragma GCC diagnostic ignored "-Wstrict-aliasing"

comm_prot::comm_prot()
{
    comm_dev_handle = nullptr;
    n_tx_data = 0;
    n_rx_data = 0;
    prot_status = UNCONNECTED;
    n_rx_errors = 0;
}

comm_prot::~comm_prot()
{
    comm_dev_handle = nullptr;
    n_tx_data = 0;
    n_rx_data = 0;
    prot_status = UNCONNECTED;
    n_rx_errors = 0;
}

int comm_prot::connect(comm_dev* comm_dev_h)
{
    comm_dev::connection_status_t res = comm_dev_h->get_status();

    if(res != comm_dev::CONNECTED)
        return EXIT_FAILURE;

    comm_dev_handle = comm_dev_h;

    prot_status = UNINITIALIZED;

    rx_actu_buff.resize(comm_dev_handle->get_internal_buffer_size());
    tx_actu_buff.resize(comm_dev_handle->get_internal_buffer_size());
    reset_buffers();

    return EXIT_SUCCESS;
}

int comm_prot::disconnect()
{
    comm_dev::connection_status_t res = comm_dev_handle->disconnect();
    if(res != comm_dev::DISCONNECTED)
        return EXIT_FAILURE;

    prot_status = UNCONNECTED;

    return EXIT_SUCCESS;
}

comm_prot::error_t comm_prot::request_descriptor_frame_and_initialize_comm_prot()
{
    //Check if data are available
    if (comm_dev_handle->get_rx_available_size() < 20)
        return NO_DATA_AVAILABLE;

    //Check if receiving was successfully
    if (comm_dev_handle->receive_all(rx_actu_buff) != EXIT_SUCCESS)
        return COMM_ERROR;

    //Find the position where the start sequence is
    vector<byte> terminator_start{0xFF,0xFF,0xFF,0xFF};
    vector<byte> terminator_end{0xEE,0xEE,0xEE,0xEE};
    vector<byte>::iterator it_info_start = search(rx_actu_buff.begin(), rx_actu_buff.end(), terminator_start.begin(), terminator_start.end());
    if ((*(it_info_start+4)==0xFF) && (*(it_info_start+5)==0xFF)&&(*(it_info_start+6)!=0xFF) && (*(it_info_start+7)!=0xFF))
        return PARSE_ERROR; //a data frame was found
    if (it_info_start == rx_actu_buff.end())
        return PARSE_ERROR; //no info frame was found

    //Find the position where the end sequence is (= position of the next start sequence)
    vector<byte>::iterator it_info_end = search(it_info_start+4, rx_actu_buff.end(), terminator_end.begin(), terminator_end.end());
    if (it_info_end == rx_actu_buff.end())
        return PARSE_ERROR; //no end sequence found, the info frame is corrupted

    //Create a new vector containing only the found info_frame
    vector<byte> info_frame(it_info_start+4, it_info_end);

    //Counter-check the info frame integrity
    if ((info_frame[0] != 0x0F))
        return DECODE_ERROR;

    if (info_frame[3] != COMM_PROT_VERSION)
        return WRONG_VERSION;

    n_rx_data = info_frame[1] + 1;//+ 1 because of the time
    n_tx_data = info_frame[2];
    tx_data_descriptor_list.resize(n_tx_data);
    rx_data_descriptor_list.resize(n_rx_data);

    //Counter-check the length of the frame
    unsigned int dim = 12 + ((n_rx_data + n_tx_data - 1) * 30);  //- 1 because of the time
    if (info_frame.size() != dim)
        return DECODE_ERROR;

    //Now iterate through the info frame for decoding
    vector<byte>::iterator idx = info_frame.begin() + 4;

    buff_dimension = static_cast<unsigned int>((*idx << 24) | (*(idx+1) << 16) | (*(idx+2) << 8) | *(idx+3));
    tx_actu_buff.resize((6 + 16 + 5*(n_tx_data))); //5 bytes per each signal, 16 for the terminal command and 6 for the init frame
    idx += 4;

    process_freq = static_cast<unsigned int>((*idx << 24) | (*(idx+1) << 16) | (*(idx+2) << 8) | *(idx+3));
    idx += 4;
    process_time_step = 1.0f / static_cast<float>(process_freq);

    //Put the time signal into the rx_data_descriptor list
    rx_data_descriptor_list[0].idx = 0;
    rx_data_descriptor_list[0].scaling_factor_applied = SCALING_FACTOR_APPLIED;
    rx_data_descriptor_list[0].type = TYPE_UINT32;
    rx_data_descriptor_list[0].signal_name = "Time";
    rx_data_descriptor_list[0].representation = 0;
    rx_data_descriptor_list[0].scaling_factor = process_time_step; //scaling factor here is the process time step
    rx_data_descriptor_list[0].r = 0;
    rx_data_descriptor_list[0].g = 0;
    rx_data_descriptor_list[0].b = 0;
    rx_data_descriptor_list[0].alpha = 0;
    rx_data_descriptor_list[0].line_width = 0;

    for (unsigned int i = 1; i < rx_data_descriptor_list.size(); i++)
    {
        rx_data_descriptor_list[i].idx = *idx;
        idx++;
        rx_data_descriptor_list[i].scaling_factor_applied = *idx;
        idx++;
        rx_data_descriptor_list[i].type = *idx;
        idx++;

        rx_data_descriptor_list[i].signal_name = "";
        for (unsigned int j = 0; j < 16; j++)
            rx_data_descriptor_list[i].signal_name.append(1, static_cast<char>((*(idx+j))));
        idx += 16;

        rx_data_descriptor_list[i].representation = static_cast<unsigned short>(((*idx) << 8) | (*(idx+1)));
        idx += 2;

        int32_t temp_int = (((*idx) << 24) | (*(idx+1) << 16) | (*(idx+2) << 8) | (*(idx+3)));
        float temp = *(float*) &temp_int;
        rx_data_descriptor_list[i].scaling_factor = static_cast<float>(temp);
        idx += 4;

        rx_data_descriptor_list[i].line_width = *idx;
        idx += 1;
        rx_data_descriptor_list[i].alpha = *idx;
        idx += 1;
        rx_data_descriptor_list[i].r = *idx;
        idx += 1;
        rx_data_descriptor_list[i].g = *idx;
        idx += 1;
        rx_data_descriptor_list[i].b = *idx;
        idx += 1;
    }

    for (unsigned int i = 0; i < tx_data_descriptor_list.size(); i++)
    {
        tx_data_descriptor_list[i].idx = *idx;
        idx++;
        tx_data_descriptor_list[i].scaling_factor_applied = *idx;
        idx++;
        tx_data_descriptor_list[i].type = *idx;
        idx++;

        tx_data_descriptor_list[i].signal_name = "";
        for (unsigned int j = 0; j < 16; j++)
            tx_data_descriptor_list[i].signal_name.append(1, static_cast<char>((*(idx+j))));
        idx += 16;

        tx_data_descriptor_list[i].representation = static_cast<unsigned short>(((*idx) << 8) | (*(idx+1)));
        idx += 2;

        int32_t temp_int = (((*idx) << 24) | (*(idx+1) << 16) | (*(idx+2) << 8) | (*(idx+3)));
        float temp = *(float*) &temp_int;
        tx_data_descriptor_list[i].scaling_factor = static_cast<float>(temp);
        idx += 4;

        tx_data_descriptor_list[i].line_width = *idx;
        idx += 1;
        tx_data_descriptor_list[i].alpha = *idx;
        idx += 1;
        tx_data_descriptor_list[i].r = *idx;
        idx += 1;
        tx_data_descriptor_list[i].g = *idx;
        idx += 1;
        tx_data_descriptor_list[i].b = *idx;
        idx += 1;
    }

    //Prepare the decoded_rx_data vector
    decoded_rx_data.resize(n_rx_data);
    prot_status = INITIALIZED;
    first_run = true;

    //Communication is established
    comm_dev_handle->purge_buffers();
    reset_buffers();

    //Send dummy data to be safe that the microcontroller sees the acknowledgement and starts sending data
    unsigned int n_tx_frames_to_be_send = 0;
    if (tx_actu_buff.size() != 0)
        n_tx_frames_to_be_send = comm_dev_handle->get_internal_buffer_size() / tx_actu_buff.size();
    vector<byte> tx_send_buff(n_tx_frames_to_be_send * tx_actu_buff.size());
    for (unsigned int i = 0; i < tx_send_buff.size(); i++)
        tx_send_buff[i] = tx_actu_buff[i % tx_actu_buff.size()];

    comm_dev_handle->receive_all(rx_actu_buff);
    fill(rx_actu_buff.begin(), rx_actu_buff.end(), 0);

    return SUCCESS;
}

int comm_prot::parse_data()
{
    //Search the start sequence of the first frame
    vector<byte> terminator{0xFF,0xFF,0xFF,0xFF, 0xFF, 0xFF, 0xEE};
    vector<byte>::iterator it = search(rx_actu_buff.begin(), rx_actu_buff.end(), terminator.begin(), terminator.end());

    //Check if data frames have been found
    if(it == rx_actu_buff.end())
    {
        if (prot_status == INITIALIZED)     //info frame is sent --> throw no error
            return EXIT_SUCCESS;
        else
            return EXIT_FAILURE;
    }

    if(it != rx_actu_buff.begin())
    {
        //Everything before it belongs to an old frame, so move this to the remaining frame buffer
        rx_remain_buff.insert(rx_remain_buff.end(), rx_actu_buff.begin(), it);
    }

    if((rx_remain_buff.size() > 0) && (!first_run))   //Something is in the rx_remain_buff which needs to be decoded
    {
        //Check for start sequence inside this remaining buff
        vector<byte>::iterator it_remain = search(rx_remain_buff.begin(), rx_remain_buff.end(), terminator.begin(), terminator.end());

        //Check if uncorrupted data is inside the remain_buff. When it is uncorrupted, save it as frame which should be decoded
        if ((rx_remain_buff.size() < buff_dimension) || (rx_remain_buff.size() > (2*buff_dimension)) || (it_remain == rx_remain_buff.end()) || ((it_remain+buff_dimension) > rx_remain_buff.end())) //Something went wrong
        {
            n_rx_errors++;
            n_rx_completetion_errors++;
        }
        else
        {
            vector<byte> temp(it_remain+7, it_remain+buff_dimension);
            rx_decode_buff.push_back(temp);
        }

        //Clear the remaining buffer
        rx_remain_buff.resize(0);
    }

    while((it+buff_dimension) <= rx_actu_buff.end()) //Parse the complete rx_actu_buff until the last complete frame
    {
        //Check if new frame is correctly coming, else search for a new start sequence
        if(!((*it == 0xFF) && (*(it+1) == 0xFF) && (*(it+2) == 0xFF) && (*(it+3) == 0xFF) && (*(it+4) == 0xFF) && (*(it+5) == 0xFF) && (*(it+6) == 0xEE)))
        {
            n_rx_errors++;
            n_rx_corrupted_errors++;

            it = search(it, rx_actu_buff.end(), terminator.begin(), terminator.end());
            if(it == rx_actu_buff.end())
                break;
        }

        //Extract this frame
        vector<byte> temp(it+7, it+buff_dimension);
        rx_decode_buff.push_back(temp);

        //Increment the iterator by frame length
        it = it+buff_dimension;
    }

    //Copy remaining data into remaining buff
    if (it < rx_actu_buff.end())
        rx_remain_buff.insert(rx_remain_buff.end(), it, rx_actu_buff.end());

    //Reset rx_actu_buff
    fill(rx_actu_buff.begin(), rx_actu_buff.end(), 0);

    if (first_run)
        first_run = false;

    prot_status = ESTABLISHED; //parsing was done correctly, connection is established
    return EXIT_SUCCESS;
}

int comm_prot::decode_data()
{
    //Decode every frame (this function can be parallized if needed)
    for (unsigned int i = 0; i < rx_decode_buff.size(); i++)
    {
        //Check which command is in front of a frame
        decoded_cmd.push_back(rx_decode_buff[i][0]);

        //Decode a single frame
        vector<float> temp = decode_frame(rx_decode_buff[i]);

        //If decoding doesn´t return an empty vector, push obtained data into the vector
        if (temp.size() > 0)
        {
            for (unsigned int j = 0; j < n_rx_data; j++)
            {
                 decoded_rx_data[j].push_back(temp[j]);
            }
        }
    }

    //Clear the decoded buffer
    rx_decode_buff.resize(0);

    return EXIT_SUCCESS;
}

vector<float> comm_prot::decode_frame(vector<byte> data_frame)
{
    bool error = false;

    //Create vector which contains timestamp and datas
    vector<float> decoded_frame(n_rx_data);
    unsigned int idx = 2;

    for(unsigned int j = 0; j < n_rx_data; j++)
    {
        switch(rx_data_descriptor_list[j].type)
        {

        case TYPE_UINT8:
        {
            uint8_t temp = data_frame[idx];
            decoded_frame[j] = static_cast<float>(temp);
            if (data_frame[idx+1] != 0xEE)
                error = true;
            idx += 2;
        }
            break;

        case TYPE_INT8:
        {
            int8_t temp = data_frame[idx];
            decoded_frame[j] = static_cast<float>(temp);
            if (data_frame[idx+1] != 0xEE)
                error = true;
            idx += 2;
        }
            break;

        case TYPE_UINT16:
        {
            uint16_t temp = (data_frame[idx] << 8) | data_frame[idx+1];
            decoded_frame[j] = static_cast<float>(temp);
            if (data_frame[idx+2] != 0xEE)
                error = true;
            idx += 3;
        }
            break;

        case TYPE_INT16:
        {
            int16_t temp = (data_frame[idx] << 8) | data_frame[idx+1];
            decoded_frame[j] = static_cast<float>(temp);
            if (data_frame[idx+2] != 0xEE)
                error = true;
            idx += 3;
        }
            break;

        case TYPE_UINT32:
        {
            uint32_t temp = (data_frame[idx] << 24) | (data_frame[idx+1] << 16) | (data_frame[idx+2] << 8) | data_frame[idx+3];
            decoded_frame[j] = static_cast<float>(temp);
            if (data_frame[idx+4] != 0xEE)
                error = true;
            idx += 5;
        }
            break;

        case TYPE_INT32:
        {
            int32_t temp = (data_frame[idx] << 24) | (data_frame[idx+1] << 16) | (data_frame[idx+2] << 8) | data_frame[idx+3];
            decoded_frame[j] = static_cast<float>(temp);
            if (data_frame[idx+4] != 0xEE)
                error = true;
            idx += 5;
        }
            break;

        case TYPE_FLOAT:
        {
            int32_t temp_int = (data_frame[idx] << 24) | (data_frame[idx+1] << 16) | (data_frame[idx+2] << 8) | data_frame[idx+3];
            float temp = *(float*) &temp_int;
            decoded_frame[j] = static_cast<float>(temp);
            if (data_frame[idx+4] != 0xEE)
                error = true;
            idx += 5;
        }
            break;

        }
    }

    if (error == true)
    {
        decoded_frame.resize(0);
        n_rx_errors++;
        n_rx_decode_errors++;
    }

    for(unsigned int j = 0; j < n_rx_data; j++)
    {
        if(rx_data_descriptor_list[j].scaling_factor_applied == SCALING_FACTOR_APPLIED)
            decoded_frame[j] *= rx_data_descriptor_list[j].scaling_factor;
    }

    return decoded_frame;
}

void comm_prot::reset_buffers()
{
    //Fill all buffers with zeros
    fill(rx_actu_buff.begin(), rx_actu_buff.end(), 0);
    fill(tx_actu_buff.begin(), tx_actu_buff.end(), 0);
    rx_remain_buff.resize(0);
    rx_decode_buff.resize(0);

    //Fill transmission buffer with start sequence
    tx_actu_buff[0] = 0xFF;
    tx_actu_buff[1] = 0xFF;
    tx_actu_buff[2] = 0xFF;
    tx_actu_buff[3] = 0xFF;
    tx_actu_buff[4] = 0xFF;
    tx_actu_buff[5] = 0xFF;
    tx_actu_buff[6] = 0xEE;

    n_rx_errors = 0;
    n_rx_completetion_errors = 0;
    n_rx_corrupted_errors = 0;
    n_rx_decode_errors = 0;

    first_run = true;
}

int comm_prot::set_tx_data(vector<int> tx_data)
{
    if (tx_data.size() != n_tx_data)
        return EXIT_FAILURE;

    unsigned int idx = 7;

    for(unsigned int i = 0; i < tx_data_descriptor_list.size(); i++ )
    {
        tx_actu_buff[idx]   = static_cast<byte>(((static_cast<uint32_t>(tx_data[i])) & 0xFF000000) >> 24); //MSB
        tx_actu_buff[idx+1] = static_cast<byte>(((static_cast<uint32_t>(tx_data[i])) & 0x00FF0000) >> 16);
        tx_actu_buff[idx+2] = static_cast<byte>(((static_cast<uint32_t>(tx_data[i])) & 0x0000FF00) >> 8);
        tx_actu_buff[idx+3] = static_cast<byte>(((static_cast<uint32_t>(tx_data[i])) & 0x000000FF));       //LSB

        tx_actu_buff[idx+4] = 0xEE;
        idx += 5;
    }

    return EXIT_SUCCESS;
}

void comm_prot::set_terminal_command(string cmd)
{
    terminal_command = cmd;
    char temp[16];
    std::strcpy(temp, cmd.c_str());

    unsigned int idx = 7+(5*tx_data_descriptor_list.size()); //length of tx stuct with init frame and all the signals

    for (unsigned int i = 0; i < idx+16; i++)
        tx_actu_buff[idx+i] = static_cast<uint8_t>(temp[i]);
}

vector<vector<float>> comm_prot::get_rx_data()
{
    vector<vector<float>> temp = decoded_rx_data;

    for (unsigned int i = 0; i < n_rx_data; i++)
    {
         decoded_rx_data[i].resize(0);
    }

    return temp;
}

vector<uint8_t> comm_prot::get_cmd()
{
    vector<uint8_t> temp = decoded_cmd;

    decoded_cmd.resize(0);

    return temp;
}

unsigned int comm_prot::get_recommended_trigger_time()
{
    unsigned int n_data_trigger = (comm_dev_handle->get_internal_buffer_size() / 2);  //half of the buffer size
    unsigned int n_data_per_second = (buff_dimension*process_freq);

    recommended_trigger_time = 1000 * n_data_trigger / n_data_per_second;
    if (recommended_trigger_time < 1)
        recommended_trigger_time = 1;

    return recommended_trigger_time;
}

comm_prot::error_t comm_prot::comm_manager()
{
    if((prot_status == UNINITIALIZED)||(prot_status == UNCONNECTED))
        return NOT_READY;

    else if ((prot_status == INITIALIZED)||(prot_status == ESTABLISHED))
    {
        //here we need to check if data are available, read the package, parse it and decode it

        //Get number of data
        unsigned int n_bytes_received = comm_dev_handle->get_rx_available_size();

        if (n_bytes_received < buff_dimension)
            return NO_DATA_AVAILABLE;  //not enough data are present to be read

        //Prepare tx buff in such a way that it contains multiple times the tx data to be sure that they reach the microcontroller
        unsigned int n_tx_frames_to_be_send = 0;
        if (tx_actu_buff.size() != 0)
            n_tx_frames_to_be_send = n_bytes_received / tx_actu_buff.size();

        //Put a limit to avoid a long blocking of the send function
        if (n_tx_frames_to_be_send > 20)
            n_tx_frames_to_be_send = 20;

        vector<byte> tx_send_buff(n_tx_frames_to_be_send * tx_actu_buff.size());
        for (unsigned int i = 0; i < tx_send_buff.size(); i++)
            tx_send_buff[i] = tx_actu_buff[i % tx_actu_buff.size()];

        //Send the data
        if (comm_dev_handle->send_buffer(tx_send_buff) != EXIT_SUCCESS)
            return COMM_ERROR;

        //Receive the data
        if (comm_dev_handle->receive_all(rx_actu_buff) != EXIT_SUCCESS)
            return COMM_ERROR;

        //Parse and decode the received data
        if (parse_data() == EXIT_FAILURE)
            return PARSE_ERROR;

        //time4 = timer.nsecsElapsed();

        if (decode_data() == EXIT_FAILURE)
            return DECODE_ERROR;
    }

    return SUCCESS;
}
