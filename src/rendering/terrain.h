/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef TERRAIN_H_BQIC4P63
#define TERRAIN_H_BQIC4P63

/*!
    The terrain system is engineered in order to be able to draw the realistic surfaces of planets
    It consist of the following:
      Terrain file descriptor loader
      Paging terrain system
      LOD (based on terrain tiles)  (Sort of geomipmapped)
      Renderer
    The system supports the rendering of large terrains (i.e. planets),
    there is a support system for constant LODing, where a LOD hierarchy is pre generated.
*/

typedef struct SGterrain SGterrain;

SGterrain* sgTerrainLoad(const char *file);
void sgTerrainDraw(SGterrain *terrain);

#endif /* end of include guard: TERRAIN_H_BQIC4P63 */
