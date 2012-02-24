/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef orbit_sdl_window_h
#define orbit_sdl_window_h

void sdl_window_resize(SDL_Window *window, int width, int height);
void sdl_window_toggle_fs(SDL_Window *window);
SDL_Window* sdl_window_init(int width, int height, bool fullscreen);

void sdl_init_gl(void);
void sdl_print_gl_attrs(void);

#endif
