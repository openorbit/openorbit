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


/*!
    \file hashtable.h
    \brief Functions and data structures implementing a generic hashtable
    
    The hash table is implemented as an array of linked lists. The linked lists
    allow for hash collissions and graceful degradation of performance.
*/

#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#ifdef __cplusplus
extern "C" {
#endif
    

#include <stdlib.h>
#include <stdbool.h>

#include <gencds/list.h>
    
typedef struct _hashtable_t hashtable_t;

/*! Hash function */
typedef unsigned int (*hash_fn_t)(const char *key);
typedef bool (*hash_cmp_fn_t)(const char *h0, const char *h1);
typedef int (*hash_key_copy_fn_t)(char **dst, const char *src);
typedef void (*hash_key_del_fn_t)(char *key);


/*!
 * \brief Default hash function.
 * 
 * The default hashfunction computes a hash from a NULL terminated text string.
 *
 * \param key The hash key, a null terminated string.
 * \return The hash of the string.
 * */
unsigned int hashtable_string_hash(const char *key);
bool hashtable_string_comp(const char *k0, const char *k1);
int hashtable_string_key_copy(char **dst, const char *src);
void hashtable_string_key_del(char *key);

hashtable_t* hashtable_new_with_str_keys(unsigned int base_size);

unsigned int hashtable_ptr_hash(const char *key);
bool hashtable_ptr_comp(const char *k0, const char *k1);
int hashtable_ptr_key_copy(char **dst, const char *src);
void hashtable_ptr_key_del(char *key);

hashtable_t* hashtable_new_with_ptr_keys(unsigned int base_size);

/*!
    \brief Creates a new hashtable
    \param fn The hashfunction, NULL if the default should be used.
    \param max_elems The size of the hashtable. Note that the hashtable use
        chaining, so the table will not run out of space, just degrade in
        performance.
    \return A pointer to a newly created hashtable
 */
 
 
hashtable_t* hashtable_new(hash_fn_t fn, hash_cmp_fn_t cmp,
                           hash_key_copy_fn_t cpy, hash_key_del_fn_t del,
                           unsigned int base_size);

/*!
 * \brief Deletes a hashtable created with hashtable_new
 * \warning This function only deletes the memory allocated by the hashtable
 *      itself.  It claims no responsibility of the objects stored in the
 *      hashtable. This mean that the one who calls this function is
 *      responsible for releasing all the objects in the table.
 * \param ht The hashtabe to be deleted.
 * */
void hashtable_delete(hashtable_t *ht);

/*!
 * \brief Looks up an object in the hashtable.
 * 
 * The function looks up an object associated with a certain key in the
 * hashtable.
 * \bug It is not possible to distinguish between a NULL object and a non
 *      existing object.
 * \param ht The hashtable that should be searched for the object.
 * \param key The key that is associated to the object.
 * \return A pointer to the object beeing searched for. NULL if the object does
 *      not exist.
 * */
void *hashtable_lookup(const hashtable_t *ht, const char *key);
// -1 = memory error, 0 = success, 1 = key already exist
int hashtable_insert(hashtable_t *ht, const char *key, void *obj);
// returns the obj for the entry being deleted, NULL if the entry cannot be found
void* hashtable_remove(hashtable_t *ht, const char *key);

list_entry_t* hashtable_lookup_list_entry(const hashtable_t * restrict ht,
                                          const char * restrict key);
    
    
list_entry_t* hashtable_first(hashtable_t *ht);
list_entry_t* hashtable_last(hashtable_t *ht);
void* hashtable_entry_key(list_entry_t *entry);
void* hashtable_entry_data(list_entry_t *entry);
void* hashtable_entry_remove(hashtable_t *ht, list_entry_t *entry);

void hashtable_print(hashtable_t *ht);
#ifdef __cplusplus
}
#endif
    
#endif
