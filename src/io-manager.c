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

void io_set_key_handler2(io_keycode_t key, IObuttonhandlerfunc keyHandler,
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
  IObuttonhandler up[IO_MOD_KEY_COUNT];
  IObuttonhandler down[IO_MOD_KEY_COUNT];
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
io_get_mouse_pos(float *x, float *y)
{
  platform_get_mouse(x, y);
}

void io_init_keys(void);

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
    io_axis_changed(IO_AXIS_RY, 0.0f);
    return;
  }

  if (data) {
    io_axis_changed(IO_AXIS_RY, -1.0);
  } else {
    io_axis_changed(IO_AXIS_RY, 1.0);
  }
}

void
io_builtin_roll(int state, void *data)
{
  if (!state) {
    io_axis_changed(IO_AXIS_RZ, 0.0f);
    return;
  }

  if (data) {
    io_axis_changed(IO_AXIS_RZ, -1.0);
  } else {
    io_axis_changed(IO_AXIS_RZ, 1.0);
  }
}

void
io_builtin_pitch(int state, void *data)
{
  if (!state) {
    io_axis_changed(IO_AXIS_RX, 0.0f);
    return;
  }

  if (data) {
    io_axis_changed(IO_AXIS_RX, -1.0);
  } else {
    io_axis_changed(IO_AXIS_RX, 1.0);
  }
}

void
io_builtin_vertical(int state, void *data)
{
  if (!state) {
    io_axis_changed(IO_AXIS_Y, 0.0f);
    return;
  }

  if (data) {
    io_axis_changed(IO_AXIS_Y, -1.0);
  } else {
    io_axis_changed(IO_AXIS_Y, 1.0);
  }
}

void
io_builtin_lateral(int state, void *data)
{
  if (!state) {
    io_axis_changed(IO_AXIS_X, 0.0f);
    return;
  }

  if (data) {
    io_axis_changed(IO_AXIS_X, -1.0);
  } else {
    io_axis_changed(IO_AXIS_X, 1.0);
  }
}

void
io_builtin_forward(int state, void *data)
{
  if (!state) {
    io_axis_changed(IO_AXIS_Z, 0.0f);
    return;
  }

  if (data) {
    io_axis_changed(IO_AXIS_Z, -1.0);
  } else {
    io_axis_changed(IO_AXIS_Z, 1.0);
  }
}


void
io_builtin_throttle(int state, void *data)
{
  if (data) {
    float newVal = io_get_slider(IO_SLIDER_THROT_0) - 0.1f;
    newVal = newVal < 0.0f ? 0.0f :newVal;
    io_slider_changed(IO_SLIDER_THROT_0, io_get_slider(IO_SLIDER_THROT_0));
  } else {
    float newVal = io_get_slider(IO_SLIDER_THROT_0) + 0.1f;
    newVal = newVal > 1.0f ? 1.0f :newVal;
    io_slider_changed(IO_SLIDER_THROT_0, io_get_slider(IO_SLIDER_THROT_0));
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
    for (size_t j = 0 ; j < IO_MOD_KEY_COUNT ; ++ j) {
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
//  gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_L_CMD].isScript = false;
//  gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_L_CMD].cHandler = ioQuitHandler;
//  gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_L_CMD].data = NULL;
//  gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_R_CMD].isScript = false;
//  gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_R_CMD].cHandler = ioQuitHandler;
//  gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_R_CMD].data = NULL;

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
  io_reg_action_handler("yaw", io_builtin_yaw, IO_BUTTON_MULTI, NULL);
  io_reg_action_handler("roll", io_builtin_roll, IO_BUTTON_MULTI, NULL);
  io_reg_action_handler("pitch", io_builtin_pitch, IO_BUTTON_MULTI, NULL);
  io_reg_action_handler("lateral", io_builtin_lateral, IO_BUTTON_MULTI, NULL);
  io_reg_action_handler("vertical", io_builtin_vertical, IO_BUTTON_MULTI, NULL);
  io_reg_action_handler("forward", io_builtin_forward, IO_BUTTON_MULTI, NULL);

  // Slider emulation
  io_reg_action_handler("throttle", io_builtin_throttle, IO_BUTTON_MULTI, NULL);
}

void
io_init(void)
{

  // Now initialize joysticks
  //ioInitJoysticks();

  // Init key data
  io_init_keys();
  io_init_joysticks();


  // Hook up the quit handler as default to cmd / meta q, this overrides the initkeys conf
 // gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_L_CMD].isScript = false;
 // gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_L_CMD].cHandler = ioQuitHandler;
 // gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_L_CMD].data = NULL;
 // gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_R_CMD].isScript = false;
 // gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_R_CMD].cHandler = ioQuitHandler;
 // gIoKeyData[SDL_SCANCODE_Q].down[IO_MOD_R_CMD].data = NULL;
}

io_keycode_t
io_get_key_code(const char *key)
{
  io_keycode_t code = (io_keycode_t) hashtable_lookup(gIoReverseKeySymMap, key);
  return code;
}

io_axis_t
io_get_axis_by_name(const char *key)
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
io_set_axis_emu(io_axis_t axis, io_keycode_t plus, io_keycode_t minus)
{
  const char *axisName = ioGetAxisName(axis);
  io_set_key_handler(plus, axisName, (void*)0);
  io_set_key_handler(minus, axisName, (void*)1);
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
  io_set_key_handler(plus, "throttle", (void*)0);
  io_set_key_handler(minus, "throttle", (void*)1);
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
        io_set_key_handler2(keys[i], io_builtin_hat_emu_dispatch,
                         keyHandler, (void*)(step_size*i));

      }
    } else if (keyHandler->kind == IO_BUTTON_MULTI) {
      // Normal state should be [0..keycount)
      for (int i = 0 ; i < keycount ; i++) {
        io_set_key_handler2(keys[i], io_builtin_hat_emu_dispatch,
                         keyHandler, (void*)i);

      }
    }
  }
}

