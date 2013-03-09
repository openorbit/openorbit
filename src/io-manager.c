/*
  Copyright 2006,2008,2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <stdbool.h>
#include <stdio.h>
#include <err.h>
#include <gencds/hashtable.h>
#include "sim.h"
#include "io-manager.h"
//#include "SDL.h"
//#include <SDL/SDL.h>
#include <openorbit/log.h>
#include "parsers/hrml.h"
#include "settings.h"
#include "palloc.h"

static hashtable_t *gIoButtonHandlers; // of type OObuttonhandler


#define SPEC_KEY_COUNT 8/* shft, cmd, ctrl, alt (left and right) */

void ioSetKeyHandler2(io_keycode_t key, IObuttonhandlerfunc keyHandler,
                      void *data0, void *data1);

void ioBindJoystickButton(const char *key, int joyStick, int button);
void ioBindButtonToAxis(const char *axisKey, int joystick, int button, float val);

void
ooButtonHandlerGnd(int state, void *data)
{
    /*Nothing*/
}

typedef struct {
  io_button_kind_t kind;
  bool isScript;
  union {
    IObuttonhandlerfunc cHandler;
    PyObject *pyHandler;
  };
  void *data;
  void *data1;
  void *data2;
} IObuttonhandler;

typedef struct OOkeyhandler {
  IObuttonhandler up[OO_Key_Mod_Count];
  IObuttonhandler down[OO_Key_Mod_Count];
} OOkeyhandler;
OOkeyhandler gIoKeyData[IO_END];

#define MOUSE_BUTTONS 8
typedef struct OOmousebuttons {
  IObuttonhandler down[MOUSE_BUTTONS];
  IObuttonhandler up[MOUSE_BUTTONS];
} OOmousebuttons;
OOmousebuttons gIoMouseButtons;


static const char *keysymmap [IO_COUNT] = {
  [IO_LSHIFT] = "lshift",
  [IO_RSHIFT] = "rshift",
  [IO_LMETA] = "lmeta",
  [IO_RMETA] = "rmeta",
  [IO_LCTRL] = "lctrl",
  [IO_RCTRL] = "rctrl",
  [IO_LALT] = "lalt",
  [IO_RALT] = "ralt",

  [IO_MODE] = "mode",
  [IO_HELP] = "help",
  [IO_SYSREQ] = "sys-req",
  [IO_CLEAR] = "clear",
  [IO_MENU] = "menu",
  [IO_POWER] = "power",

  [IO_RETURN] = "return",
  [IO_SPACE] = "space",
  [IO_TAB] = "tab",
  [IO_BACKSPACE] = "backspace",
  [IO_ESCAPE] = "esc",
  [IO_PERIOD] = ".",
  [IO_COMMA] = ",",
  [IO_PAUSE] = "pause",
  [IO_MINUS] = "-",
  [IO_SLASH] = "/",

  [IO_SEMICOLON] = ";",
  [IO_EQUALS] = "=",
  [IO_LEFTBRACKET] = "[",
  [IO_BACKSLASH] = "\\",
  [IO_RIGHTBRACKET] = "]",

  [IO_0] = "0",
  [IO_1] = "1",
  [IO_2] = "2",
  [IO_3] = "3",
  [IO_4] = "4",
  [IO_5] = "5",
  [IO_6] = "6",
  [IO_7] = "7",
  [IO_8] = "8",
  [IO_9] = "9",

  [IO_KP_0] = "kp 0",
  [IO_KP_1] = "kp 1",
  [IO_KP_2] = "kp 2",
  [IO_KP_3] = "kp 3",
  [IO_KP_4] = "kp 4",
  [IO_KP_5] = "kp 5",
  [IO_KP_6] = "kp 6",
  [IO_KP_7] = "kp 7",
  [IO_KP_8] = "kp 8",
  [IO_KP_9] = "kp 9",

  [IO_KP_PERIOD] = "kp .",
  [IO_KP_DIV] = "kp /",
  [IO_KP_MUL] = "kp *",
  [IO_KP_MIN] = "kp -",
  [IO_KP_PLUS] = "kp +",
  [IO_KP_ENTER] = "enter",
  [IO_KP_EQ] = "kp =",
  [IO_KP_CLR] = "kp clr",

  [IO_DEL] = "delete",

  [IO_A] = "a",
  [IO_B] = "b",
  [IO_C] = "c",
  [IO_D] = "d",
  [IO_E] = "e",
  [IO_F] = "f",
  [IO_G] = "g",
  [IO_H] = "h",
  [IO_I] = "i",
  [IO_J] = "j",
  [IO_K] = "k",
  [IO_L] = "l",
  [IO_M] = "m",
  [IO_N] = "n",
  [IO_O] = "o",
  [IO_P] = "p",
  [IO_Q] = "q",
  [IO_R] = "r",
  [IO_S] = "s",
  [IO_T] = "t",
  [IO_U] = "u",
  [IO_V] = "v",
  [IO_W] = "w",
  [IO_X] = "x",
  [IO_Y] = "y",
  [IO_Z] = "z",

  [IO_F1] = "f1",
  [IO_F2] = "f2",
  [IO_F3] = "f3",
  [IO_F4] = "f4",
  [IO_F5] = "f5",
  [IO_F6] = "f6",
  [IO_F7] = "f7",
  [IO_F8] = "f8",
  [IO_F9] = "f9",
  [IO_F10] = "f10",
  [IO_F11] = "f11",
  [IO_F12] = "f12",
  [IO_F13] = "f13",
  [IO_F14] = "f14",
  [IO_F15] = "f15",

  [IO_UP] = "up",
  [IO_DOWN] = "down",
  [IO_LEFT] = "left",
  [IO_RIGHT] = "right",

  [IO_INSERT] = "insert",
  [IO_HOME] = "home",
  [IO_END] = "end",
  [IO_PAGEUP] = "page up",
  [IO_PAGEDOWN] = "page down",
};
hashtable_t *gIoReverseKeySymMap;

