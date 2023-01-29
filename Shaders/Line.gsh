/**
  *********************************************************************************************************************************************************
  @file     :Line.gsh
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
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
in vec4 geom_color[];
in vec3 geom_params[];

uniform highp vec2 screen_size;

out vec4 frag_color;

void main(){
   float alfa, deltax, deltay, LWX, LWY;
   float screen_width = screen_size.x;
   float screen_height = screen_size.y;
   LWX = geom_params[0].x / 2.0 / screen_width;
   LWY = geom_params[0].x / 2.0 / screen_height;
   deltax = gl_in[1].gl_Position.x - gl_in[0].gl_Position.x;
   deltay = gl_in[1].gl_Position.y - gl_in[0].gl_Position.y;
   if (deltax == 0)
      alfa = 0;
   else
      alfa = 1.5707963 - atan(deltay, deltax);
   gl_Position = vec4(gl_in[0].gl_Position.x - LWX * cos(alfa), gl_in[0].gl_Position.y + LWY * sin(alfa), 0.0, 1.0);
   frag_color = geom_color[0];
   EmitVertex();
   gl_Position = vec4(gl_in[0].gl_Position.x + LWX * cos(alfa), gl_in[0].gl_Position.y - LWY * sin(alfa), 0.0, 1.0);
   frag_color = geom_color[0];
   EmitVertex();
   gl_Position = vec4(gl_in[1].gl_Position.x - LWX * cos(alfa), gl_in[1].gl_Position.y + LWY * sin(alfa), 0.0, 1.0);
   frag_color = geom_color[1];
   EmitVertex();
   gl_Position = vec4(gl_in[1].gl_Position.x + LWX * cos(alfa), gl_in[1].gl_Position.y - LWY * sin(alfa), 0.0, 1.0);
   frag_color = geom_color[1];
   EmitVertex();
   EndPrimitive();
}
