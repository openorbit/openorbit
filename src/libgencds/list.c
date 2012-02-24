/*
 Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#include <stdlib.h>
#include <assert.h>

#include <gencds/list.h>
#include "list-private.h"

list_t*
list_new(void)
{
    list_t *the_list = malloc(sizeof(list_t));
    
    if (the_list == NULL) {
        return NULL;
    }
    
    the_list->first = NULL;
    the_list->last = NULL;
    the_list->entry_count = 0;
    
    return the_list;
}


void
list_delete(list_t *list)
{
    assert(list != NULL);
    
    list_entry_t *e = list_first(list);
    while (e) {
        list_entry_t *p = e;
        e = list_entry_next(e);
        free(p);
    }
    
    free(list);
}

list_entry_t*
list_first(const list_t *list)
{
    assert(list != NULL);
    return list->first;
}

list_entry_t*
list_last(const list_t *list)
{
    assert(list != NULL);
    return list->last;
}

// TODO: list_rank is really slow (O(n)), should we really have this at all?
list_entry_t*
list_rank(const list_t *list, unsigned int rank)
{
    assert(list != NULL);
    assert(rank < (list->entry_count));
    
    list_entry_t *entry = list->first;
    
    for (int i = 0 ; i < rank ; i ++) {
        entry = entry->next;
    }
    
    return entry;
}

list_entry_t*
list_entry_next(const list_entry_t *entry)
{
    assert(entry != NULL);

    return entry->next;
}

list_entry_t*
list_entry_previous(const list_entry_t *entry)
{
    assert(entry != NULL);

    return entry->previous;
}

void*
list_entry_data(const list_entry_t *entry)
{
    assert(entry != NULL);
    return entry->data;
}

// TODO: Handle malloc errors
void
list_append(list_t *list, void *obj)
{
    assert(list != NULL);

    list_entry_t *last  = list->last;
    if (last) {
        last->next = malloc(sizeof(list_entry_t));
        last->next->previous = last;
        last->next->next = NULL;
        
        list->last = last->next;
    } else {
        list->first = malloc(sizeof(list_entry_t));
        list->last = list->first;
        
        list->first->previous = NULL;
        list->first->next = NULL;
    }
    
    list->last->data = obj;
    list->entry_count ++;
}

void
list_insert(list_t *list, void *obj)
{
    assert(list != NULL);

    list_entry_t *first  = list->first;
    if (first) {
        first->previous = malloc(sizeof(list_entry_t));
        first->previous->next = first;
        first->previous->previous = NULL;
        
        list->first = first->previous;
    } else {
        list->first = malloc(sizeof(list_entry_t));
        list->last = list->first;
        
        list->first->previous = NULL;
        list->first->next = NULL;
    }
    
    list->first->data = obj;
    list->entry_count ++;
}

list_entry_t*
list_find_entry(list_t *list, void *obj)
{
    list_entry_t *e = list->first;
    
    while (e && e->data != obj) {
        e = e->next;
    }
    
    // Will return the entry if found, otherwise NULL as ends are grounded
    return e;
}


void*
list_remove_first(list_t *list)
{
    assert(list != NULL);
    assert(list->entry_count > 0);
    
    list_entry_t *first = list_first(list);
    void *data = list_entry_data(first);
    
    list->first = list->first->next;
    
    if (list->first) {
        list->first->previous = NULL;
    }
    
    list->entry_count --;
    
    free(first);
    
    return data;
}

void*
list_remove_last(list_t *list)
{
    assert(list != NULL);
    assert(list->entry_count > 0);

    list_entry_t *last = list_last(list);
    void *data = list_entry_data(last);
    
    list->last = list->last->previous;
    
    if (list->last) {
        list->last->next = NULL;
    }
    
    list->entry_count --;
    
    free(last);
    
    return data;
}

void*
list_remove_entry(list_t *list, list_entry_t *entry)
{
    assert(list != NULL);
    assert(entry != NULL);
    assert(list->entry_count > 0);
    // It is required that th entry is in the list, but that take O(n) to check
    // It is not quite that bad, but if the entry is the first or the last
    // entry in a list and the wrong list is supplied the list actually contain
    // the entry will have its first / last object pointing to undefined data
    // maybe there should be a debug version of this function that actually
    // verify that the entry is in the list?

    list_entry_t *prev = entry->previous;
    list_entry_t *next = entry->next;

    void *data = list_entry_data(entry);

    if (list->first == entry) list->first = next; 
    if (list->last == entry) list->last = prev;
    
    if (next != NULL) next->previous = prev;
    if (prev != NULL) prev->next = next;
    
    list->entry_count --;
    
    free(entry);
    
    return data;
}


size_t
list_length(const list_t *list)
{
    return list->entry_count;
}