/*
  Copyright 2006, 2010 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit. Open Orbit is free software: you can
  redistribute it and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

  Some files of Open Orbit have relaxed licensing conditions. This file is
  licenced under the 2-clause BSD licence.

  Redistribution and use of this file in source and binary forms, with or
  without modification, are permitted provided that the following conditions are
  met:
 
    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
