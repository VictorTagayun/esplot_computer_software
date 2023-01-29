/**
  *********************************************************************************************************************************************************
  @file     :XYDot.vsh
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
layout(location = 0) in float x_vertex;
layout(location = 1) in float y_vertex;
uniform highp vec2 conv_fact;
uniform highp vec4 in_color;
uniform highp vec3 in_params;
uniform highp float pastValueGain;
uniform highp int n_samples;
uniform highp int instance;
uniform highp mat4 transfMatrix;

out vec4 frag_color;

//in_params = LW, Y_Axis_Pos, X_Axis_Pos

void main() {
   float x_conv_fact;
   float y_conv_fact;
   float x;
   float y;
   int position;
   float k;
   float alfa;
   vec4 col;

   position = gl_VertexID - (instance * n_samples);
   position = n_samples - position;
   k = log(pastValueGain) / n_samples * -1.0;
   alfa = exp(k * position * (-1));
   col = in_color;
   col.a = alfa;

   x_conv_fact = conv_fact.x;
   y_conv_fact = conv_fact.y;

   x = (x_vertex * x_conv_fact) + in_params.y;
   y = (y_vertex * y_conv_fact) + in_params.z;

   gl_Position = transfMatrix * vec4(vec3(x, y, 1.0), 1.0);

   gl_Position.z = 0.0;
   gl_PointSize = in_params[0] * 2.0;  //depends on the LineWidth of the signal => 5 times the line width so that it gets visible when overlapped with the line segment

   frag_color = col;
}