hashtable_t *gIoAxisNameMap;
hashtable_t *gIoPhysAxisNameMap;

hashtable_t *gIoVirtSliderNameMap;
hashtable_t *gIoPhysSliderNameMap;

static const char *sliderMap [IO_SLIDER_COUNT] = {
  [IO_SLIDER_THROT_0] = "throttle0",
  [IO_SLIDER_THROT_1] = "throttle1",
};

static const char *physSliderMap [IO_SLIDER_COUNT] = {
  [IO_SLIDER_THROT_0] = "throttle0",
  [IO_SLIDER_THROT_1] = "throttle1",
};

static const char *axisMap [IO_AXIS_COUNT] = {
  [IO_AXIS_X] = "lateral",
  [IO_AXIS_Y] = "vertical",
  [IO_AXIS_Z] = "forward",
  [IO_AXIS_RX] = "pitch",
  [IO_AXIS_RY] = "yaw",
  [IO_AXIS_RZ] = "roll",
};

static const char *physAxisMap [IO_AXIS_COUNT] = {
  [IO_AXIS_X] = "x",
  [IO_AXIS_Y] = "y",
  [IO_AXIS_Z] = "z",
  [IO_AXIS_RX] = "rx",
  [IO_AXIS_RY] = "ry",
  [IO_AXIS_RZ] = "rz",
};


void platform_get_mouse(float *x, float *y);

void
ioGetMousePos(float *x, float *y)
{
  platform_get_mouse(x, y);
}

void ioInitKeys(void);

typedef struct {
  int vendorID;
  int productID;
  char *vendorName;
  char *productName;
  int buttons;
  int hats;
  io_axis_t axis_map[IO_AXIS_COUNT];
  io_slider_t slider_map[IO_SLIDER_COUNT];
  IObuttonhandler *buttonHandler;
  IObuttonhandler *hatHandler;
} io_device_info_t;

obj_array_t devices;

void
io_builtin_yaw(int state, void *data)
{
  if (!state) {
    ioAxisChanged(IO_AXIS_RY, 0.0f);
    return;
  }

  if (data) {
    ioAxisChanged(IO_AXIS_RY, -1.0);
  } else {
    ioAxisChanged(IO_AXIS_RY, 1.0);
  }
}

void
io_builtin_roll(int state, void *data)
{
  if (!state) {
    ioAxisChanged(IO_AXIS_RZ, 0.0f);
    return;
  }

  if (data) {
    ioAxisChanged(IO_AXIS_RZ, -1.0);
  } else {
    ioAxisChanged(IO_AXIS_RZ, 1.0);
  }
}

void
io_builtin_pitch(int state, void *data)
{
  if (!state) {
    ioAxisChanged(IO_AXIS_RX, 0.0f);
    return;
  }

  if (data) {
    ioAxisChanged(IO_AXIS_RX, -1.0);
  } else {
    ioAxisChanged(IO_AXIS_RX, 1.0);
  }
}

void
io_builtin_vertical(int state, void *data)
{
  if (!state) {
    ioAxisChanged(IO_AXIS_Y, 0.0f);
    return;
  }

  if (data) {
    ioAxisChanged(IO_AXIS_Y, -1.0);
  } else {
    ioAxisChanged(IO_AXIS_Y, 1.0);
  }
}

void
io_builtin_lateral(int state, void *data)
{
  if (!state) {
    ioAxisChanged(IO_AXIS_X, 0.0f);
    return;
  }

  if (data) {
    ioAxisChanged(IO_AXIS_X, -1.0);
  } else {
    ioAxisChanged(IO_AXIS_X, 1.0);
  }
}

