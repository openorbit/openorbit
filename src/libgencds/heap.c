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


#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <gencds/heap.h>


heap_t
*heap_new(size_t n_levels, compute_rank_f f)
{
    heap_t *h;

    h = malloc(sizeof(heap_t));
    if (! h) return NULL;

    h->elements = calloc(1 << n_levels, sizeof(heap_element_t));
    if (! h->elements) {
        free(h);
        return NULL;
    }

    h->size = (1 << n_levels) - 1;
    h->last = 0;
    h->compute_rank = f;

#if THREAD_SAFE_HEAP
    if (pthread_mutex_init(&h->lock, NULL)) {
        free(h->elements);
        free(h);
    }
#endif

    return h;
}

void
heap_delete(heap_t *h)
{
    assert(h);
#if THREAD_SAFE_HEAP
    pthread_mutex_destroy(&h->lock);
#endif

    free(h->elements);
    free(h);
}

bool
heap_insert(heap_t *h, void *data)
{
    assert(h);

#if THREAD_SAFE_HEAP
    if (pthread_mutex_lock(&h->lock)) return false;
#endif

    if (h->last < h->size - 1) {
        h->elements[h->last + 1].rank = h->compute_rank(data);
        h->elements[h->last + 1].data = data;
        h->last ++;
        // bubble up
        int elem_n = h->last;
        int parent_n = elem_n >> 1;
        while (elem_n > 1) {
            if (h->elements[elem_n].rank < h->elements[parent_n].rank) {
                heap_element_t tmp = h->elements[elem_n];
                h->elements[elem_n] = h->elements[parent_n];
                h->elements[parent_n] = tmp;
                elem_n = parent_n;
                parent_n = elem_n >> 1;
            } else {
                // cannot bubble up any more, heap order is satisfied
                break;
            }
        }

#if THREAD_SAFE_HEAP
        pthread_mutex_unlock(&h->lock);
#endif

        return true;
    }


#if THREAD_SAFE_HEAP
    pthread_mutex_unlock(&h->lock);
#endif

    return false;
}

void
*heap_remove(heap_t *h)
{
    assert(h);

#if THREAD_SAFE_HEAP
    if (pthread_mutex_lock(&h->lock)) return NULL;
#endif


    if (h->last >= 1) {
        void *data = h->elements[1].data;
        h->elements[1] = h->elements[h->last];
        h->last --;
        // bubble down
        int elem_n = 1;

        while (elem_n < h->last) {
            int left_child_n = elem_n << 1;
            int right_child_n = (elem_n << 1) + 1;
            int correct_child_n;

            if (right_child_n <= h->last) {
                correct_child_n = (h->elements[left_child_n].rank < h->elements[right_child_n].rank)    ? left_child_n
                                                                                                        : right_child_n;
            } else if (left_child_n <= h->last) {
                correct_child_n = left_child_n;
            } else {
                // at bottom, cannot bubble down any more
                break;
            }

            // compare parent and child ranks and bubble down parent if child rank is smaller
            if (h->elements[elem_n].rank > h->elements[correct_child_n].rank) {
                heap_element_t tmp = h->elements[elem_n];
                h->elements[elem_n] = h->elements[correct_child_n];
                h->elements[correct_child_n] = tmp;
                elem_n = correct_child_n;
            } else {
                // cannot bubble down any more, heap order satisfied
                break;
            }
        }

#if THREAD_SAFE_HEAP
        pthread_mutex_unlock(&h->lock);
#endif

        return data;
    }

#if THREAD_SAFE_HEAP
    pthread_mutex_unlock(&h->lock);
#endif

    return NULL;
}

void
*heap_peek(heap_t *h)
{
    assert(h);
    if (h->last) return h->elements[1].data;
    else return NULL;
}

unsigned int
heap_peek_rank(heap_t *h)
{
    assert(h);
    if (h->last) return h->elements[1].rank;
    else return 0xffffffff;
}

void
heap_lock(heap_t *h)
{
#if THREAD_SAFE_HEAP
    pthread_mutex_lock(&h->lock);
#endif
}
void
heap_unlock(heap_t *h)
{
#if THREAD_SAFE_HEAP
    pthread_mutex_unlock(&h->lock);
#endif
}
