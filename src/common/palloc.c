/*
 Copyright 2009,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include "common/bitutils.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <assert.h>

struct bump_pool_t {
  size_t obj_len;
  void *data_start;
  void *data_last;
  void *free_pointer;
};

bump_pool_t*
bump_init(size_t obj_count, size_t obj_size)
{
  bump_pool_t *pool = malloc(sizeof(bump_pool_t));

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
bump_alloc(bump_pool_t *pool, size_t count)
{
  if (pool->free_pointer + (count-1) * pool->obj_len > pool->data_last) {
    return NULL;
  }

  void *data = pool->free_pointer;
  pool->free_pointer = pool->free_pointer + count * pool->obj_len;

  return data;
}

void
bump_free(bump_pool_t *pool)
{
  free(pool->data_start);

  pool->data_start = NULL;
  pool->free_pointer = NULL;
  pool->data_last = NULL;

  free(pool);
}

void
bump_clear(bump_pool_t *pool)
{
  pool->free_pointer = pool->data_start;
}


struct pool_t {
  pthread_mutex_t lock;
  size_t obj_size;
  void *free_pointer;
};

typedef struct pool_obj_t {
  union {
    pool_t *pool;
    struct pool_obj_t *next;
  } u;
  uint8_t data[] __attribute__ ((aligned (16)));
} pool_obj_t;

pool_t*
pool_create(size_t obj_size)
{
  assert(obj_size <= 4096);
  assert(8 <= obj_size);
  pool_t *pool = malloc(sizeof(pool_t));
  assert(pool);

  pool->obj_size = clp2_32(obj_size);
  pool->free_pointer = mmap(NULL, 8*4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, 0, 0);

  pool_obj_t *pool_obj = pool->free_pointer;

  size_t end = (8*4096-1) / (sizeof (pool_obj_t) + pool->obj_size);
  for (size_t i = 0 ; i < end ; i ++) {
    uint8_t *ptr = (((uint8_t*)pool_obj) + sizeof(pool_obj_t) + pool->obj_size);
    pool_obj->u.next = (pool_obj_t*) ptr;

    pool_obj = pool_obj->u.next;
  }

  pool_obj->u.next = NULL;

  pthread_mutex_init(&pool->lock, NULL);

  return pool;
}

void*
pool_alloc(pool_t *pool)
{
  pthread_mutex_lock(&pool->lock);
  if (pool->free_pointer == NULL) {
    pool->free_pointer = mmap(NULL, 8*4096, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, 0, 0);

    pool_obj_t *pool_obj = pool->free_pointer;

    for (size_t i = 0 ; i < (8*4096-1) / (sizeof (pool_obj_t) + pool->obj_size) ; i++) {
      pool_obj->u.next = (pool_obj_t*) (((uint8_t*)pool_obj) + sizeof(pool_obj_t) + pool->obj_size);
      pool_obj = pool_obj->u.next;
    }

    pool_obj->u.next = NULL;
  }

  pool_obj_t *obj = pool->free_pointer;
  pool->free_pointer = obj->u.next;
  obj->u.pool = pool;

  pthread_mutex_unlock(&pool->lock);

  assert((((uintptr_t)&obj->data & 4) == 0) && "invalid pointer allocated");
  return &obj->data;
}

void
pool_free(void *obj)
{
  pool_obj_t *pool_obj = obj - offsetof(pool_obj_t, data);
  pool_t *pool = pool_obj->u.pool;

  pthread_mutex_lock(&pool->lock);

  pool_obj->u.next = pool->free_pointer;
  pool->free_pointer = pool_obj;

  pthread_mutex_unlock(&pool->lock);
}

void*
smalloc(size_t sz)
{
  void *data = malloc(sz);

  if (data == NULL) abort();

  return data;
}


