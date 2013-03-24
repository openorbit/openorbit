/*
Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>

This file is part of Open Orbit.

Open Orbit is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Open Orbit is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef orbit_menu_manager_h
#define orbit_menu_manager_h

typedef void menu_t;
typedef void (*menu_func_t)(void*);
#define MENU_SEP ":"

menu_t* menu_get(const char *menu_name);

// Creates a new menu entry
// If f is NULL, it is a submenu
menu_t* menu_new(menu_t *parent, const char *name, menu_func_t f, void *arg);


#endif
