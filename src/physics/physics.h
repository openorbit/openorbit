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

#ifndef PHYSICS_H_MW8CHG7V
#define PHYSICS_H_MW8CHG7V
#include <ode/ode.h>
#include "rendering/scenegraph.h"

typedef struct PLorbsys PLorbsys;

typedef struct PLobject {
    char *name;
    dBodyID id;
    float m;
    OOdrawable *drawable; //!< Link to scenegraph drawable object representing this
                          //!< object.
    PLorbsys *sys;
} PLobject;


#endif /* end of include guard: PHYSICS_H_MW8CHG7V */