void
io_builtin_forward(int state, void *data)
{
  if (!state) {
    ioAxisChanged(IO_AXIS_Z, 0.0f);
    return;
  }

  if (data) {
    ioAxisChanged(IO_AXIS_Z, -1.0);
  } else {
    ioAxisChanged(IO_AXIS_Z, 1.0);
  }
}


void
io_builtin_throttle(int state, void *data)
{
  if (data) {
    float newVal = ioGetSlider(IO_SLIDER_THROT_0) - 0.1f;
    newVal = newVal < 0.0f ? 0.0f :newVal;
    ioSliderChanged(IO_SLIDER_THROT_0, ioGetSlider(IO_SLIDER_THROT_0));
  } else {
    float newVal = ioGetSlider(IO_SLIDER_THROT_0) + 0.1f;
    newVal = newVal > 1.0f ? 1.0f :newVal;
    ioSliderChanged(IO_SLIDER_THROT_0, ioGetSlider(IO_SLIDER_THROT_0));
  }
}


MODULE_INIT(iomanager, NULL)
{
  ooLogTrace("initialising 'iomanager' module");
  gIoReverseKeySymMap = hashtable_new_with_str_keys(1024);
  gIoButtonHandlers = hashtable_new_with_str_keys(2048);
  gIoAxisNameMap = hashtable_new_with_str_keys(16);
  gIoPhysAxisNameMap = hashtable_new_with_str_keys(16);
  gIoVirtSliderNameMap = hashtable_new_with_str_keys(16);
  gIoPhysSliderNameMap = hashtable_new_with_str_keys(16);

  for (size_t i = 0 ; i < IO_END ; ++ i) {
    if (keysymmap[i] != NULL) {
      hashtable_insert(gIoReverseKeySymMap, keysymmap[i], (void*)i);
    }
  }

  for (size_t i = 0 ; i < IO_END ; ++ i) {
    for (size_t j = 0 ; j < OO_Key_Mod_Count ; ++ j) {
      gIoKeyData[i].up[j].isScript = false;
      gIoKeyData[i].up[j].cHandler = ooButtonHandlerGnd;
      gIoKeyData[i].up[j].data = NULL;
      gIoKeyData[i].down[j].isScript = false;
      gIoKeyData[i].down[j].cHandler = ooButtonHandlerGnd;
      gIoKeyData[i].down[j].data = NULL;

    }
  }

  for (size_t i = 0 ; i < MOUSE_BUTTONS ; ++ i) {
    gIoMouseButtons.down[i].isScript = false;
    gIoMouseButtons.down[i].cHandler = ooButtonHandlerGnd;
    gIoMouseButtons.up[i].isScript = false;
    gIoMouseButtons.up[i].cHandler = ooButtonHandlerGnd;
  }

  // Hook up the quit handler as default to cmd / meta q, this overrides the initkeys conf
//  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].isScript = false;
//  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].cHandler = ioQuitHandler;
//  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].data = NULL;
//  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].isScript = false;
//  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].cHandler = ioQuitHandler;
//  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].data = NULL;

  for (int i = 0 ; i < IO_AXIS_COUNT ; i++) {
    hashtable_insert(gIoAxisNameMap, axisMap[i], (void*)i);
  }

  for (int i = 0 ; i < IO_AXIS_COUNT ; i++) {
    hashtable_insert(gIoPhysAxisNameMap, physAxisMap[i], (void*)i);
  }

  for (int i = 0 ; i < IO_SLIDER_COUNT ; i++) {
    hashtable_insert(gIoVirtSliderNameMap, sliderMap[i], (void*)i);
  }

  for (int i = 0 ; i < IO_SLIDER_COUNT ; i++) {
    hashtable_insert(gIoPhysSliderNameMap, physSliderMap[i], (void*)i);
  }

  obj_array_init(&devices);

  // Axis emulation
  ioRegActionHandler("yaw", io_builtin_yaw, IO_BUTTON_MULTI, NULL);
  ioRegActionHandler("roll", io_builtin_roll, IO_BUTTON_MULTI, NULL);
  ioRegActionHandler("pitch", io_builtin_pitch, IO_BUTTON_MULTI, NULL);
  ioRegActionHandler("lateral", io_builtin_lateral, IO_BUTTON_MULTI, NULL);
  ioRegActionHandler("vertical", io_builtin_vertical, IO_BUTTON_MULTI, NULL);
  ioRegActionHandler("forward", io_builtin_forward, IO_BUTTON_MULTI, NULL);

  // Slider emulation
  ioRegActionHandler("throttle", io_builtin_throttle, IO_BUTTON_MULTI, NULL);
}

