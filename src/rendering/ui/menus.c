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

#include <stdio.h>
#include <string.h>
#include "menus.h"

ui_menu_t*
ui_new_menu(const char *title)
{
  ui_menu_t *menu = malloc(sizeof(ui_menu_t));
  obj_array_init(&menu->items);
  menu->title = strdup(title);

  return menu;
}

ui_menu_item_t*
ui_new_menu_item(ui_menu_t *menu, const char *title,
                 ui_menu_action_f action, void *data)
{
  ui_menu_item_t *mi = malloc(sizeof(ui_menu_item_t));
  mi->title = strdup(title);
  mi->action = action;
  mi->data = data;
  obj_array_push(&menu->items, mi);

  return mi;
}

void
ui_menu_prev(ui_menu_t *menu)
{
  menu->current_item --;
  if (menu->current_item >= ARRAY_LEN(menu->items)) {
    menu->current_item = ARRAY_LEN(menu->items) - 1;
  }
}

void
ui_menu_next(ui_menu_t *menu)
{
  menu->current_item ++;
  menu->current_item %= ARRAY_LEN(menu->items);
}

void
ui_menu_select(ui_menu_t *menu)
{
  ui_menu_item_t *item = ARRAY_ELEM(menu->items, menu->current_item);
  
  if (item->kind == UI_Menu_Item) {
    item->action(item, item->data);
  } else if (item->kind == UI_Menu) {
    ui_menu_t *selected_menu = ARRAY_ELEM(menu->items, menu->current_item);
    selected_menu->current_item = 0;
  }
}

void
ui_menu_mouse_up(ui_menu_t *menu, unsigned new_x, unsigned new_y)
{
  ui_menu_select(menu);
}

void
ui_menu_mouse_move(ui_menu_t *menu, unsigned new_x, unsigned new_y)
{
  
}

void
ui_menu_build(ui_menu_t *menu)
{
  // Builds drawable menus in texture memory
}

void
ui_menu_draw(ui_menu_t *menu)
{
  // Draw current menu, highlights selection

}

void
ui_menu_window_size_changed(ui_menu_t *menu,
                            unsigned w, unsigned h)
{
  // Window size changed, called for the root menu only
}
