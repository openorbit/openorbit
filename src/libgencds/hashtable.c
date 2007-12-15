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

#include "hashtable.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define IS_POWER_OF_2(val) ((!(val & (val-1))) && val)
#define HASH(ht, key) (ht->hash(key) & (ht->ts - 1))


typedef struct _hashentry_t {
    struct _hashentry_t *next; /*!< Next entry in chained list */
    char *key; /*!< A C-string used for as a key when looking up an element */
    void *object; /*!< Pointer to the object associated to the key */
} hashentry_t;

struct _hashtable_t {
    size_t ts; /*!< Size of table, this is also used as a compression map */
    hashentry_t **t; /*!< Array of linked lists of key-value pairs */
    hash_fn_t hash; /*!< The hash function used for this table */
    hash_cmp_fn_t cmp; /*!< The hash function used for this table */
    hash_key_copy_fn_t cpy_key;
    hash_key_del_fn_t del_key;
};


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

int
hashtable_insert(hashtable_t * restrict ht, const char * restrict key,
                 void * restrict obj)
{
    assert(ht != NULL);
    assert(key != NULL);
    
    unsigned int hash = HASH(ht, key);
    
    if ( ! ht->t[hash] ) {
        // remember, calloc clears the memory and thus next will be grounded to NULL
        ht->t[hash] = calloc(1, sizeof(hashentry_t));
        if (! ht->t[hash]) return -1;
        ht->cpy_key(&(ht->t[hash]->key), key);
        if (! ht->t[hash]->key) {
            free(ht->t[hash]);
            return -1;
        }
        ht->t[hash]->object = obj;
    } else {
        // not first element, go lower in bucket
        hashentry_t *entry = ht->t[hash];
        if ( ht->cmp(key, entry->key) ) return 1; // key already exists in table
        
        // go to end of bucket
        while (entry->next) {
            entry = entry->next;
            if ( ht->cmp(key, entry->key) ) return 1; // key exsists?
        }
        
        entry->next = calloc(1, sizeof(hashentry_t));
        if (! entry->next) return -1;
        
        ht->cpy_key(&(entry->next->key), key);
        
        if (! entry->next->key) {
            free(entry->next);
            return -1;
        }
        entry->next->object = obj;
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
        free(entry_to_delete);
        return obj;
    }
    
    // could not find entry matching the key
    return NULL;
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