void
ioInit(void)
{

  // Now initialize joysticks
  //ioInitJoysticks();

  // Init key data
  ioInitKeys();
  ioInitJoysticks();


  // Hook up the quit handler as default to cmd / meta q, this overrides the initkeys conf
 // gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].isScript = false;
 // gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].cHandler = ioQuitHandler;
 // gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].data = NULL;
 // gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].isScript = false;
 // gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].cHandler = ioQuitHandler;
 // gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].data = NULL;
}

io_keycode_t
ioGetKeyCode(const char *key)
{
  io_keycode_t code = (io_keycode_t) hashtable_lookup(gIoReverseKeySymMap, key);
  return code;
}

io_axis_t
ioGetAxisByName(const char *key)
{
  io_axis_t axis = (io_axis_t)hashtable_lookup(gIoAxisNameMap, key);
  return axis;
}

const char*
ioGetAxisName(io_axis_t axis)
{
  return axisMap[axis];
}

io_axis_t
ioGetPhysAxisByName(const char *key)
{
  io_axis_t axis = (io_axis_t)hashtable_lookup(gIoPhysAxisNameMap, key);
  return axis;
}

const char*
ioGetPhysAxisName(io_axis_t axis)
{
  return physAxisMap[axis];
}

void
ioSetAxisEmu(io_axis_t axis, io_keycode_t plus, io_keycode_t minus)
{
  const char *axisName = ioGetAxisName(axis);
  ioSetKeyHandler(plus, axisName, (void*)0);
  ioSetKeyHandler(minus, axisName, (void*)1);
}

io_slider_t
ioGetSliderByName(const char *key)
{
  io_slider_t slider = (io_slider_t)hashtable_lookup(gIoVirtSliderNameMap, key);
  return slider;
}

const char*
ioGetSliderName(io_slider_t slider)
{
  return sliderMap[slider];
}

void
ioSetSliderEmu(io_slider_t slider, io_keycode_t plus, io_keycode_t minus)
{
  ioSetKeyHandler(plus, "throttle", (void*)0);
  ioSetKeyHandler(minus, "throttle", (void*)1);
}


void
io_builtin_hat_emu_dispatch(int state, void *data)
{
  IObuttonhandler *button = data;
  IObuttonhandler *secondary = button->data1;
  int val = (int)button->data2;

  if (state) {
    secondary->cHandler(val, NULL);
  } else {
    secondary->cHandler(-1, NULL);
  }
}

void
ioSetHatEmu(const char *action, int keycount, io_keycode_t keys[keycount])
{
  IObuttonhandler *keyHandler = hashtable_lookup(gIoButtonHandlers, action);
  if (keyHandler) {
    if (keyHandler->kind == IO_BUTTON_HAT) {
      // Normal state is degrees from north, steps are 360/keycount
      int step_size = 360 / keycount;

      for (int i = 0 ; i < keycount ; i++) {
        ioSetKeyHandler2(keys[i], io_builtin_hat_emu_dispatch,
                         keyHandler, (void*)(step_size*i));

      }
    } else if (keyHandler->kind == IO_BUTTON_MULTI) {
      // Normal state should be [0..keycount)
      for (int i = 0 ; i < keycount ; i++) {
        ioSetKeyHandler2(keys[i], io_builtin_hat_emu_dispatch,
                         keyHandler, (void*)i);

      }
    }
  }
}

void
ioInitKeys(void)
{
  OOconfarr *keys = ooConfGetNamedArr("openorbit/controls/keys");
  int len = ooConfGetArrLen(keys);
  for (int i = 0 ; i < len ; i++) {
    OOconfnode *keynode = ooConfGetArrObj(keys, i);
    const char *kind = ooConfGetStrByName(keynode, "kind");
    if (!strcmp(kind, "normal")) {
      const char *key = ooConfGetStrByName(keynode, "key");
      const char *action = ooConfGetStrByName(keynode, "action");
      io_keycode_t kc = ioGetKeyCode(key);
      ioSetKeyHandler(kc, action, NULL);
    } else if (!strcmp(kind, "hat")) {
      OOconfarr *keyarr = ooConfGetArrByName(keynode, "keys");
      const char *action = ooConfGetStrByName(keynode, "action");
      int keyCount = ooConfGetArrLen(keyarr);
      io_keycode_t keys[keyCount];
      for (int i = 0 ; i < keyCount ; i++) {
        const char *key = ooConfGetArrStr(keyarr, i);
        io_keycode_t kc = ioGetKeyCode(key);
        keys[i] = kc;
      }
      ioSetHatEmu(action, keyCount, keys);
    } else if (!strcmp(kind, "axis")) {
      OOconfarr *keyarr = ooConfGetArrByName(keynode, "keys");
      const char *action = ooConfGetStrByName(keynode, "action");
      int keyCount = ooConfGetArrLen(keyarr);
      if (keyCount != 2) {
        ooLogError("to many keys for axis emulation");
        goto next;
      }
      const char *key0 = ooConfGetArrStr(keyarr, 0);
      const char *key1 = ooConfGetArrStr(keyarr, 0);
      io_keycode_t kc0 = ioGetKeyCode(key0);
      io_keycode_t kc1 = ioGetKeyCode(key1);
      io_axis_t axis = ioGetAxisByName(action);
      ioSetAxisEmu(axis, kc0, kc1);
    } else if (!strcmp(kind, "slider")) {
      OOconfarr *keyarr = ooConfGetArrByName(keynode, "keys");
      const char *action = ooConfGetStrByName(keynode, "action");
      int keyCount = ooConfGetArrLen(keyarr);
      if (keyCount != 2) {
        ooLogError("to many keys for slider emulation");
        goto next;
      }
      const char *key0 = ooConfGetArrStr(keyarr, 0);
      const char *key1 = ooConfGetArrStr(keyarr, 0);
      io_keycode_t kc0 = ioGetKeyCode(key0);
      io_keycode_t kc1 = ioGetKeyCode(key1);
      io_slider_t slider = ioGetSliderByName(action);
      ioSetSliderEmu(slider, kc0, kc1);
    }
  next:
    ooConfNodeDispose(keynode);
  }
}

