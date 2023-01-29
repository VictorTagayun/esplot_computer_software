/**
  *********************************************************************************************************************************************************
  @file     :signal_data.cpp
  @brief    :Functions of the Signal Data Class
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

#include "signal_data.h"
#include <QDebug>

Signal_Data::Signal_Data(QString Name, uint32_t Index, int type, float scaling)
{
    name = Name;
    index = Index;
    sig_type = type;
    scaling_factor = scaling;

    data_count = static_cast<uint32_t>(signal_data.size());
}

Signal_Data::~Signal_Data()
{
    signal_data.clear();
}

void Signal_Data::Add_Data(float *data_ptr, int N_data, unsigned int maxData)
{
    unsigned long long N, old_N;
    unsigned long long size = sizeof(float);

    //if N_data is <= 0 we exit
    if (N_data <= 0)
        return;

    old_N = signal_data.size();
    //we resize with the new dimension
    signal_data.resize(old_N + N_data);

    memcpy(signal_data.data() + old_N, data_ptr, N_data * size);

    if (record == false)
    {
        //if the new data overcome the maxData allowed, we cut
        N = signal_data.size();
        if (N > maxData)
            signal_data.erase(signal_data.begin(), signal_data.begin() + static_cast<int>((N - maxData)));
    }


/*  THIS IS OLD CODE. IT'S RELIABLE BUT MUCH SLOWER THAN THE CODE ABOVE. I KEEP IT JUST IN CASE INSTABILITY ARISES AND WE WANT TO RESTORE THE OLD METHOD
    if (N_data > 0)
        for (i = 0; i < N_data; i++)
            signal_data.push_back(data_ptr[i]);

    if (record == false)
    {
        N = static_cast<unsigned int>(signal_data.size());
        if (N > maxData)
            signal_data.erase(signal_data.begin(), signal_data.begin() + static_cast<int>((N - maxData)));
    }
*/
    data_count = static_cast<uint32_t>(signal_data.size());
}

float Signal_Data::getLastSample()
{
    if (data_count > 0)
        return signal_data.data()[data_count - 1];
    else
        return 0.0;
}

float Signal_Data::abs_float(float value)
{
    if (value < 0)
        return value * (-1);
    else
        return value;
}
