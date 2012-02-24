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
