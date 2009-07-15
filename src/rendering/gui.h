/*
  Copyright 2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef GUI_H_49ZN3I4A
#define GUI_H_49ZN3I4A

#include "io-manager.h"


//    typedef void (*OObuttonhandlerfunc)(bool buttonUp, void *data);

typedef struct {
    OObuttonhandlerfunc mouseDown;
    OObuttonhandlerfunc mouseUp;
    void *data;
    float x, y; // x, y (origin is top left corner)
    float w, h; // width, height
    OOtexture *tex;
} OObutton;

typedef struct {
    OObuttonhandlerfunc click;
    void *data;
    char *str;
    OOtexture *tex;
    float u, v, du, dv;
} OOmenu;


#endif /* end of include guard: GUI_H_49ZN3I4A */
