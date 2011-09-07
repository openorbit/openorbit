/*
  Copyright 2006,2008,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef __IO_MANAGER_H__
#define __IO_MANAGER_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <Python.h>
	
//#include "SDL.h"
#include <SDL/SDL.h>

/* These use the same values as SDL, so for porting (to not use SDL) they must be
   translated in some way */
#define OO_IO_MOD_NONE   KMOD_NONE
#define OO_IO_MOD_LSHIFT KMOD_LSHIFT
#define OO_IO_MOD_RSHIFT KMOD_RSHIFT
#define OO_IO_MOD_LCTRL  KMOD_LCTRL
#define OO_IO_MOD_RCTRL  KMOD_RCTRL
#define OO_IO_MOD_LALT   KMOD_LALT
#define OO_IO_MOD_RALT   KMOD_RALT
#define OO_IO_MOD_LMETA  KMOD_LGUI
#define OO_IO_MOD_RMETA  KMOD_RGUI
#define OO_IO_MOD_NUM    KMOD_NUM
#define OO_IO_MOD_CAPS   KMOD_CAPS
#define OO_IO_MOD_MODE   KMOD_MODE

typedef enum io_keycode_t
{
  IO_INVALID = 0,
  IO_LSHIFT,
  IO_RSHIFT,
  IO_LMETA,
  IO_RMETA,
  IO_LCTRL,
  IO_RCTRL,
  IO_LALT,
  IO_RALT,

  IO_MODE,
  IO_HELP,
  IO_SYSREQ,
  IO_CLEAR,
  IO_MENU,
  IO_POWER,

  IO_RETURN,
  IO_SPACE,
  IO_TAB,
  IO_BACKSPACE,
  IO_ESCAPE,
  IO_PERIOD,
  IO_COMMA,
  IO_PAUSE,
  IO_MINUS,
  IO_SLASH,

  IO_SEMICOLON,
  IO_EQUALS,
  IO_LEFTBRACKET,
  IO_BACKSLASH,
  IO_RIGHTBRACKET,

  IO_0,
  IO_1,
  IO_2,
  IO_3,
  IO_4,
  IO_5,
  IO_6,
  IO_7,
  IO_8,
  IO_9,

  IO_KP_0,
  IO_KP_1,
  IO_KP_2,
  IO_KP_3,
  IO_KP_4,
  IO_KP_5,
  IO_KP_6,
  IO_KP_7,
  IO_KP_8,
  IO_KP_9,

  IO_KP_PERIOD,
  IO_KP_DIV,
  IO_KP_MUL,
  IO_KP_MIN,
  IO_KP_PLUS,
  IO_KP_ENTER,
  IO_KP_EQ,
  IO_KP_CLR,

  IO_DEL,

  IO_A,
  IO_B,
  IO_C,
  IO_D,
  IO_E,
  IO_F,
  IO_G,
  IO_H,
  IO_I,
  IO_J,
  IO_K,
  IO_L,
  IO_M,
  IO_N,
  IO_O,
  IO_P,
  IO_Q,
  IO_R,
  IO_S,
  IO_T,
  IO_U,
  IO_V,
  IO_W,
  IO_X,
  IO_Y,
  IO_Z,

  IO_F1,
  IO_F2,
  IO_F3,
  IO_F4,
  IO_F5,
  IO_F6,
  IO_F7,
  IO_F8,
  IO_F9,
  IO_F10,
  IO_F11,
  IO_F12,
  IO_F13,
  IO_F14,
  IO_F15,

  IO_UP,
  IO_DOWN,
  IO_LEFT,
  IO_RIGHT,

  IO_INSERT,
  IO_HOME,
  IO_END,
  IO_PAGEUP,
  IO_PAGEDOWN,

  IO_COUNT,
} io_keycode_t;

void ioInit(void);
void ioDispatchKeyUp(io_keycode_t key, uint16_t mask);
void ioDispatchKeyDown(io_keycode_t key, uint16_t mask);
void ioBindKeyToAxis(const char *key, const char *button, float val);
void ioDispatchButtonDown(int dev, int button);
void ioDispatchButtonUp(int dev, int button);

typedef void (*OObuttonhandlerfunc)(bool buttonDown, void *data);

void ioRegActionHandler(const char *name, OObuttonhandlerfunc handlerFunc, void *data);
/*! Register key handler C function */
void ooIoRegCKeyHandler(const char *name, OObuttonhandlerfunc handlerFunc);
/*! Register key handler Python function */
void ooIoRegPyKeyHandler(const char *name, PyObject *handlerFunc);

/*! Bind keyboard with keyName to the keyAction key

    \param keyName String key for the keyboard button
    \param keyAction Key corresponding to the registered action
    \param up 0 if the handler should fire on button down, otherwise it will fire on
        button up.
    \param mask ored mask from the OO_IO_MOD* constants
*/
void ooIoBindKeyHandler(const char *keyName, const char *keyAction, int up,
                        uint16_t mask);


void ooIoPrintJoystickNames(void);
float ooIoGetAxis(float *val, const char *axis);
int ooIoGetJoystickId(const char *name, int subId);
void ooIoBindAxis(const char *key, int joyStick, int axis);

const char *ooIoSdlMouseButtonNameLookup(unsigned buttonId);

#ifdef __cplusplus
}
#endif

#endif /* ! __IO_MANAGER_H */
