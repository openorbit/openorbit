/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#include "cmod.h"
#include <stdio.h>
#include <string.h>

static void
load_material(FILE *fp)
{
  double f0, f1, f2;
  int i0;
  char line[256];
  char str[128];

  while (fgets(line, 255, fp) == 0) {
    if (sscanf(line, "diffuse %lf %lf %lf", &f0, &f1, &f2) == 3) {
      
    } else if (sscanf(line, "specular %lf %lf %lf", &f0, &f1, &f2) == 3) {
      
    } else if (sscanf(line, "specpower %d", &i0) == 1) {
      
    } else if (sscanf(line, "texture%d \"%127s\"", &i0, str) == 2) {
      
    } else if (memcmp(line, "end_material", 12) == 0) { // Issue distinguishing between error and 0
      
    } else {
      // Unknown line
    }  
  }

  if (feof(fp)) {
  
  }
}

typedef enum VertType {
  Vert_F3 = 0,
  Pos_F3,
} VertType;

static void
load_mesh(FILE *fp)
{
  double f0, f1, f2, f3, f4, f5;
  int i0;
  char line[256];
  int tristrip_id, count;
  while (fgets(line, 255, fp) == 0) {
    if (!memcmp(line, "vertexdesc", 10)) {
      //  "position %s" // These should determine the vertex parser
      //  "normal %s"
      //  "end_vertexdesc"
    } else if (sscanf(line, "vertices %d", &count) == 1) {
      for (int i = 0 ; i < count ; ++ i) {
        if (fscanf(fp, "%lf %lf %lf %lf %lf %lf", &f0, &f1, &f2, &f3, &f4, &f5) == 6) {
          
        } else {
          // invalid vertex found
        }
      }
    } else if (sscanf(line, "tristrip %d %d", &tristrip_id, &count) == 2) {
      for (int i = 0 ; i < count ; ++ i) {
        if (fscanf(fp, "%d", &i0) == 1) {
        
        } else {
          // invalid syntax, missing tristrip num
        } 
      }
    } else if (memcmp(line, "end_mesh", 8) == 0) {
    
    }
  }
}
model_t*
cmod_load(const char *path)
{
  FILE *fp = fopen(path, "r");
  if (!fp) {
    return NULL;
  }
  /* Read header */
  char celheader[18];
  if (fgets(celheader, 17, fp) == 0) {
    if (memcmp(celheader, "#celmodel__ascii\n", 16)) {
      fclose(fp);
      return NULL;
    }
  }

  char line[256];
  // We have a valid header, parse rest of the file
  while (fgets(line, 255, fp) == 0) {
    if (!memcmp(line, "mesh", 4)) {
      load_material(fp);
    } else if (!memcmp(line, "material", 8)) {
      load_mesh(fp);
    }
  }

  if (feof(fp)) {
  
  } else if (ferror(fp)) {
  
  }
}