void
ioDispatchKeyUp(io_keycode_t key, uint16_t mask)
{
  IOkeymod kmod = OO_None;
  if (mask == OO_IO_MOD_NONE) {
    kmod = OO_None;
  } else if (mask & OO_IO_MOD_LSHIFT) {
    kmod = OO_L_Shift;
  } else if (mask & OO_IO_MOD_RSHIFT) {
    kmod = OO_R_Shift;
  } else if (mask & OO_IO_MOD_LCTRL) {
    kmod = OO_L_Ctrl;
  } else if (mask & OO_IO_MOD_RCTRL) {
    kmod = OO_R_Ctrl;
  } else if (mask & OO_IO_MOD_LALT ) {
    kmod = OO_L_Alt;
  } else if (mask & OO_IO_MOD_RALT ) {
    kmod = OO_R_Alt;
  } else if (mask & OO_IO_MOD_LMETA) {
    kmod = OO_L_Cmd;
  } else if (mask & OO_IO_MOD_RMETA) {
    kmod = OO_R_Cmd;
  }

  if (gIoKeyData[key].up[kmod].isScript) {

  } else {
    gIoKeyData[key].up[kmod].cHandler(false, gIoKeyData[key].up[kmod].data);
  }
}

void
ioDispatchKeyDown(io_keycode_t key, uint16_t mask)
{
  IOkeymod kmod = OO_None;
  if (mask == OO_IO_MOD_NONE) {
    kmod = OO_None;
  } else if (mask & OO_IO_MOD_LSHIFT) {
    kmod = OO_L_Shift;
  } else if (mask & OO_IO_MOD_RSHIFT) {
    kmod = OO_R_Shift;
  } else if (mask & OO_IO_MOD_LCTRL) {
    kmod = OO_L_Ctrl;
  } else if (mask & OO_IO_MOD_RCTRL) {
    kmod = OO_R_Ctrl;
  } else if (mask & OO_IO_MOD_LALT ) {
    kmod = OO_L_Alt;
  } else if (mask & OO_IO_MOD_RALT ) {
    kmod = OO_R_Alt;
  } else if (mask & OO_IO_MOD_LMETA) {
    kmod = OO_L_Cmd;
  } else if (mask & OO_IO_MOD_RMETA) {
    kmod = OO_R_Cmd;
  }

  if (gIoKeyData[key].down[kmod].isScript) {

  } else {
    gIoKeyData[key].down[kmod].cHandler(true, gIoKeyData[key].down[kmod].data);
  }
}
#if 0
void
ioDispatchButtonDown(int dev, int button)
{
  if (gJoyButtons.joy[dev]->down[button].isScript) {

  } else {
    gJoyButtons.joy[dev]->down[button].cHandler(true, gJoyButtons.joy[dev]->down[button].data);
  }
}

void
ioDispatchButtonUp(int dev, int button)
{
  if (gJoyButtons.joy[dev]->up[button].isScript) {

  } else {
    gJoyButtons.joy[dev]->up[button].cHandler(false, gJoyButtons.joy[dev]->up[button].data);
  }
}
#endif
void
ioRegActionHandler(const char *name, IObuttonhandlerfunc handlerFunc, io_button_kind_t kind, void *data)
{
  IObuttonhandler *handler =
    (IObuttonhandler*)hashtable_lookup(gIoButtonHandlers, name);
  if (handler != NULL) {
    ooLogWarn("%s already registered as button handler", name);
    return;
  }

  handler = smalloc(sizeof(IObuttonhandler));
  handler->isScript = false;
  handler->cHandler = handlerFunc;
  handler->data = data;
  handler->kind = kind;
  hashtable_insert(gIoButtonHandlers, name, handler);
}