void
io_init_keys(void)
{
  config_arr_t *keys = config_get_named_arr("openorbit/controls/keys");
  int len = config_get_arr_len(keys);
  for (int i = 0 ; i < len ; i++) {
    config_node_t *keynode = config_get_arr_obj(keys, i);
    const char *kind = config_get_str_by_name(keynode, "kind");
    if (!strcmp(kind, "normal")) {
      const char *key = config_get_str_by_name(keynode, "key");
      const char *action = config_get_str_by_name(keynode, "action");
      io_keycode_t kc = io_get_key_code(key);
      io_set_key_handler(kc, action, NULL);
    } else if (!strcmp(kind, "hat")) {
      config_arr_t *keyarr = config_get_arr_by_name(keynode, "keys");
      const char *action = config_get_str_by_name(keynode, "action");
      int keyCount = config_get_arr_len(keyarr);
      io_keycode_t keys[keyCount];
      for (int i = 0 ; i < keyCount ; i++) {
        const char *key = config_get_arr_str(keyarr, i);
        io_keycode_t kc = io_get_key_code(key);
        keys[i] = kc;
      }
      ioSetHatEmu(action, keyCount, keys);
    } else if (!strcmp(kind, "axis")) {
      config_arr_t *keyarr = config_get_arr_by_name(keynode, "keys");
      const char *action = config_get_str_by_name(keynode, "action");
      int keyCount = config_get_arr_len(keyarr);
      if (keyCount != 2) {
        ooLogError("to many keys for axis emulation");
        goto next;
      }
      const char *key0 = config_get_arr_str(keyarr, 0);
      const char *key1 = config_get_arr_str(keyarr, 0);
      io_keycode_t kc0 = io_get_key_code(key0);
      io_keycode_t kc1 = io_get_key_code(key1);
      io_axis_t axis = io_get_axis_by_name(action);
      io_set_axis_emu(axis, kc0, kc1);
    } else if (!strcmp(kind, "slider")) {
      config_arr_t *keyarr = config_get_arr_by_name(keynode, "keys");
      const char *action = config_get_str_by_name(keynode, "action");
      int keyCount = config_get_arr_len(keyarr);
      if (keyCount != 2) {
        ooLogError("to many keys for slider emulation");
        goto next;
      }
      const char *key0 = config_get_arr_str(keyarr, 0);
      const char *key1 = config_get_arr_str(keyarr, 0);
      io_keycode_t kc0 = io_get_key_code(key0);
      io_keycode_t kc1 = io_get_key_code(key1);
      io_slider_t slider = ioGetSliderByName(action);
      ioSetSliderEmu(slider, kc0, kc1);
    }
  next:
    config_node_dispose(keynode);
  }
}

void
io_dispatch_key_up(io_keycode_t key, uint16_t mask)
{
  io_keymod_t kmod = IO_MOD_NONE;
  if (mask == OO_IO_MOD_NONE) {
    kmod = IO_MOD_NONE;
  } else if (mask & OO_IO_MOD_LSHIFT) {
    kmod = IO_MOD_L_SHIFT;
  } else if (mask & OO_IO_MOD_RSHIFT) {
    kmod = IO_MOD_R_SHIFT;
  } else if (mask & OO_IO_MOD_LCTRL) {
    kmod = IO_MOD_L_CTRL;
  } else if (mask & OO_IO_MOD_RCTRL) {
    kmod = IO_MOD_R_CTRL;
  } else if (mask & OO_IO_MOD_LALT ) {
    kmod = IO_MOD_L_ALT;
  } else if (mask & OO_IO_MOD_RALT ) {
    kmod = IO_MOD_R_ALT;
  } else if (mask & OO_IO_MOD_LMETA) {
    kmod = IO_MOD_L_CMD;
  } else if (mask & OO_IO_MOD_RMETA) {
    kmod = IO_MOD_R_CMD;
  }

  if (gIoKeyData[key].up[kmod].isScript) {

  } else {
    gIoKeyData[key].up[kmod].cHandler(false, gIoKeyData[key].up[kmod].data);
  }
}

