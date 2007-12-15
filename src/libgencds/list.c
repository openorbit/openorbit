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


#include <stdlib.h>
#include <assert.h>

#include "list.h"


struct _list_entry_t {
    void *data;
    struct _list_entry_t *next;
    struct _list_entry_t *previous;
};

struct _list_t {
    list_entry_t *first;
    list_entry_t *last;
    size_t entry_count;
};

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
    // It is required that th entry is in the list, but that take O(n) to check
    assert(list->entry_count > 0);

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



 