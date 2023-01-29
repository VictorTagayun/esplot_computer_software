/**
  *********************************************************************************************************************************************************
  @file     :signal_data.h
  @brief    :Header of the Signal Data Class
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

#ifndef SIGNAL_DATAH
#define SIGNAL_DATAH

#include <QVector>
#include <QString>
#include <QColor>
#include <QElapsedTimer>

#include <math.h>

class Signal_Data
{
public:
    Signal_Data(QString Name, uint32_t Index, int type, float scaling);
    ~Signal_Data();

    void Add_Data(float* data_ptr, int N_data, unsigned int maxData);  //used to add new data to the signal buffer
    void Clean_Data() { signal_data.clear(); data_count = static_cast<uint32_t>(signal_data.size()); }  //cleans the whole signal buffer
    uint32_t Count_Data() {return static_cast<uint32_t>(signal_data.size()); }  //returns the number of data in the signal buffer

    QString get_Name() { return name; }
    uint32_t get_Index() { return index; }

    void set_Record(bool rec) { record = rec; }

    float* retrieve_Data_Pointer() { return signal_data.data(); }
    float getLastSample();

private:
    QString name;   //name of the signal
    uint32_t index;  //ID of the signal

    bool record;  //if record is true, new data are added to the old ones, if record is false then old data are cleared before accepting new ones

    int sig_type;
    float scaling_factor;

    std::vector<float> signal_data;  //includes all the datas of the signal  ==> signal buffer
    uint32_t data_count;  //number of data contained in the signal buffer

    float abs_float(float value);
};

#endif // SIGNAL_H
