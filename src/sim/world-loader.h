/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SIM_WORLD_LOADER_H
#define SIM_WORLD_LOADER_H


#include "physics/physics.h"
#include "rendering/scenegraph.h"
/*!
 Loads an hrml description of a solar system and builds a solar system graph
 it also connects the physics system to the graphics system.

 This function does not belong in the physics system, but will be here for
 now beeing.
 */
pl_world_t* ooOrbitLoad(sg_scene_t *sc, const char *fileName);



#endif /* !SIM_WORLD_LOADER_H */
