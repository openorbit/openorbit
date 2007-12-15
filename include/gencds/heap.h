/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
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

#define THREAD_SAFE_HEAP 1

#if THREAD_SAFE_HEAP
#include <pthread.h>
#endif

typedef unsigned int (*compute_rank_f)(void *);

typedef struct {
    unsigned int rank;
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
unsigned int heap_peek_rank(heap_t *h);
void heap_lock(heap_t *h);
void heap_unlock(heap_t *h);

#ifdef __cplusplus
}
#endif
    
#endif
