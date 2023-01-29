/**
  *********************************************************************************************************************************************************
  @file     :LineVertexID.vsh
  @brief    :Shader file
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

#version 330
layout(location = 0) in float in_vertex;
uniform highp vec4 in_color;
uniform highp vec4 in_params;
uniform highp mat4 transfMatrix;

uniform int n_points;
uniform int instance;
uniform float pastValueGain;

out vec4 geom_color;
out vec4 geom_params;

void main() {
   float conv_fact;
   int position;
   float x_pos;
   float k;
   float alfa;
   float alfa_orig;
   vec4 col;

   alfa_orig = in_color.a;
   conv_fact = in_params.y;

   position = gl_VertexID - (instance * n_points);

   x_pos = -0.95 + (in_params.w * position);

   k = log(pastValueGain) / n_points * -1.0;
   alfa = exp(k * (n_points - position)* (-1));
   col = in_color;
   col.a = alfa * alfa_orig;

   gl_Position = transfMatrix * vec4(vec3(x_pos, (in_vertex * conv_fact) + in_params.z, 1.0), 1.0);

   gl_Position.z = 0.0;
   geom_color = col;
   geom_params = in_params;
}
