/*
  Copyright 2006,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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
    \file heap
    \brief   Functions and structures for working with a generic heap type.

    The heap can be used as an priority queue, this was actually the reason it
    was implemented, in order to be used for keeping track on events that are
    to be triggered on some time in the future. THe heap structure is designed
    to be thread safe by default. This mean that when modifying the heap with
    an insert or remove, the heap will be locked and prevent other operations
    on it.
*/


#ifndef __HEAP_H__
#define __HEAP_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#define THREAD_SAFE_HEAP 1

#if THREAD_SAFE_HEAP
#include <pthread.h>
#endif

typedef int64_t (*compute_rank_f)(void *);

typedef struct {
    int64_t rank;
    void *data;
} heap_element_t;

typedef struct {
    compute_rank_f compute_rank;
    int last;
    size_t size;
    heap_element_t *elements;
#if THREAD_SAFE_HEAP
    pthread_mutex_t lock;
#endif
} heap_t;

heap_t *heap_new(size_t n_levels, compute_rank_f f);
void heap_delete(heap_t *h);
bool heap_insert(heap_t *h, void *data);
void *heap_remove(heap_t *h);
void *heap_peek(heap_t *h);
int64_t heap_peek_rank(heap_t *h);
void heap_lock(heap_t *h);
void heap_unlock(heap_t *h);

#ifdef __cplusplus
}
#endif

#endif
