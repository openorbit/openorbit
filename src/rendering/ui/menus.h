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

#ifndef orbit_menus_h
#define orbit_menus_h

#include "gencds/array.h"
#include "rendering/texture.h"
#include "rendering/text.h"

typedef struct ui_menu_item_t ui_menu_item_t;

typedef void (*ui_menu_action_f)(ui_menu_item_t *menu, void *data);

typedef enum {
  UI_Menu,
  UI_Menu_Item,
} ui_menu_kind_t;

struct ui_menu_item_t {
  ui_menu_kind_t kind;
  const char *title;
  ui_menu_action_f action;
  void *data;

  GLuint texid;
  unsigned bounds_x;
  unsigned bounds_y;
};

typedef struct {
  ui_menu_kind_t kind;
  const char *title;
  obj_array_t items;
  unsigned current_item;
  
  GLuint texid;
  unsigned bounds_x;
  unsigned bounds_y;
  
} ui_menu_t;

ui_menu_t* ui_new_menu(const char *title);
ui_menu_item_t* ui_new_menu_item(ui_menu_t *menu, const char *title,
                                 ui_menu_action_f action, void *data);

void ui_menu_mouse_move(ui_menu_t *menu, unsigned new_x, unsigned new_y);
void ui_menu_prev(ui_menu_t *menu);
void ui_menu_next(ui_menu_t *menu);

void ui_menu_select(ui_menu_t *menu);
void ui_menu_exit(ui_menu_t *menu);
void ui_menu_mouse_up(ui_menu_t *menu, unsigned new_x, unsigned new_y);

void ui_menu_build(ui_menu_t *menu);
void ui_menu_draw(ui_menu_t *menu);

void ui_menu_window_size_changed(ui_menu_t *menu,
                                 unsigned w, unsigned h);

#endif
