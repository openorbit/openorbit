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

#include "palloc.h"
#include <stddef.h>
#include <stdlib.h>
struct pool_t {
  size_t obj_len;
  void *data_start;
  void *data_last;
  void *free_pointer;
};

pool_t*
pinit(size_t obj_count, size_t obj_size)
{
  pool_t *pool = malloc(sizeof(pool_t));

  if (pool == NULL) {
    return NULL;
  }

  pool->data_start = calloc(obj_count, obj_size);

  if (pool->data_start == NULL) {
    free(pool);
    return NULL;
  }

  pool->obj_len = obj_size;
  pool->free_pointer = pool->data_start;
  pool->data_last = pool->data_start + obj_count * obj_size - obj_size;

  return pool;
}

void*
palloc(pool_t *pool, size_t count)
{
  if (pool->free_pointer + (count-1) * pool->obj_len > pool->data_last) {
    return NULL;
  }

  void *data = pool->free_pointer;
  pool->free_pointer = pool->free_pointer + count * pool->obj_len;

  return data;
}

void
pfree(pool_t *pool)
{
  free(pool->data_start);

  pool->data_start = NULL;
  pool->free_pointer = NULL;
  pool->data_last = NULL;

  free(pool);
}

void
pclear(pool_t *pool)
{
  pool->free_pointer = pool->data_start;
}