void
ooIoRegPyKeyHandler(const char *name, PyObject *handlerFunc)
{
  IObuttonhandler *handler
       = (IObuttonhandler*)hashtable_lookup(gIoButtonHandlers, name);
  if (handler != NULL) {
    ooLogWarn("%s already registered as button handler", name);
    return;
  }

  handler = smalloc(sizeof(IObuttonhandler));
  handler->isScript = true;
  handler->pyHandler = handlerFunc;

  hashtable_insert(gIoButtonHandlers, name, handler);
}

void
ioSetKeyHandler(io_keycode_t key, const char *action, void *data)
{
  IObuttonhandler *keyHandler = hashtable_lookup(gIoButtonHandlers, action);
  gIoKeyData[key].down[OO_None] = *keyHandler;
  gIoKeyData[key].down[OO_None].data = data;

  gIoKeyData[key].up[OO_None] = *keyHandler;
  gIoKeyData[key].up[OO_None].data = data;
}

void
ioSetKeyHandler2(io_keycode_t key, IObuttonhandlerfunc keyHandler,
                 void *data0, void *data1)
{
  gIoKeyData[key].down[OO_None].cHandler = keyHandler;
  gIoKeyData[key].down[OO_None].data = &gIoKeyData[key].down[OO_None];
  gIoKeyData[key].down[OO_None].data1 = data0;
  gIoKeyData[key].down[OO_None].data2 = data1;


  gIoKeyData[key].up[OO_None].cHandler = keyHandler;
  gIoKeyData[key].up[OO_None].data = &gIoKeyData[key].up[OO_None];
  gIoKeyData[key].up[OO_None].data1 = data0;
  gIoKeyData[key].up[OO_None].data2 = data1;
}



void
ooIoBindKeyHandler(const char *keyName, const char *keyAction, int up, uint16_t mask)
{
  uintptr_t key_id = (uintptr_t) hashtable_lookup(gIoReverseKeySymMap, keyName);
  IOkeymod kmod = OO_None;
  if (mask == OO_IO_MOD_NONE) {
    kmod = OO_None;
  } else if (mask & OO_IO_MOD_LSHIFT) {
    kmod = OO_L_Shift;
  } else if (mask & OO_IO_MOD_RSHIFT) {
    kmod = OO_R_Shift;
  } else if (mask & OO_IO_MOD_LCTRL) {
    kmod = OO_L_Ctrl;
  } else if (mask & OO_IO_MOD_RCTRL) {
    kmod = OO_R_Ctrl;
  } else if (mask & OO_IO_MOD_LALT ) {
    kmod = OO_L_Alt;
  } else if (mask & OO_IO_MOD_RALT ) {
    kmod = OO_R_Alt;
  } else if (mask & OO_IO_MOD_LMETA) {
    kmod = OO_L_Cmd;
  } else if (mask & OO_IO_MOD_RMETA) {
    kmod = OO_R_Cmd;
  }

  IObuttonhandler *keyHandler = hashtable_lookup(gIoButtonHandlers, keyAction);

  if (keyHandler == NULL) {
    ooLogWarn("%s not found in button handler dictionary", keyAction);
    return;
  }


  if (keyHandler) {
    if (key_id == 0) {
      ooLogWarn("got key id 0 for %s", keyName);
    }
    if (up) {
      gIoKeyData[key_id].up[kmod] = *keyHandler;
    } else {
      gIoKeyData[key_id].down[kmod] = *keyHandler;
    }
  }
}

