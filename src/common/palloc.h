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

#ifndef COMMON_POOL_H
#define COMMON_POOL_H

#include <stddef.h>

typedef struct bump_pool_t bump_pool_t;

/*!
  Initialise a new bump allocation pool on the heap

  \param obj_count Number of objects that fits in the pool
  \param obj_size Size of the objects in the pool
 */
bump_pool_t* bump_init(size_t obj_count, size_t obj_size);

/*!
  Allocate count objects from the bump allocation pool
  \param pool The pool
  \param count Number of objects to be allocated
  \return Pointer to object array of size count, if there is no more space in
          the pool the result will be NULL.
 */
void* bump_alloc(bump_pool_t *pool, size_t count);

/*!
  Frees all objects and the pool descriptor

  \param pool The pool
 */
void bump_free(bump_pool_t *pool);
/*!
 Frees all objects in the pool but does not free the pool descriptor, the pool
 descriptor is ready to be reused.

 \param pool The pool
 */
void bump_clear(bump_pool_t *pool);


typedef struct pool_t pool_t;

pool_t* pool_create(size_t obj_size);
void* pool_alloc(pool_t *pool);
void pool_free(void *obj);

/*!
  Safe malloc. Either returns the requested size or calls abort if out of memory
 */
void* smalloc(size_t sz) __attribute__((malloc));
/*!
 Safe calloc. Either returns the requested objects or calls abort if out of memory
 */
void* scalloc(size_t count, size_t sz) __attribute__((malloc));

#endif /* !COMMON_POOL_H */
