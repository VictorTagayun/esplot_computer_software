/**
  *********************************************************************************************************************************************************
  @file     :matlabfilesaver.cpp
  @brief    :Functions for Matlab File Saver class
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

#include "matlabfilesaver.h"

MatlabFileSaver::MatlabFileSaver()
{
    MATFile_Version = 5;
    file_header.resize(128);  //header is 128 bytes
    N_signals = 0;
    N_samples = 0;
    descriptors.resize(maxNSignals);
}

int MatlabFileSaver::AddSignalInfoToWrite(QString desc)
{
    int id;

    if (N_signals == maxNSignals)  //it's not possible to add any more signals
        return -1;

    id = N_signals;
    desc = desc.replace(" ", "_");
    if ((desc.length() % 8) != 0)
        desc = desc.leftJustified(((desc.length() / 8) + 1) * 8, '\0');
    descriptors[id] = desc;
    N_signals++;

    return id;
}

QString MatlabFileSaver::getDescriptor(int idx)
{
    if ((idx < 0) || (idx >= N_signals))
        return "Not Found";

    return descriptors[idx];
}

void MatlabFileSaver::Create_File_Header()
{
    int i, n_char;
    QByteArray head_st;
    QString head_string;

    head_string = "MATLAB " + QString::number(MATFile_Version) + ".0 MAT-file, Platform: PCWIN64, Created on: " + QDate::currentDate().toString("dd.MM.yyyy");
    n_char = head_string.length();
    head_st.resize(n_char);
    head_st = head_string.toLocal8Bit();
    for (i = 0; i < n_char; i++)
        file_header[i] = head_st[i];

    for (i = n_char; i < 116; i++)
        file_header[i] = 32;  //introduces a space

    for (i = 116; i < 124; i++)
        file_header[i] = 0;  //writes a 0

    file_header[124] = 0x00; file_header[125] = 0x01; file_header[126] = 0x49; file_header[127] = 0x4D;

    //Header is ready
}

int MatlabFileSaver::Save_MATLAB_File(float **data_ptr, int length, QString filename)
{
    int i, j, size;
    const int arrayflagssize = 16;
    const int dimensionssize = 16;
    const int arraynametagsize = 8;
    const int datatagsize = 8;
    QByteArray desc;

    if (length <= 0)  //no samples to be saved
        return -1;
    N_samples = length;

    Create_File_Header();

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return -1;

    out = new QDataStream(&file);  //ready to write

    for (i = 0; i < 128; i++)
        *out << static_cast<uint8_t>(file_header[i]);


    out->setByteOrder(QDataStream::LittleEndian);

    for (i = 0; i < N_signals; i++)
    {
        //Writes the TAG => miMATRIX = 14 (0x0E) and SIZE (has to be calculated)
        size = arrayflagssize + dimensionssize + arraynametagsize + descriptors[i].length() + datatagsize + (8 * N_samples);
        *out << static_cast<uint32_t>(0x0000000E) << static_cast<uint32_t>(size);

        //Writes the Array Flags : (TAG) miUINT32 = 6 (0x06) and SIZE = 8  (DATA) 0x0006 (mxDOUBLE_CLASS) and 0x00000000 (Undefined)
        *out << static_cast<uint32_t>(0x00000006) << static_cast<uint32_t>(0x00000008);
        *out << static_cast<uint32_t>(0x00000006) << static_cast<uint32_t>(0x00000000);

        //Writes the Dimensions Array : (TAG) miINT32 = 5 (0x00000005) and SIZE = 8; DATA = 1 and 1
        *out << static_cast<uint32_t>(0x00000005) << static_cast<uint32_t>(0x00000008);
        *out << static_cast<uint32_t>(0x00000001) << static_cast<uint32_t>(N_samples);

        //Writes the Array Name : (TAG) miINT8 = 1, SIZE = descriptor.length; DATA = the string itself
        *out << static_cast<uint32_t>(0x00000001) << static_cast<uint32_t>(descriptors[i].length());
        desc = descriptors[i].toLocal8Bit();
        for (j = 0; j < descriptors[i].length(); j++)
            *out << static_cast<uint8_t>(desc[j]);

        //Writes the Values : (TAG) miDOUBLE = 9, SIZE = N_samples * 8;
        *out << static_cast<uint32_t>(0x00000009) << static_cast<uint32_t>(N_samples * 8);
        for (j = 0; j < N_samples; j++)
            *out << static_cast<double>(data_ptr[i][j]);
    }

    delete  out;

    file.close();

    return 0;
}
