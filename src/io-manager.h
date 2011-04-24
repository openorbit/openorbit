/*
  Copyright 2006,2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

  void ioInit(void);
  void ioDispatchKeyUp(int key, uint16_t mask);
  void ioDispatchKeyDown(int key, uint16_t mask);
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
