/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "terrain.h"
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>

typedef struct SGterrainloddata {
  size_t vertexCount;
  float *vertices;
  float *texCoords;
  float *normals;
} SGterrainloddata;

typedef struct SGterrainchunk {
  size_t lodLevels;
  SGterrainloddata **lodData;
} SGterrainchunk;

struct SGterrain {
  size_t width;
  size_t height;
  SGterrainchunk ***chunks;
};

/*
  Loads a terrain object from file the terrain consist of a meta-document
  detailing the terrain structure and the actual terrain directory.
  
  terraindir/
    metadata.hrml
    chunks/
      0N0E+90N90E/
        lowres.png
 */
SGterrain*
sgTerrainLoad(const char *file)
{
  assert(file != NULL && "Not NULL");
}

void
sgTerrainDrawChunk(SGterrainchunk *chunk)
{
  assert(chunk != NULL);
  
  // Determine which lod level to draw this at
  
  // Draw the lod chunk
}


void
sgTerrainDraw(SGterrain *terrain)
{
  assert(terrain != NULL && "Not NULL");
  
  for (size_t i = 0; i < terrain->height; ++ i) {
    for (size_t j = 0; i < terrain->width; ++ j) {
      sgTerrainDrawChunk(terrain->chunks[i][j]);
    }
  }
}