void
ioInitJoysticks(void)
{
  OOconfarr *joyArr = ooConfGetNamedArr("openorbit/controls/joystick");
  int len = ooConfGetArrLen(joyArr);
  for (int i = 0 ; i < len ; i++) {
    OOconfnode *node = ooConfGetArrObj(joyArr, i);
    const char *joyName = ooConfGetStrByName(node, "name");
    int joyId = ooConfGetIntByName(node, "id");

    if (!strcmp(joyName, "default")) {
      // Default joystick assignment, special case
    } else {
      int device = ioGetNamedDevice(joyId, NULL, joyName);
      OOconfarr *axises = ooConfGetArrByName(node, "axises");
      int axisLen = ooConfGetArrLen(axises);
      for (int i = 0 ; i < axisLen ; i ++) {
        const char *axisBinding = ooConfGetArrStr(axises, i);
        io_axis_t vaxis = ioGetAxisByName(axisBinding);
        ioBindDeviceAxis(device, i, vaxis);
      }

      OOconfarr *sliders = ooConfGetArrByName(node, "sliders");
      int sliderLen = ooConfGetArrLen(sliders);
      for (int i = 0 ; i < sliderLen ; i ++) {
        const char *sliderBinding = ooConfGetArrStr(sliders, i);
        io_slider_t vslider = ioGetSliderByName(sliderBinding);
        ioBindDeviceSlider(device, i, vslider);
      }

      OOconfarr *buttons = ooConfGetArrByName(node, "buttons");
      int buttonLen = ooConfGetArrLen(buttons);
      for (int i = 0 ; i < buttonLen ; i ++) {
        const char *buttonBinding = ooConfGetArrStr(buttons, i);
        ioBindDeviceButton(device, i, buttonBinding);
      }

      OOconfarr *hats = ooConfGetArrByName(node, "hats");
      int hatLen = ooConfGetArrLen(hats);
      for (int i = 0 ; i < hatLen ; i ++) {
        const char *hatBinding = ooConfGetArrStr(hats, i);
        ioBindDeviceHat(device, i, hatBinding);
      }

    }
    ooConfNodeDispose(node);
  }
}

static float _axisVals[IO_AXIS_COUNT];

void
ioPhysicalAxisChanged(int dev_id, io_axis_t axis, float val)
{
  io_device_info_t *dev = obj_array_get(&devices, dev_id);
  io_axis_t vaxis = dev->axis_map[axis];
  ioAxisChanged(vaxis, val);
}


void
ioAxisChanged(io_axis_t axis, float val)
{
  assert(-1.0f <= val && val <= 1.0f);
  _axisVals[axis] = val;
}

float
ioGetAxis(io_axis_t axis)
{
  return _axisVals[axis];
}


static float _sliderVals[IO_SLIDER_COUNT];

void
ioPhysicalSliderChanged(int dev_id, io_slider_t slider, float val)
{
  io_device_info_t *dev = obj_array_get(&devices, dev_id);
  io_slider_t vslider = dev->slider_map[slider];
  ioSliderChanged(vslider, val);
}

void
ioSliderChanged(io_slider_t slider, float val)
{
  assert(0.0f <= val && val <= 1.0f);
  _sliderVals[slider] = val;
}

float
ioGetSlider(io_slider_t slider)
{
  return _sliderVals[slider];
}


int
ioRegisterDevice(int vendorID, const char *vendorName,
                 int productID, const char *productName,
                 int buttonCount, int hatCount)
{
  io_device_info_t *dev = smalloc(sizeof(io_device_info_t));
  dev->vendorName = strdup(vendorName);
  dev->productName = strdup(productName);
  dev->vendorID = vendorID;
  dev->productID = productID;
  dev->buttons = buttonCount;
  dev->buttonHandler = calloc(buttonCount, sizeof(IObuttonhandler));
  dev->hats = hatCount;
  dev->hatHandler = calloc(hatCount, sizeof(IObuttonhandler));

  for (int i = 0 ; i < IO_AXIS_COUNT ; i++) {
    dev->axis_map[i] = i;
  }
  for (int i = 0 ; i < IO_SLIDER_COUNT ; i++) {
    dev->slider_map[i] = i;
  }

  ooLogInfo("registering device: vendor = %s, product = %s, buttons = %d, "
            "hats = %d",
            vendorName, productName, buttonCount, hatCount);

  // Bind all buttons to no button
  for (int i = 0 ; i < buttonCount ; i++) {
    dev->buttonHandler[i].cHandler = ooButtonHandlerGnd;
  }

  for (int i = 0 ; i < hatCount ; i++) {
    dev->hatHandler[i].cHandler = ooButtonHandlerGnd;
  }

  // Lookup button assignments
  obj_array_push(&devices, dev);

  // Reload joystick config
  ioInitJoysticks();

  return devices.length-1; // Return unique device code
}

int
ioGetNamedDevice(int pos, const char *vendorName, const char *productName)
{
  // No vendor or product set
  if (!vendorName && !productName) {
    return -1;
  }

  int reached_pos = 0;
  ARRAY_FOR_EACH(i, devices) {
    io_device_info_t *dev = ARRAY_ELEM(devices, i);
    if (dev) {
      bool vendorNameMatches = true;
      if (vendorName) {
        if (strcmp(vendorName, dev->vendorName)) vendorNameMatches = false;
      }
      bool productNameMatches = true;
      if (productName) {
        if (strcmp(productName, dev->productName)) productNameMatches = false;
      }

      if (vendorNameMatches && productNameMatches) {
        if (reached_pos == pos) {
          return i;
        }
        reached_pos ++;
      }
    }
  }

  // No device found
  return -1;
}

