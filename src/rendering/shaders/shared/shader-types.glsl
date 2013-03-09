/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit.

 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
  This file is a kind of bridge between GLSL and C. It defines the types used
  for uniforms (for example light parameters).

  The locations are queried using functions in shader-manager.c and location.c.
  The shader loader code will preprocess the shaders and replace #include
  directives with the content of this file.
 */

struct SGMaterial {
  vec4 emission;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

struct SGLight {
  vec3 pos;
  vec4 ambient;
  vec4 specular;
  vec4 diffuse;
  vec3 dir; // Only used for spotlights

  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;

  vec4 globAmbient;
};

