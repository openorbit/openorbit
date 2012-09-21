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
#include "console.h"
#include "palloc.h"

ui_console_t*
ui_new_console(void)
{
  ui_console_t *console = smalloc(sizeof(ui_console_t));
  console->bottom = NULL;
  console->commands = hashtable_new_with_str_keys(1024);
}


void
ui_console_scroll_to_top(ui_console_t *console)
{
  
}


void
ui_console_scroll_to_bottom(ui_console_t *console)
{
  
}


void
ui_console_scroll_up(ui_console_t *console, unsigned lines)
{
  
}

void
ui_console_scroll_down(ui_console_t *console, unsigned lines)
{
  
}


void
ui_console_draw(ui_console_t *console)
{
  
}


void
ui_console_clear(ui_console_t *console)
{

}

void
ui_console_char_entry(ui_console_t *console, char ch)
{
  if (ch == '\n') {
    // Parse current line
  } else {
    // Add char to buffer and re-render texture
  }
}

void
ui_console_register_command(ui_console_t *console,
                            const char *cmd, ui_command_t handler)
{
  hashtable_insert(console->commands, cmd, handler);
}

void
ui_console_window_size_changed(ui_console_t *console,
                               unsigned w, unsigned h)
{

}