void
io_dispatch_key_down(io_keycode_t key, uint16_t mask)
{
  io_keymod_t kmod = IO_MOD_NONE;
  if (mask == OO_IO_MOD_NONE) {
    kmod = IO_MOD_NONE;
  } else if (mask & OO_IO_MOD_LSHIFT) {
    kmod = IO_MOD_L_SHIFT;
  } else if (mask & OO_IO_MOD_RSHIFT) {
    kmod = IO_MOD_R_SHIFT;
  } else if (mask & OO_IO_MOD_LCTRL) {
    kmod = IO_MOD_L_CTRL;
  } else if (mask & OO_IO_MOD_RCTRL) {
    kmod = IO_MOD_R_CTRL;
  } else if (mask & OO_IO_MOD_LALT ) {
    kmod = IO_MOD_L_ALT;
  } else if (mask & OO_IO_MOD_RALT ) {
    kmod = IO_MOD_R_ALT;
  } else if (mask & OO_IO_MOD_LMETA) {
    kmod = IO_MOD_L_CMD;
  } else if (mask & OO_IO_MOD_RMETA) {
    kmod = IO_MOD_R_CMD;
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
io_reg_action_handler(const char *name, IObuttonhandlerfunc handlerFunc, io_button_kind_t kind, void *data)
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
io_reg_py_key_handler(const char *name, PyObject *handlerFunc)
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
io_set_key_handler(io_keycode_t key, const char *action, void *data)
{
  IObuttonhandler *keyHandler = hashtable_lookup(gIoButtonHandlers, action);
  gIoKeyData[key].down[IO_MOD_NONE] = *keyHandler;
  gIoKeyData[key].down[IO_MOD_NONE].data = data;

  gIoKeyData[key].up[IO_MOD_NONE] = *keyHandler;
  gIoKeyData[key].up[IO_MOD_NONE].data = data;
}

void
io_set_key_handler2(io_keycode_t key, IObuttonhandlerfunc keyHandler,
                 void *data0, void *data1)
{
  gIoKeyData[key].down[IO_MOD_NONE].cHandler = keyHandler;
  gIoKeyData[key].down[IO_MOD_NONE].data = &gIoKeyData[key].down[IO_MOD_NONE];
  gIoKeyData[key].down[IO_MOD_NONE].data1 = data0;
  gIoKeyData[key].down[IO_MOD_NONE].data2 = data1;


  gIoKeyData[key].up[IO_MOD_NONE].cHandler = keyHandler;
  gIoKeyData[key].up[IO_MOD_NONE].data = &gIoKeyData[key].up[IO_MOD_NONE];
  gIoKeyData[key].up[IO_MOD_NONE].data1 = data0;
  gIoKeyData[key].up[IO_MOD_NONE].data2 = data1;
}



void
io_bind_key_handler(const char *keyName, const char *keyAction, int up, uint16_t mask)
{
  uintptr_t key_id = (uintptr_t) hashtable_lookup(gIoReverseKeySymMap, keyName);
  io_keymod_t kmod = IO_MOD_NONE;
  if (mask == OO_IO_MOD_NONE) {
    kmod = IO_MOD_NONE;
  } else if (mask & OO_IO_MOD_LSHIFT) {
    kmod = IO_MOD_L_SHIFT;
  } else if (mask & OO_IO_MOD_RSHIFT) {
    kmod = IO_MOD_R_SHIFT;
  } else if (mask & OO_IO_MOD_LCTRL) {
    kmod = IO_MOD_L_CTRL;
  } else if (mask & OO_IO_MOD_RCTRL) {
    kmod = IO_MOD_R_CTRL;
  } else if (mask & OO_IO_MOD_LALT ) {
    kmod = IO_MOD_L_ALT;
  } else if (mask & OO_IO_MOD_RALT ) {
    kmod = IO_MOD_R_ALT;
  } else if (mask & OO_IO_MOD_LMETA) {
    kmod = IO_MOD_L_CMD;
  } else if (mask & OO_IO_MOD_RMETA) {
    kmod = IO_MOD_R_CMD;
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
io_init_joysticks(void)
{
  config_arr_t *joyArr = config_get_named_arr("openorbit/controls/joystick");
  int len = config_get_arr_len(joyArr);
  for (int i = 0 ; i < len ; i++) {
    config_node_t *node = config_get_arr_obj(joyArr, i);
    const char *joyName = config_get_str_by_name(node, "name");
    int joyId = config_get_int_by_name(node, "id");

    if (!strcmp(joyName, "default")) {
      // Default joystick assignment, special case
    } else {
      int device = io_get_named_device(joyId, NULL, joyName);
      config_arr_t *axises = config_get_arr_by_name(node, "axises");
      int axisLen = config_get_arr_len(axises);
      for (int i = 0 ; i < axisLen ; i ++) {
        const char *axisBinding = config_get_arr_str(axises, i);
        io_axis_t vaxis = io_get_axis_by_name(axisBinding);
        io_bind_device_axis(device, i, vaxis);
      }

      config_arr_t *sliders = config_get_arr_by_name(node, "sliders");
      int sliderLen = config_get_arr_len(sliders);
      for (int i = 0 ; i < sliderLen ; i ++) {
        const char *sliderBinding = config_get_arr_str(sliders, i);
        io_slider_t vslider = ioGetSliderByName(sliderBinding);
        io_bind_device_slider(device, i, vslider);
      }

      config_arr_t *buttons = config_get_arr_by_name(node, "buttons");
      int buttonLen = config_get_arr_len(buttons);
      for (int i = 0 ; i < buttonLen ; i ++) {
        const char *buttonBinding = config_get_arr_str(buttons, i);
        io_bind_device_button(device, i, buttonBinding);
      }

      config_arr_t *hats = config_get_arr_by_name(node, "hats");
      int hatLen = config_get_arr_len(hats);
      for (int i = 0 ; i < hatLen ; i ++) {
        const char *hatBinding = config_get_arr_str(hats, i);
        io_bind_device_hat(device, i, hatBinding);
      }

    }
    config_node_dispose(node);
  }
}

static float _axisVals[IO_AXIS_COUNT];

void
io_physical_axis_changed(int dev_id, io_axis_t axis, float val)
{
  io_device_info_t *dev = obj_array_get(&devices, dev_id);
  io_axis_t vaxis = dev->axis_map[axis];
  io_axis_changed(vaxis, val);
}


void
io_axis_changed(io_axis_t axis, float val)
{
  assert(-1.0f <= val && val <= 1.0f);
  _axisVals[axis] = val;
}

float
io_get_axis(io_axis_t axis)
{
  return _axisVals[axis];
}


static float _sliderVals[IO_SLIDER_COUNT];

void
io_physical_slider_changed(int dev_id, io_slider_t slider, float val)
{
  io_device_info_t *dev = obj_array_get(&devices, dev_id);
  io_slider_t vslider = dev->slider_map[slider];
  io_slider_changed(vslider, val);
}

void
io_slider_changed(io_slider_t slider, float val)
{
  assert(0.0f <= val && val <= 1.0f);
  _sliderVals[slider] = val;
}

float
io_get_slider(io_slider_t slider)
{
  return _sliderVals[slider];
}


int
io_register_device(int vendorID, const char *vendorName,
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
  io_init_joysticks();

  return devices.length-1; // Return unique device code
}

int
io_get_named_device(int pos, const char *vendorName, const char *productName)
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
io_remove_device(int deviceID)
{
  io_device_info_t *dev = obj_array_get(&devices, deviceID);
  free(dev->vendorName);
  free(dev->productName);
  free(dev->buttonHandler);
  free(dev);
  obj_array_set(&devices, deviceID, NULL);
}

void
io_device_button_down(int deviceID, int button)
{
  io_device_info_t *dev = obj_array_get(&devices, deviceID);
  if (dev->buttonHandler[button-1].isScript) {
    ooLogInfo("is script...");
  } else {
    dev->buttonHandler[button-1].cHandler(true, dev->buttonHandler[button-1].data);
  }
}

void
io_device_button_up(int deviceID, int button)
{
  io_device_info_t *dev = obj_array_get(&devices, deviceID);
  if (dev->buttonHandler[button-1].isScript) {

  } else {
    dev->buttonHandler[button-1].cHandler(false, dev->buttonHandler[button-1].data);
  }
}

void
io_device_hat_set(int deviceID, int hat_id, int state, int dir)
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
io_bind_device_button(int deviceID, int button, const char *key)
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
io_bind_device_axis(int deviceID, io_axis_t phys_axis, io_axis_t virt_axis)
{
  if (deviceID == -1) {
    // TODO: Handle default values
  } else {
    io_device_info_t *dev = obj_array_get(&devices, deviceID);
    dev->axis_map[phys_axis] = virt_axis;
  }
}

void
io_bind_device_slider(int deviceID, io_slider_t phys_slider,
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
io_bind_device_hat(int deviceID, int hat, const char *key)
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
