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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


#include <gencds/hashtable.h>
#include <gencds/list.h>

#include "hashtable-private.h"
#include "list-private.h"


#define IS_POWER_OF_2(val) ((!(val & (val-1))) && val)
#define HASH(ht, key) (ht->hash(key) & (ht->ts - 1))



// returns 1 iff the two strings are equal and 0 iff they are unequal
bool
hashtable_string_comp(const char * restrict s0, const char * restrict s1)
{
    while (*s0 || *s1) {
        if (*s0 != *s1) return false;
        s0 ++;
        s1 ++;
    }
    
    return true;
}

// horrible implementation for now...
unsigned int
hashtable_string_hash(const char *key)
{
    assert(key  != NULL);
    
    unsigned int hash = 0;
    
    // compute hash
    while (*key) {
        hash = (hash + (unsigned int)*key) << 5;
        key ++;
    }
    
    // permute
    hash = (~hash) + (hash << 15);
    hash = hash ^ (hash >> 15);
    hash = hash + (hash << 3);
    hash = hash ^ (hash >> 5);
    hash = hash + (hash << 10);
    hash = hash ^ (hash >> 16);
    
    return hash;
}


int
hashtable_string_key_copy(char **dst, const char *src)
{
    *dst = strdup(src);
    if (*dst == NULL) return 1;
    return 0;
}

void
hashtable_string_key_del(char *key)
{
    free(key);
}


unsigned int
hashtable_ptr_hash(const char *key)
{
    return ((unsigned int)key & 0xfffffff0) >> 4; 
}

bool
hashtable_ptr_comp(const char *h0, const char *h1)
{
    return h0 == h1;
}

int
hashtable_ptr_key_copy(char **dst, const char *src)
{
    *dst = (char*)src;
    
    return 0;
}

void
hashtable_ptr_key_del(char *key)
{
    return;
}


hashtable_t
*hashtable_new(hash_fn_t fn, hash_cmp_fn_t cmp,
               hash_key_copy_fn_t cpy, hash_key_del_fn_t del,
               unsigned int base_size)
{
    assert(fn != NULL);
    assert(cmp != NULL);
    assert(cpy != NULL);
    assert(del != NULL);

    
    hashtable_t *ht;
    
    if ( ! IS_POWER_OF_2(base_size) ) {
        return NULL;
    }
    if (! (ht = calloc(1, sizeof(hashtable_t))) ) {
        return NULL;
    }
    
    if (! (ht->t = calloc(base_size, sizeof(hashentry_t*))) ) {
        free(ht);
        return NULL;
    }
    
    ht->ts = base_size;
    
    ht->hash = fn;
    ht->cmp = cmp;
    ht->cpy_key = cpy;
    ht->del_key = del;
    ht->hash_entry_list.first = NULL;
    ht->hash_entry_list.last = NULL;
    ht->hash_entry_list.entry_count = 0;
    
    return ht;
}

hashtable_t*
hashtable_new_with_str_keys(unsigned int base_size)
{
    return hashtable_new(hashtable_string_hash, hashtable_string_comp,
                         hashtable_string_key_copy, hashtable_string_key_del,
                         base_size);
}

hashtable_t*
hashtable_new_with_ptr_keys(unsigned int base_size)
{
    return hashtable_new(hashtable_ptr_hash, hashtable_ptr_comp,
                         hashtable_ptr_key_copy, hashtable_ptr_key_del,
                         base_size);
}


void
hashtable_delete(hashtable_t *ht)
{
    assert(ht != NULL);
    assert(ht->t != NULL);
    
    for (int i = 0 ; i < ht->ts ; i ++) {
        hashentry_t *entry = ht->t[i];
        if (!entry) continue;
        
        while (entry) {
            hashentry_t *next_entry = entry->next;
            ht->del_key(entry->key);
            free(entry);
            entry = next_entry;
        }
    }
    
    list_entry_t *e = list_first(& ht->hash_entry_list);
    while (e) {
        list_entry_t *p = e;
        e = list_entry_next(e);
        free(p);
    }
    
    free(ht->t);
    free(ht);
}


void
*hashtable_lookup(const hashtable_t * restrict ht, const char * restrict key)
{
    assert(ht != NULL);
    assert(key != NULL);
    
    unsigned int hash = HASH(ht, key);
        
    hashentry_t *entry = ht->t[hash];
    
    while (entry && ! ht->cmp(key, entry->key)) {
        entry = entry->next;
    }
    
    return (entry) ? entry->object : NULL;
}

list_entry_t*
hashtable_lookup_list_entry(const hashtable_t * restrict ht,
                            const char * restrict key)
{
    assert(ht != NULL);
    assert(key != NULL);
    
    unsigned int hash = HASH(ht, key);
    
    hashentry_t *entry = ht->t[hash];
    
    while (entry && ! ht->cmp(key, entry->key)) {
        entry = entry->next;
    }
    
    return (entry) ? entry->list_entry : NULL;
}


