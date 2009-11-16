/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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

/*!
    Master include header for plugins.
*/
 
#ifndef OPENORBIT_H__
#define OPENORBIT_H__

#include <stdlib.h>
#include <openorbit/plugin.h>

typedef void OOobject;

//typedef struct {
//    size_t asize; //!< Size of elems array in objects
//    size_t length; //!< Usage of elems array in objects
//    OOobject **elems;
//} OOobjvector;

//void ooObjVecInit(OOobjvector *vec);
//void ooObjVecCompress(OOobjvector *vec);
//void ooObjVecPush(OOobjvector *vec, OOobject *obj);
//OOobject* ooObjVecPop(OOobjvector *vec);

/*!
 * Reads out the i:th element in vec
 *
 * The function should not be used in normal cases as it introduce a high
 * overhead, however it should be used from scripts as it ensure that the
 * index is within bounds.
 */
//OOobject* ooObjVecGet(OOobjvector *vec, size_t i);
//void ooObjVecSet(OOobjvector *vec, size_t i, OOobject *obj);

#define FOR_EACH(I, VEC) for (size_t I = 0 ; I < VEC.length ; I ++)

#include <gencds/object-manager.h>
#include <gencds/hashtable.h>
#include <gencds/list.h>
#include <gencds/heap.h>
#include <gencds/array.h>

//#include <openorbit/plugin-handler.h>

#endif /* ! OPENORBIT_H__ */
 