void
ioRemoveDevice(int deviceID)
{
  io_device_info_t *dev = obj_array_get(&devices, deviceID);
  free(dev->vendorName);
  free(dev->productName);
  free(dev->buttonHandler);
  free(dev);
  obj_array_set(&devices, deviceID, NULL);
}

void
ioDeviceButtonDown(int deviceID, int button)
{
  io_device_info_t *dev = obj_array_get(&devices, deviceID);
  if (dev->buttonHandler[button-1].isScript) {
    ooLogInfo("is script...");
  } else {
    dev->buttonHandler[button-1].cHandler(true, dev->buttonHandler[button-1].data);
  }
}

void
ioDeviceButtonUp(int deviceID, int button)
{
  io_device_info_t *dev = obj_array_get(&devices, deviceID);
  if (dev->buttonHandler[button-1].isScript) {

  } else {
    dev->buttonHandler[button-1].cHandler(false, dev->buttonHandler[button-1].data);
  }
}

void
ioDeviceHatSet(int deviceID, int hat_id, int state, int dir)
{
  io_device_info_t *dev = obj_array_get(&devices, deviceID);
  IObuttonhandler *handler = &dev->hatHandler[hat_id];

  if (handler->kind == IO_BUTTON_MULTI) {

    if (state < 0) {
      if (handler->isScript) {
      } else {
        handler->cHandler(-1, handler->data);
      }
    } else {
      if (handler->isScript) {
      } else {
        handler->cHandler(state, handler->data);
      }

    }
  } else if (handler->kind == IO_BUTTON_HAT) {
    if (dir < 0) {
      if (handler->isScript) {
      } else {
        handler->cHandler(-1, handler->data);
      }
    } else {
      if (handler->isScript) {
      } else {
        handler->cHandler(dir, handler->data);
      }
    }
  }
}

void
ioBindDeviceButton(int deviceID, int button, const char *key)
{
  if (deviceID == -1) {
    // TODO: Handle default values
    return;
  }

  io_device_info_t *dev = obj_array_get(&devices, deviceID);

  if (button < 0 || dev->buttons <= button) {
    ooLogWarn("device '%s' cannot handle button id %d",
              dev->productName, button);
    return;
  }

  IObuttonhandler *handler
    = (IObuttonhandler*)hashtable_lookup(gIoButtonHandlers, key);
  if (handler == NULL) {
    if (!strcmp(key, "default")) {
      dev->buttonHandler[button].isScript = false;
      dev->buttonHandler[button].cHandler = ooButtonHandlerGnd;
      dev->buttonHandler[button].data = NULL;
      dev->buttonHandler[button].kind = IO_BUTTON_PUSH;
      return;
    }

    ooLogWarn("could not find button handler '%s'", key);
    return;
  }

  if (handler->kind == IO_BUTTON_PUSH) {
    dev->buttonHandler[button] = *handler;
  } else {
    ooLogWarn("cannot bind multi value handler to simple push button");
  }
}

void
ioBindDeviceAxis(int deviceID, io_axis_t phys_axis, io_axis_t virt_axis)
{
  if (deviceID == -1) {
    // TODO: Handle default values
  } else {
    io_device_info_t *dev = obj_array_get(&devices, deviceID);
    dev->axis_map[phys_axis] = virt_axis;
  }
}

void
ioBindDeviceSlider(int deviceID, io_slider_t phys_slider,
                   io_slider_t virt_slider)
{
  if (deviceID == -1) {
    // TODO: Handle default values
  } else {
    io_device_info_t *dev = obj_array_get(&devices, deviceID);
    dev->slider_map[phys_slider] = virt_slider;
  }
}

void
ioBindDeviceHat(int deviceID, int hat, const char *key)
{
  if (deviceID == -1) {
    // TODO: Handle default values
    return;
  }
  io_device_info_t *dev = obj_array_get(&devices, deviceID);

  if (hat < 0 || dev->hats <= hat) {
    ooLogWarn("device '%s' cannot handle hat id %d",
              dev->productName, hat);
    return;
  }

  IObuttonhandler *handler
    = (IObuttonhandler*)hashtable_lookup(gIoButtonHandlers, key);
  if (handler == NULL) {
    if (!strcmp(key, "default")) {
      dev->hatHandler[hat].isScript = false;
      dev->hatHandler[hat].cHandler = ooButtonHandlerGnd;
      dev->hatHandler[hat].data = NULL;
      dev->hatHandler[hat].kind = IO_BUTTON_HAT;
      return;
    }
    ooLogWarn("could not find button handler '%s'", key);
    return;
  }

  if (handler->kind == IO_BUTTON_MULTI || handler->kind == IO_BUTTON_HAT) {
    ooLogInfo("binding hat %d to %s (kind = %d)", hat, key, handler->kind);
    dev->hatHandler[hat] = *handler;
  } else {
    ooLogWarn("cannot bind push button handler to hat switch");
  }
}
