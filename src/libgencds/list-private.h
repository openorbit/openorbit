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
 
#ifndef __LIST_PRIVATE__H__
#define __LIST_PRIVATE__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <gencds/list.h>

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


#ifdef __cplusplus
}
#endif
    
#endif /* ! __LIST_PRIVATE__H__ */
