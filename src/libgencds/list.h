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
 * \file list.h
 * \brief List data structures and functions.
 * 
 * This file contain the public inteface to the list data structure.
 * */

#ifndef LIST_H_
#define LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief A node in the list.
 * 
 * The list_entry_t can only be used as with a pointer interface. The real
 * implementation is hidden in order to ensure that the data structures can be
 * updated.
 * 
 * In the first implementation these are simple nodes containing next, previous
 * and data pointers. The internals might change, non the less. The
 * list_entry_t will continue to exist, even if it is just in the form of an
 * iterator.
 * */
typedef struct _list_entry_t list_entry_t;

/*!
 * \brief A list object.
 * 
 * The list_t type represent a list ADT, the first version was implemented as a
 * linked list of list_entry_t nodes. Note that the internals are subject to
 * change, so do not use the internal structure in any way. Use only the public
 * functions.
 * */
typedef struct _list_t list_t;

list_t *list_new(void);
void list_delete(list_t *list);

list_entry_t *list_first(const list_t *list);
list_entry_t *list_last(const list_t *list);
list_entry_t *list_rank(const list_t *list, unsigned int rank);

list_entry_t *list_entry_next(const list_entry_t *entry);
list_entry_t *list_entry_previous(const list_entry_t *entry);
void *list_entry_data(const list_entry_t *entry);

void list_append(list_t *list, void *obj);
void list_insert(list_t *list, void *obj);

void* list_remove_first(list_t *list);
void* list_remove_last(list_t *list);
void* list_remove_entry(list_t *list, list_entry_t *entry);

/*!
 * \brief Applies any function that take the list object as first parameter to
 *      all list objects.
 * 
 * \param lst A list_t object.
 * \param fn The function to apply to the list objects.
 * \param ... An arbitrary number of arguments (zero or more) to pass into the
 *      function after the object argument.
 * */
#define LIST_APPLY(lst, fn, ...)                        \
do {                                                    \
    list_entry_t *entry = list_first(lst);              \
                                                        \
    while (entry) {                                     \
        (fn)(list_entry_data(entry), ## __VA_ARGS__);   \
        entry = list_entry_next(entry);                 \
    }                                                   \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /*LIST_H_*/
