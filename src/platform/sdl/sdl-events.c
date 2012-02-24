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
#include "io-manager.h"


static io_keycode_t keymap [SDL_NUM_SCANCODES] = {
  [SDL_SCANCODE_LSHIFT] = IO_LSHIFT,
  [SDL_SCANCODE_RSHIFT] = IO_RSHIFT,
  [SDL_SCANCODE_LGUI] = IO_LMETA,
  [SDL_SCANCODE_RGUI] = IO_RMETA,
  [SDL_SCANCODE_LCTRL] = IO_LCTRL,
  [SDL_SCANCODE_RCTRL] = IO_RCTRL,

  [SDL_SCANCODE_MODE] = IO_MODE,
  [SDL_SCANCODE_HELP] = IO_HELP,
  [SDL_SCANCODE_SYSREQ] = IO_SYSREQ,
  [SDL_SCANCODE_CLEAR] = IO_CLEAR,
  [SDL_SCANCODE_MENU] = IO_MENU,
  [SDL_SCANCODE_POWER] = IO_POWER,

  [SDL_SCANCODE_RETURN] = IO_RETURN,
  [SDL_SCANCODE_SPACE] = IO_SPACE,
  [SDL_SCANCODE_TAB] = IO_TAB,
  [SDL_SCANCODE_BACKSPACE] = IO_BACKSPACE,
  [SDL_SCANCODE_ESCAPE] = IO_ESCAPE,
  [SDL_SCANCODE_PERIOD] = IO_PERIOD,
  [SDL_SCANCODE_COMMA] = IO_COMMA,
  [SDL_SCANCODE_PAUSE] = IO_PAUSE,
  [SDL_SCANCODE_MINUS] = IO_MINUS,
  [SDL_SCANCODE_SLASH] = IO_SLASH,

  [SDL_SCANCODE_SEMICOLON] = IO_SEMICOLON,
  [SDL_SCANCODE_EQUALS] = IO_EQUALS,
  [SDL_SCANCODE_LEFTBRACKET] = IO_LEFTBRACKET,
  [SDL_SCANCODE_BACKSLASH] = IO_BACKSLASH,
  [SDL_SCANCODE_RIGHTBRACKET] = IO_RIGHTBRACKET,

  [SDL_SCANCODE_0] = IO_0,
  [SDL_SCANCODE_1] = IO_1,
  [SDL_SCANCODE_2] = IO_2,
  [SDL_SCANCODE_3] = IO_3,
  [SDL_SCANCODE_4] = IO_4,
  [SDL_SCANCODE_5] = IO_5,
  [SDL_SCANCODE_6] = IO_6,
  [SDL_SCANCODE_7] = IO_7,
  [SDL_SCANCODE_8] = IO_8,
  [SDL_SCANCODE_9] = IO_9,

  [SDL_SCANCODE_KP_0] = IO_KP_0,
  [SDL_SCANCODE_KP_1] = IO_KP_1,
  [SDL_SCANCODE_KP_2] = IO_KP_2,
  [SDL_SCANCODE_KP_3] = IO_KP_3,
  [SDL_SCANCODE_KP_4] = IO_KP_4,
  [SDL_SCANCODE_KP_5] = IO_KP_5,
  [SDL_SCANCODE_KP_6] = IO_KP_6,
  [SDL_SCANCODE_KP_7] = IO_KP_7,
  [SDL_SCANCODE_KP_8] = IO_KP_8,
  [SDL_SCANCODE_KP_9] = IO_KP_9,

  [SDL_SCANCODE_KP_PERIOD]   = IO_KP_PERIOD,
  [SDL_SCANCODE_KP_DIVIDE]   = IO_KP_DIV,
  [SDL_SCANCODE_KP_MULTIPLY] = IO_KP_MUL,
  [SDL_SCANCODE_KP_MINUS]    = IO_KP_MIN,
  [SDL_SCANCODE_KP_PLUS]     = IO_KP_PLUS,
  [SDL_SCANCODE_KP_ENTER]    = IO_KP_ENTER,
  [SDL_SCANCODE_KP_EQUALS]   = IO_KP_EQ,
  [SDL_SCANCODE_DELETE]      = IO_DEL,

  [SDL_SCANCODE_A] = IO_A,
  [SDL_SCANCODE_B] = IO_B,
  [SDL_SCANCODE_C] = IO_C,
  [SDL_SCANCODE_D] = IO_D,
  [SDL_SCANCODE_E] = IO_E,
  [SDL_SCANCODE_F] = IO_F,
  [SDL_SCANCODE_G] = IO_G,
  [SDL_SCANCODE_H] = IO_H,
  [SDL_SCANCODE_I] = IO_I,
  [SDL_SCANCODE_J] = IO_J,
  [SDL_SCANCODE_K] = IO_K,
  [SDL_SCANCODE_L] = IO_L,
  [SDL_SCANCODE_M] = IO_M,
  [SDL_SCANCODE_N] = IO_N,
  [SDL_SCANCODE_O] = IO_O,
  [SDL_SCANCODE_P] = IO_P,
  [SDL_SCANCODE_Q] = IO_Q,
  [SDL_SCANCODE_R] = IO_R,
  [SDL_SCANCODE_S] = IO_S,
  [SDL_SCANCODE_T] = IO_T,
  [SDL_SCANCODE_U] = IO_U,
  [SDL_SCANCODE_V] = IO_V,
  [SDL_SCANCODE_W] = IO_W,
  [SDL_SCANCODE_X] = IO_X,
  [SDL_SCANCODE_Y] = IO_Y,
  [SDL_SCANCODE_Z] = IO_Z,

  [SDL_SCANCODE_F1]  = IO_F1,
  [SDL_SCANCODE_F2]  = IO_F2,
  [SDL_SCANCODE_F3]  = IO_F3,
  [SDL_SCANCODE_F4]  = IO_F4,
  [SDL_SCANCODE_F5]  = IO_F5,
  [SDL_SCANCODE_F6]  = IO_F6,
  [SDL_SCANCODE_F7]  = IO_F7,
  [SDL_SCANCODE_F8]  = IO_F8,
  [SDL_SCANCODE_F9]  = IO_F9,
  [SDL_SCANCODE_F10] = IO_F10,
  [SDL_SCANCODE_F11] = IO_F11,
  [SDL_SCANCODE_F12] = IO_F12,
  [SDL_SCANCODE_F13] = IO_F13,
  [SDL_SCANCODE_F14] = IO_F14,
  [SDL_SCANCODE_F15] = IO_F15,

  [SDL_SCANCODE_UP]    = IO_UP,
  [SDL_SCANCODE_DOWN]  = IO_DOWN,
  [SDL_SCANCODE_LEFT]  = IO_LEFT,
  [SDL_SCANCODE_RIGHT] = IO_RIGHT,

  [SDL_SCANCODE_INSERT]   = IO_INSERT,
  [SDL_SCANCODE_HOME]     = IO_HOME,
  [SDL_SCANCODE_END]      = IO_END,
  [SDL_SCANCODE_PAGEUP]   = IO_PAGEUP,
  [SDL_SCANCODE_PAGEDOWN] = IO_PAGEDOWN,
};

void
sdl_dispatch_key_down(int scancode, uint16_t mod)
{
  ioDispatchKeyDown(keymap[scancode], mod);
}


void
sdl_dispatch_key_up(int scancode, uint16_t mod)
{
  ioDispatchKeyUp(keymap[scancode], mod);
}
