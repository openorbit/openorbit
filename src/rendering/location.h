/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>
 
 This file is part of Open Orbit.
 
 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef orbit_location_h
#define orbit_location_h

/*!
  OpenGL 3.3 location ids, these will be used whenever
  GL_ARB_explicit_attrib_location is well supported on the Mac.
 */
typedef enum {
  SG_MODELVIEW_LOC = 0,
  SG_PROJECTION_LOC,
  SG_VERTEX_LOC,
  SG_NORMAL_LOC,

  SG_TEX0_COORD_LOC,
  SG_TEX1_COORD_LOC,
  SG_TEX2_COORD_LOC,
  SG_TEX3_COORD_LOC,

  SG_TEX0_LOC,
  SG_TEX1_LOC,
  SG_TEX2_LOC,
  SG_TEX3_LOC,
  SG_LIGHT0_LOC,
  SG_LIGHT1_LOC,
  SG_LIGHT2_LOC,
  SG_LIGHT3_LOC
} SGlocation;

/*!
  In the meanwhile, without GL_ARB_explicit_attrib_location, we use these
  symbols with the glGetAttribLocation() functions to determine location.
 */
#define SG_MODELVIEW_NAME "ModelViewMatrix"
#define SG_PROJECTION_NAME "ProjectionMatrix"

#define SG_VERTEX_NAME "Position"
#define SG_NORMAL_NAME "Normal"
#define SG_COLOUR_NAME "Colour"

#define SG_TEX0_COORD_NAME "Tex0Coord"
#define SG_TEX1_COORD_NAME "Tex1Coord"
#define SG_TEX2_COORD_NAME "Tex2Coord"
#define SG_TEX3_COORD_NAME "Tex3Coord"

#define SG_TEX0_NAME "Tex0"
#define SG_TEX1_NAME "Tex1"
#define SG_TEX2_NAME "Tex2"
#define SG_TEX3_NAME "Tex3"

#define SG_LIGHT0_NAME "Light0"
#define SG_LIGHT1_NAME "Light1"
#define SG_LIGHT2_NAME "Light2"
#define SG_LIGHT3_NAME "Light3"

#endif /* !orbit_location_h */