int
hashtable_insert(hashtable_t * restrict ht, const char * restrict key,
                 void * restrict obj)
{
    assert(ht != NULL);
    assert(key != NULL);
    
    unsigned int hash = HASH(ht, key);
    
    hashentry_t *new_entry = calloc(1, sizeof(hashentry_t));
    if (new_entry == NULL) {
        return -1;
    }
    ht->cpy_key(&(new_entry->key), key);
    if (new_entry->key == NULL) {
        free(new_entry);
        return -1;
    }
    
    list_append(&ht->hash_entry_list, new_entry);
    new_entry->list_entry = list_last(&ht->hash_entry_list);
    new_entry->object = obj;
    

    // is the top of the bucket free?
    if ( ! ht->t[hash] ) {
        // remember, calloc clears the memory and thus next will be grounded to NULL
        ht->t[hash] = new_entry;
    } else {
        // not first element, go lower in bucket
        hashentry_t *entry = ht->t[hash];
        if ( ht->cmp(key, entry->key) ) return 1; // key already exists in table
        
        // go to end of bucket
        while (entry->next) {
            entry = entry->next;
            if ( ht->cmp(key, entry->key) ) return 1; // key exsists?
        }
        
        entry->next = new_entry;
    }
    return 0;
}

void
*hashtable_remove(hashtable_t * restrict ht, const char * restrict key)
{
    assert(ht != NULL);
    assert(key != NULL);
    
    unsigned int hash = HASH(ht, key);
    hashentry_t *entry = ht->t[hash];
    
    // remove first entry in bucket
    if (entry && ht->cmp(key, entry->key)) {
        void *obj = entry->object;
        ht->t[hash] = entry->next;
        
        ht->del_key(entry->key);
        
        list_remove_entry(&ht->hash_entry_list, entry->list_entry);

        free(entry);
        return obj;
    } else if (!entry) {
        return NULL;
    }
    
    // go to entry before the one we are to remove an element in
    while (entry->next && ! ht->cmp(key, entry->next->key)) entry = entry->next;
    
    // cannot delete a nonexisting entry
    if (entry->next) {
        hashentry_t *entry_to_delete;
        entry_to_delete = entry->next;
        
        // never introduce invalid entries
        entry->next = entry->next->next;
        
        void *obj = entry_to_delete->object;
        
        ht->del_key(entry_to_delete->key);
        list_remove_entry(&ht->hash_entry_list, entry_to_delete->list_entry);

        free(entry_to_delete);
        return obj;
    }
    
    // could not find entry matching the key
    return NULL;
}

list_entry_t*
hashtable_first(hashtable_t *ht)
{
    assert(ht != NULL);
    
    return ht->hash_entry_list.first;
}

list_entry_t*
hashtable_last(hashtable_t *ht)
{
    assert(ht != NULL);
    
    return ht->hash_entry_list.last;
}

void*
hashtable_entry_key(list_entry_t *entry)
{
    assert(entry != NULL);
    
    hashentry_t *hashentry = (hashentry_t*)entry->data;
    if (hashentry == NULL) return NULL;
    return hashentry->key;
}

void*
hashtable_entry_data(list_entry_t *entry)
{
    if (entry == NULL) return NULL;
    hashentry_t *hashentry = (hashentry_t*)entry->data;
    if (hashentry == NULL) return NULL;
    return hashentry->object;
}

void*
hashtable_entry_remove(hashtable_t *ht, list_entry_t *entry)
{
    // TODO: Delete with code in this function instead of calling the
    //       hashtable_remove as we already have the entry to remove, but this
    //       is easier and requires minimal testing. Also, if no one relies on
    //       this for performance critical code, we should just leave it.
    assert(ht != NULL);
    assert(entry != NULL);
    
    hashentry_t *hashentry = (hashentry_t*)entry->data;
    void *data = hashentry->object;
    hashtable_remove(ht, hashentry->key);
    return data;
}


void
hashtable_print(hashtable_t *ht)
{
    assert(ht != NULL);
    
    printf("ht = {\n\tts = %d\n\tt = {\n", ht->ts);
    for (int i = 0 ; i < ht->ts ; i ++) {
        if (ht->t[i]) {
            printf("\t\tid = %d {\n", i);
            hashentry_t *entry = ht->t[i];
            
            while (entry) {
                printf("\t\t\tentry = {\n\t\t\t\tkey = %s\n\t\t\t\tobj = %x\n\t\t\t}\n",
                       entry->key, entry->object);
                entry = entry->next;
            }
            printf("\t\t}\n");
        }
    }
    printf("\t}\n}\n");
}
