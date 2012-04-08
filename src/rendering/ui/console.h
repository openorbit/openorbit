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

#ifndef orbit_console_h
#define orbit_console_h
#include "rendering/texture.h"
#include "rendering/text.h"
#include "hashtable.h"

typedef struct ui_line_t {
  struct ui_line_t *prev;
  struct ui_line_t *next;
  
  size_t line_len; // Allocation length
  char *line;

  SGtextbuffer *buff;
  GLuint texid;
} ui_line_t;

typedef struct {
  ui_line_t *bottom;

  unsigned w;
  unsigned h;

  hashtable_t *commands;
} ui_console_t;

typedef int (*ui_command_t)(unsigned argc, const char * argv[argc]);

ui_console_t* ui_new_console(void);
void ui_console_scroll_to_top(ui_console_t *console);
void ui_console_scroll_to_bottom(ui_console_t *console);
void ui_console_scroll_up(ui_console_t *console, unsigned lines);
void ui_console_scroll_down(ui_console_t *console, unsigned lines);

void ui_console_draw(ui_console_t *console);

void ui_console_clear(ui_console_t *console);
void ui_console_char_entry(ui_console_t *console, char ch);

void ui_console_register_command(ui_console_t *console,
                                 const char *cmd, ui_command_t handler);

void ui_console_window_size_changed(ui_console_t *console,
                                    unsigned w, unsigned h);
#endif
