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

  typedef enum IOkeymod {
    OO_None = 0,

    OO_L_Shift,
    OO_L_Ctrl,
    OO_L_Cmd,
    OO_L_Alt,

    OO_R_Shift,
    OO_R_Ctrl,
    OO_R_Cmd,
    OO_R_Alt,

    OO_Key_Mod_Count
  } IOkeymod;


#define OO_IO_MOD_NONE   (0)
#define OO_IO_MOD_LSHIFT (1 << OO_L_Shift)
#define OO_IO_MOD_RSHIFT (1 << OO_R_Shift)
#define OO_IO_MOD_LCTRL  (1 << OO_L_Ctrl)
#define OO_IO_MOD_RCTRL  (1 << OO_R_Ctrl)
#define OO_IO_MOD_LALT   (1 << OO_L_Alt)
#define OO_IO_MOD_RALT   (1 << OO_R_Alt)
#define OO_IO_MOD_LMETA  (1 << OO_L_Cmd)
#define OO_IO_MOD_RMETA  (1 << OO_R_Cmd)

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

// Axises are ranged in calibrated form from -1.0 to +1.0
typedef enum {
  IO_AXIS_X = 0,
  IO_AXIS_Y,
  IO_AXIS_Z,
  IO_AXIS_RX,
  IO_AXIS_RY,
  IO_AXIS_RZ,
  IO_AXIS_COUNT,
} io_axis_t;

// Sliders are like axises, but range is from 0.0 to 1.0
typedef enum {
  IO_SLIDER_THROT_0 = 0,
  IO_SLIDER_THROT_1,
  IO_SLIDER_COUNT,
} io_slider_t;


// Vector controls
typedef enum {
  IO_VEC_X,
  IO_VEC_Y,
  IO_VEC_Z,
  IO_VEC_BRX,
  IO_VEC_BRY,
  IO_VEC_BRZ,
  IO_VEC_NO,
} io_vec_t;

typedef enum {
  IO_BUTTON_PUSH, // 0 = depressed, 1 = pressed
  IO_BUTTON_MULTI, // -1 = depressed, 0..n are normal states, also used for switches that are never depressed
  IO_BUTTON_HAT, // Like multi, except values are degrees from north, -1 is depressed
} io_button_kind_t;

void ioSetKeyHandler(io_keycode_t key, const char *action, void *data);

void ioInit(void);
void ioInitKeys(void);
void ioInitJoysticks(void);

void ioDispatchKeyUp(io_keycode_t key, uint16_t mask);
void ioDispatchKeyDown(io_keycode_t key, uint16_t mask);
void ioBindKeyToAxis(const char *key, const char *button, float val);
void ioDispatchButtonDown(int dev, int button);
void ioDispatchButtonUp(int dev, int button);

void ioSetAxisEmu(io_axis_t axis, io_keycode_t plus, io_keycode_t minus);

void ioPhysicalAxisChanged(int dev_id, io_axis_t axis, float val);
void ioAxisChanged(io_axis_t axis, float val);
float ioGetAxis(io_axis_t axis);

void ioPhysicalSliderChanged(int dev_id, io_slider_t slider, float val);
void ioSliderChanged(io_slider_t slider, float val);
float ioGetSlider(io_slider_t slider);

int ioRegisterDevice(int vendorID, const char *vendorName,
                     int productID, const char *productName,
                     int buttonCount, int hatCount);
void ioRemoveDevice(int deviceID);

void ioBindDeviceSlider(int deviceID, io_slider_t phys_slider,
                        io_slider_t virt_slider);

void ioBindDeviceAxis(int deviceID, io_axis_t phys_axis, io_axis_t virt_axis);
void ioBindDeviceButton(int deviceID, int button, const char *key);
void ioBindDeviceHat(int deviceID, int button, const char *key);

void ioDeviceButtonDown(int deviceID, int button);
void ioDeviceButtonUp(int deviceID, int button);

// State will be 0..n
// Dir will be direction in degrees from up, if the hat is cleared
// dir will be -1
void ioDeviceHatSet(int deviceID, int hat_id, int state, int dir);

int ioGetNamedDevice(int pos, const char *vendorName, const char *productName);

// For buttons, 0 means depressed, 1 is down, further values can be
// issued by for example hat switches
typedef void (*IObuttonhandlerfunc)(int buttonVal, void *data);

void ioRegActionHandler(const char *name, IObuttonhandlerfunc handlerFunc, io_button_kind_t kind, void *data);

/*! Register key handler Python function */
void ooIoRegPyKeyHandler(const char *name, PyObject *handlerFunc);

/*! Bind keyboard with keyName to the keyAction key

    \param keyName String key for the keyboard button
    \param keyAction Key corresponding to the registered action
    \param up 0 if the handler should fire on button down, otherwise it will
        fire on button up.
    \param mask ored mask from the OO_IO_MOD* constants
*/
void ooIoBindKeyHandler(const char *keyName, const char *keyAction, int up,
                        uint16_t mask);

#ifdef __cplusplus
}
#endif

#endif /* ! __IO_MANAGER_H */
