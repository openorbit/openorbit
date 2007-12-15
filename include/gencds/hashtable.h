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

void hashtable_print(hashtable_t *ht);
#ifdef __cplusplus
}
#endif
    
#endif
