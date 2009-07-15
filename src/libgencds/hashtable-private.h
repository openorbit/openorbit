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


#ifndef __HASHTABLE_PRIVATE__H__
#define __HASHTABLE_PRIVATE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include "list-private.h"

typedef struct _hashentry_t {
    struct _hashentry_t *next; /*!< Next entry in chained list */
    char *key; /*!< A C-string used for as a key when looking up an element */
    void *object; /*!< Pointer to the object associated to the key */
    struct _list_entry_t *list_entry;
} hashentry_t;

struct _hashtable_t {
    size_t ts; /*!< Size of table, this is also used as a compression map */
    hashentry_t **t; /*!< Array of linked lists of key-value pairs */
    hash_fn_t hash; /*!< The hash function used for this table */
    hash_cmp_fn_t cmp; /*!< The hash function used for this table */
    hash_key_copy_fn_t cpy_key;
    hash_key_del_fn_t del_key;
    struct _list_t hash_entry_list;
};

#ifdef __cplusplus
}
#endif
    
#endif /* ! __HASHTABLE_PRIVATE__H__ */
