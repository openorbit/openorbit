/*
  Copyright 2006,2008,2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <stdbool.h>
#include <stdio.h>
#include <err.h>
#include <gencds/hashtable.h>
#include "sim.h"
#include "io-manager.h"
//#include "SDL.h"
#include <SDL/SDL.h>
#include "log.h"
#include "parsers/hrml.h"
#include "settings.h"


static const char * gIoSdlMouseStringMap[7];
static hashtable_t *gIoButtonHandlers; // of type OObuttonhandler

static hashtable_t *gIoAxisHandlers; // of type OOaxishandler


#define SPEC_KEY_COUNT 8/* shft, cmd, ctrl, alt (left and right) */

void ioBindJoystickButton(const char *key, int joyStick, int button);
void ioBindButtonToAxis(const char *axisKey, int joystick, int button, float val);

void
ooButtonHandlerGnd(bool buttonDown, void *data)
{
    /*Nothing*/
}

typedef struct {
  bool isScript;
  union {
    OObuttonhandlerfunc cHandler;
    PyObject *pyHandler;
  };
  void *data;
} OObuttonhandler;


typedef struct OOaxishandler {
  SDL_Joystick *joyId;
  int axisId;
  float nullZone;
  float trim;

  bool buttonDown;
  float val;
} OOaxishandler;


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

typedef struct OOkeyhandler {
  OObuttonhandler up[OO_Key_Mod_Count];
  OObuttonhandler down[OO_Key_Mod_Count];
} OOkeyhandler;
OOkeyhandler gIoKeyData[SDL_NUM_SCANCODES];

#define MOUSE_BUTTONS 8
typedef struct OOmousebuttons {
  OObuttonhandler down[MOUSE_BUTTONS];
  OObuttonhandler up[MOUSE_BUTTONS];
} OOmousebuttons;
OOmousebuttons gIoMouseButtons;

typedef struct IOjbuttons {
  size_t buttonCount;
  OObuttonhandler *down;
  OObuttonhandler *up;
} IOjbuttons;

typedef struct IOjsticks {
  size_t joyCount;
  IOjbuttons **joy;
} IOjsticks;

IOjsticks gJoyButtons = {0, NULL};


#if 0
[SDL_BUTTON_LEFT-1]	= "mouse-left";
[SDL_BUTTON_MIDDLE-1] =	"mouse-middle";
[SDL_BUTTON_RIGHT-1] = "mouse-right";
[SDL_BUTTON_WHEELUP-1] = "mouse-up";
[SDL_BUTTON_WHEELDOWN-1] = "mouse-down";
[SDL_BUTTON_X1-1] = "mouse-x1";
[SDL_BUTTON_X2-1] = "mouse-x2";
#endif

static const char *keysymmap [SDL_NUM_SCANCODES] = {
  [SDL_SCANCODE_LSHIFT] = "lshift",
  [SDL_SCANCODE_RSHIFT] = "rshift",
  [SDL_SCANCODE_LGUI] = "lmeta",
  [SDL_SCANCODE_RGUI] = "rmeta",
  [SDL_SCANCODE_LCTRL] = "lctrl",
  [SDL_SCANCODE_RCTRL] = "rctrl",

  [SDL_SCANCODE_MODE] = "mode",
  [SDL_SCANCODE_HELP] = "help",
  [SDL_SCANCODE_SYSREQ] = "sys-req",
  [SDL_SCANCODE_CLEAR] = "clear",
  [SDL_SCANCODE_MENU] = "menu",
  [SDL_SCANCODE_POWER] = "power",

  [SDL_SCANCODE_RETURN] = "return",
  [SDL_SCANCODE_SPACE] = "space",
  [SDL_SCANCODE_TAB] = "tab",
  [SDL_SCANCODE_BACKSPACE] = "backspace",
  [SDL_SCANCODE_ESCAPE] = "esc",
  [SDL_SCANCODE_PERIOD] = ".",
  [SDL_SCANCODE_COMMA] = ",",
  [SDL_SCANCODE_PAUSE] = "pause",
  [SDL_SCANCODE_MINUS] = "-",
  [SDL_SCANCODE_SLASH] = "/",

  [SDL_SCANCODE_SEMICOLON] = ";",
  [SDL_SCANCODE_EQUALS] = "=",
  [SDL_SCANCODE_LEFTBRACKET] = "[",
  [SDL_SCANCODE_BACKSLASH] = "\\",
  [SDL_SCANCODE_RIGHTBRACKET] = "]",

  [SDL_SCANCODE_0] = "0",
  [SDL_SCANCODE_1] = "1",
  [SDL_SCANCODE_2] = "2",
  [SDL_SCANCODE_3] = "3",
  [SDL_SCANCODE_4] = "4",
  [SDL_SCANCODE_5] = "5",
  [SDL_SCANCODE_6] = "6",
  [SDL_SCANCODE_7] = "7",
  [SDL_SCANCODE_8] = "8",
  [SDL_SCANCODE_9] = "9",

  [SDL_SCANCODE_KP_0] = "kp 0",
  [SDL_SCANCODE_KP_1] = "kp 1",
  [SDL_SCANCODE_KP_2] = "kp 2",
  [SDL_SCANCODE_KP_3] = "kp 3",
  [SDL_SCANCODE_KP_4] = "kp 4",
  [SDL_SCANCODE_KP_5] = "kp 5",
  [SDL_SCANCODE_KP_6] = "kp 6",
  [SDL_SCANCODE_KP_7] = "kp 7",
  [SDL_SCANCODE_KP_8] = "kp 8",
  [SDL_SCANCODE_KP_9] = "kp 9",

  [SDL_SCANCODE_KP_PERIOD] = "kp .",
  [SDL_SCANCODE_KP_DIVIDE] = "kp /",
  [SDL_SCANCODE_KP_MULTIPLY] = "kp *",
  [SDL_SCANCODE_KP_MINUS] = "kp -",
  [SDL_SCANCODE_KP_PLUS] = "kp +",
  [SDL_SCANCODE_KP_ENTER] = "enter",
  [SDL_SCANCODE_KP_EQUALS] = "kp =",
  [SDL_SCANCODE_DELETE] = "delete",

  [SDL_SCANCODE_A] = "a",
  [SDL_SCANCODE_B] = "b",
  [SDL_SCANCODE_C] = "c",
  [SDL_SCANCODE_D] = "d",
  [SDL_SCANCODE_E] = "e",
  [SDL_SCANCODE_F] = "f",
  [SDL_SCANCODE_G] = "g",
  [SDL_SCANCODE_H] = "h",
  [SDL_SCANCODE_I] = "i",
  [SDL_SCANCODE_J] = "j",
  [SDL_SCANCODE_K] = "k",
  [SDL_SCANCODE_L] = "l",
  [SDL_SCANCODE_M] = "m",
  [SDL_SCANCODE_N] = "n",
  [SDL_SCANCODE_O] = "o",
  [SDL_SCANCODE_P] = "p",
  [SDL_SCANCODE_Q] = "q",
  [SDL_SCANCODE_R] = "r",
  [SDL_SCANCODE_S] = "s",
  [SDL_SCANCODE_T] = "t",
  [SDL_SCANCODE_U] = "u",
  [SDL_SCANCODE_V] = "v",
  [SDL_SCANCODE_W] = "w",
  [SDL_SCANCODE_X] = "x",
  [SDL_SCANCODE_Y] = "y",
  [SDL_SCANCODE_Z] = "z",

  [SDL_SCANCODE_F1] = "f1",
  [SDL_SCANCODE_F2] = "f2",
  [SDL_SCANCODE_F3] = "f3",
  [SDL_SCANCODE_F4] = "f4",
  [SDL_SCANCODE_F5] = "f5",
  [SDL_SCANCODE_F6] = "f6",
  [SDL_SCANCODE_F7] = "f7",
  [SDL_SCANCODE_F8] = "f8",
  [SDL_SCANCODE_F9] = "f9",
  [SDL_SCANCODE_F10] = "f10",
  [SDL_SCANCODE_F11] = "f11",
  [SDL_SCANCODE_F12] = "f12",
  [SDL_SCANCODE_F13] = "f13",
  [SDL_SCANCODE_F14] = "f14",
  [SDL_SCANCODE_F15] = "f15",

  [SDL_SCANCODE_UP] = "up",
  [SDL_SCANCODE_DOWN] = "down",
  [SDL_SCANCODE_LEFT] = "left",
  [SDL_SCANCODE_RIGHT] = "right",

  [SDL_SCANCODE_INSERT] = "insert",
  [SDL_SCANCODE_HOME] = "home",
  [SDL_SCANCODE_END] = "end",
  [SDL_SCANCODE_PAGEUP] = "page up",
  [SDL_SCANCODE_PAGEDOWN] = "page down",
};
hashtable_t *gIoReverseKeySymMap;

void
ioQuitHandler(bool buttonDown, void *data)
{
  SDL_Event quitEvent;
  quitEvent.type = SDL_QUIT;
  if (SDL_PushEvent(&quitEvent) != 0) {
    ooLogFatal("Clean shutdown prevented by push event failure");
  } else {
    ooLogTrace("Pushed SDL_QUIT event");
  }
}

void
ioGetMousePos(float *x, float *y)
{
  assert(x != NULL);
  assert(y != NULL);
  int xp, yp;

  SDL_GetMouseState(&xp, &yp);
  SDL_Surface *videoSurface = SDL_GetVideoSurface();

  *x = ((float)xp / (float)videoSurface->w) * 2.0f - 1.0f;
  *y = ((float)yp / (float)videoSurface->h) * 2.0f - 1.0f;
}

void ioInitJoysticks(void);

/*
    Used to filter away joystick axis events
 */
static int ioFilter(void *data, SDL_Event *ev)
{
  switch (ev->type) {
  case SDL_JOYAXISMOTION:
  case SDL_JOYBALLMOTION:
    return 0;
  default:
    // Accept all other events
    return 1;
  }
}
void ioInitKeys(void);

INIT_PRIMARY_MODULE
{
  gIoReverseKeySymMap = hashtable_new_with_str_keys(1024);
  gIoButtonHandlers = hashtable_new_with_str_keys(2048);
  for (size_t i = 0 ; i < SDL_NUM_SCANCODES ; ++ i) {
    if (keysymmap[i] != NULL) {
      hashtable_insert(gIoReverseKeySymMap, keysymmap[i], (void*)i);
    }
  }

  for (size_t i = 0 ; i < SDL_NUM_SCANCODES ; ++ i) {
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
  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].isScript = false;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].cHandler = ioQuitHandler;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].data = NULL;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].isScript = false;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].cHandler = ioQuitHandler;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].data = NULL;

}

void
ioInit(void)
{
  assert(SDL_WasInit(SDL_INIT_JOYSTICK) == SDL_INIT_JOYSTICK);

  SDL_SetEventFilter(ioFilter, NULL);

  // Now initialize joysticks
  ioInitJoysticks();

  // Init key data
  ioInitKeys();


  // Hook up the quit handler as default to cmd / meta q, this overrides the initkeys conf
  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].isScript = false;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].cHandler = ioQuitHandler;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_L_Cmd].data = NULL;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].isScript = false;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].cHandler = ioQuitHandler;
  gIoKeyData[SDL_SCANCODE_Q].down[OO_R_Cmd].data = NULL;
}

void
ioInitKeys(void)
{
  HRMLobject *confObj = ooConfGetNode("openorbit/controls");
  for (HRMLobject *keyboard = confObj->children; keyboard != NULL ; keyboard = keyboard->next) {
    if (!strcmp(keyboard->name, "keyboard")) {

      for (HRMLobject *keyInfo = keyboard->children; keyInfo != NULL;
           keyInfo = keyInfo->next)
      {
        if (!strcmp(keyInfo->name, "key")) {
          const char *actionName = hrmlGetStr(keyInfo);
          HRMLvalue keyId = hrmlGetAttrForName(keyInfo, "id");
          HRMLvalue keyMod = hrmlGetAttrForName(keyInfo, "mod");

          uint32_t flags = 0;

          if (keyMod.typ == HRMLStr) {
            if (!strcmp(keyMod.u.str, "lmeta")) {
              flags = OO_IO_MOD_LMETA;
            }
          }

          assert(keyId.typ == HRMLStr);
          ooIoBindKeyHandler(keyId.u.str, actionName, 0, flags);
        } else if (!strcmp(keyInfo->name, "key-axis")) {
          const char *axisName = hrmlGetStr(keyInfo);
          HRMLvalue keyId = hrmlGetAttrForName(keyInfo, "id");
          assert(keyId.typ == HRMLStr);
          HRMLvalue buttonVal = hrmlGetAttrForName(keyInfo, "val");
          assert(buttonVal.typ == HRMLFloat);
          ioBindKeyToAxis(axisName, keyId.u.str, buttonVal.u.real);
        }
      }
    }
  }
}

void
ioDispatchKeyUp(int key, uint16_t mask)
{
  assert(key < SDL_NUM_SCANCODES);

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
ioDispatchKeyDown(int key, uint16_t mask)
{
  assert(key < SDL_NUM_SCANCODES);

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

void
ioRegActionHandler(const char *name, OObuttonhandlerfunc handlerFunc, void *data)
{
  OObuttonhandler *handler =
    (OObuttonhandler*)hashtable_lookup(gIoButtonHandlers, name);
  if (handler != NULL) {
    ooLogWarn("%s already registered as button handler", name);
    return;
  }

  handler = malloc(sizeof(OObuttonhandler));
  handler->isScript = false;
  handler->cHandler = handlerFunc;
  handler->data = data;

  hashtable_insert(gIoButtonHandlers, name, handler);
}


void
ooIoRegCKeyHandler(const char *name, OObuttonhandlerfunc handlerFunc)
{
  OObuttonhandler *handler
       = (OObuttonhandler*)hashtable_lookup(gIoButtonHandlers, name);
  if (handler != NULL) {
      ooLogWarn("%s already registered as button handler", name);
      return;
  }

  handler = malloc(sizeof(OObuttonhandler));
  handler->isScript = false;
  handler->cHandler = handlerFunc;

  hashtable_insert(gIoButtonHandlers, name, handler);
}

void
ooIoRegPyKeyHandler(const char *name, PyObject *handlerFunc)
{
  OObuttonhandler *handler
       = (OObuttonhandler*)hashtable_lookup(gIoButtonHandlers, name);
  if (handler != NULL) {
    ooLogWarn("%s already registered as button handler", name);
    return;
  }

  handler = malloc(sizeof(OObuttonhandler));
  handler->isScript = true;
  handler->pyHandler = handlerFunc;

  hashtable_insert(gIoButtonHandlers, name, handler);
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

  OObuttonhandler *keyHandler = hashtable_lookup(gIoButtonHandlers, keyAction);

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

typedef struct IOaxisemudata {
  const char *axisKey;
  float val;
} IOaxisemudata;

void
ooAxisEmulation(bool buttonDown, void *data)
{
  assert(data != NULL);

  IOaxisemudata *ad = data;
  OOaxishandler *handler = hashtable_lookup(gIoAxisHandlers, ad->axisKey);

  if (handler) {
    if (buttonDown) {
      handler->buttonDown = true;
      handler->val = ad->val;
    } else {
      handler->buttonDown = false;
      handler->val = 0.0;
    }
  }
}


void
ioInitJoysticks(void)
{
  gIoAxisHandlers = hashtable_new_with_str_keys(128);
  HRMLobject *confObj = ooConfGetNode("openorbit/controls");
  assert(confObj != NULL);

  gJoyButtons.joyCount = SDL_NumJoysticks();
  gJoyButtons.joy = calloc(gJoyButtons.joyCount, sizeof(IOjbuttons*));

  for (int i = 0 ; i < gJoyButtons.joyCount; ++ i) {
    gJoyButtons.joy[i] = malloc(sizeof(IOjbuttons));
    SDL_Joystick *stick = SDL_JoystickOpen(i);
    gJoyButtons.joy[i]->buttonCount = SDL_JoystickNumButtons(stick);
    gJoyButtons.joy[i]->down = calloc(gJoyButtons.joy[i]->buttonCount, sizeof(OObuttonhandler));
    gJoyButtons.joy[i]->up = calloc(gJoyButtons.joy[i]->buttonCount, sizeof(OObuttonhandler));
  }

  for (int i = 0 ; i < gJoyButtons.joyCount; ++ i) {
    for (int j = 0 ; j < gJoyButtons.joy[i]->buttonCount ; ++ j) {
      gJoyButtons.joy[i]->down[j].isScript = false;
      gJoyButtons.joy[i]->down[j].cHandler = ooButtonHandlerGnd;
      gJoyButtons.joy[i]->down[j].data = NULL;
      gJoyButtons.joy[i]->up[j].isScript = false;
      gJoyButtons.joy[i]->up[j].cHandler = ooButtonHandlerGnd;
      gJoyButtons.joy[i]->up[j].data = NULL;
    }
  }

  for (HRMLobject *jstick = confObj->children; jstick != NULL ; jstick = jstick->next) {
    if (!strcmp(jstick->name, "joystick")) {
      HRMLvalue name = hrmlGetAttrForName(jstick, "name");
      HRMLvalue id = hrmlGetAttrForName(jstick, "id");

      int idVal = 0;
      if (name.typ != HRMLStr) {
        fprintf(stderr, "joystick name is not a string\n");
        return;
      }
      const char *nameStr = name.u.str;

      if (id.typ == HRMLInt) {
        idVal = id.u.integer;
      }

      int joystickId = ooIoGetJoystickId(nameStr, idVal);
      if (joystickId == -1) continue;
      for (HRMLobject *joystickSensor = jstick->children; joystickSensor != NULL;
           joystickSensor = joystickSensor->next)
      {
        if (!strcmp(joystickSensor->name, "axis")) {
          const char *axisName = hrmlGetStr(joystickSensor);
          HRMLvalue axisId = hrmlGetAttrForName(joystickSensor, "id");
          assert(axisId.typ == HRMLInt);
          ooIoBindAxis(axisName, joystickId, axisId.u.integer);
        } else if (!strcmp(joystickSensor->name, "button-axis")) {
          const char *axisName = hrmlGetStr(joystickSensor);
          HRMLvalue buttonId = hrmlGetAttrForName(joystickSensor, "id");
          assert(buttonId.typ == HRMLInt);

          HRMLvalue buttonDir = hrmlGetAttrForName(joystickSensor, "val");
          assert(buttonDir.typ == HRMLFloat);
          ioBindButtonToAxis(axisName, joystickId, buttonId.u.integer,
                             buttonDir.u.real);
        } else if (!strcmp(joystickSensor->name, "button")) {
          const char *actionName = hrmlGetStr(joystickSensor);
          HRMLvalue buttonId = hrmlGetAttrForName(joystickSensor, "id");
          assert(buttonId.typ == HRMLInt);
          ioBindJoystickButton(actionName, joystickId, buttonId.u.integer);
        }
      }
    }
  }
}

void
ooIoPrintJoystickNames(void)
{
  printf("===== Joystick =====\n");

  int joyCount = SDL_NumJoysticks();
  for (int i = 0 ; i < joyCount ; ++ i) {
    const char *joyName = SDL_JoystickName(i);
    printf("Joystick %d: '%s' available\n", i, joyName);
  }
  printf("====================\n");
}

int
ooIoGetJoystickId(const char *name, int subId)
{
  assert(name != NULL);
  assert(0 <= subId);

  int joyCount = SDL_NumJoysticks();
  for (int i = 0 ; i < joyCount ; ++ i) {
    const char *joyName = SDL_JoystickName(i);
    if (!strcmp(joyName, name)) {
      if (subId == 0) {
        return i;
      } else {
        subId --;
      }
    }
  }
  return -1;
}

void
ioBindJoystickButton(const char *key, int joyStick, int button)
{
  if (key == NULL) {
    return;
  }

  OObuttonhandler *keyHandler = hashtable_lookup(gIoButtonHandlers, key);
  if (keyHandler == NULL) {
    return;
  }

  if (joyStick >= gJoyButtons.joyCount || joyStick < 0) {
    ooLogWarn("attempted to bind '%s' to invalid joystick id '%d'", key, joyStick);
    return;
  }

  if (button >= gJoyButtons.joy[joyStick]->buttonCount || button < 0) {
    ooLogWarn("attempted to bind '%s' to invalid joystick '%d' button '%d'",
              key, joyStick, button);
    return;
  }

  // TODO: We should unmap the old entry properly, not just replace it
  gJoyButtons.joy[joyStick]->down[button] = *keyHandler;
  gJoyButtons.joy[joyStick]->down[button].data = NULL;
  //gJoyButtons.joy[joyStick]->up[button] = *keyHandler;

}

void
ioBindButtonToAxis(const char *axisKey, int joystick, int button, float val)
{
  assert(axisKey != NULL);
  assert(joystick >= 0);
  assert(button >= 0);

  OOaxishandler *handler = hashtable_lookup(gIoAxisHandlers, axisKey);

  if (!handler) {
    handler = malloc(sizeof(OOaxishandler));
    handler->joyId = NULL;

    handler->axisId = -1;
    handler->nullZone = 0.1;
    handler->trim = 0.0;
    handler->buttonDown = false;
    handler->val = 0.0;
    hashtable_insert(gIoAxisHandlers, axisKey, handler);
  }

  gJoyButtons.joy[joystick]->down[button].isScript = false;
  gJoyButtons.joy[joystick]->down[button].cHandler = ooAxisEmulation;
  gJoyButtons.joy[joystick]->up[button].isScript = false;
  gJoyButtons.joy[joystick]->up[button].cHandler = ooAxisEmulation;

  IOaxisemudata *ad = malloc(sizeof(IOaxisemudata));
  ad->axisKey = strdup(axisKey);
  ad->val = val;

  gJoyButtons.joy[joystick]->down[button].data = ad;
  gJoyButtons.joy[joystick]->up[button].data = ad;
}



void
ooIoBindAxis(const char *key, int joyStick, int axis)
{
  OOaxishandler *handler
      = hashtable_lookup(gIoAxisHandlers, key);
  if (handler) {
    handler->joyId = SDL_JoystickOpen(joyStick);
    assert(handler->joyId);
    handler->axisId = axis;
    handler->nullZone = 0.1;
    handler->trim = 0.0;
    handler->buttonDown = false;
    handler->val = 0.0;
  } else {
    OOaxishandler *axisHandler = malloc(sizeof(OOaxishandler));
    axisHandler->joyId = SDL_JoystickOpen(joyStick);
    assert(axisHandler->joyId);
    axisHandler->axisId = axis;
    axisHandler->nullZone = 0.1;
    axisHandler->trim = 0.0;
    axisHandler->buttonDown = false;
    axisHandler->val = 0.0;
    hashtable_insert(gIoAxisHandlers, key, axisHandler);
  }
}

void
ioBindKeyToAxis(const char *key, const char *button, float val)
{
  uintptr_t key_id = (uintptr_t) hashtable_lookup(gIoReverseKeySymMap, button);
  OOaxishandler *handler = hashtable_lookup(gIoAxisHandlers, key);
  if (handler) {
    gIoKeyData[key_id].up[OO_None].isScript = false;
    gIoKeyData[key_id].up[OO_None].cHandler = ooAxisEmulation;
    gIoKeyData[key_id].down[OO_None].isScript = false;
    gIoKeyData[key_id].down[OO_None].cHandler = ooAxisEmulation;

    // TODO: Address memory leak when replacing handlers
    IOaxisemudata *ad = malloc(sizeof(IOaxisemudata));
    ad->axisKey = strdup(key);
    ad->val = val;

    gIoKeyData[key_id].up[OO_None].data = ad;
    gIoKeyData[key_id].down[OO_None].data = ad;

  } else {
    handler = malloc(sizeof(OOaxishandler));
    handler->joyId = NULL;

    handler->axisId = -1;
    handler->nullZone = 0.1;
    handler->trim = 0.0;
    handler->buttonDown = false;
    handler->val = 0.0;



    gIoKeyData[key_id].up[OO_None].isScript = false;
    gIoKeyData[key_id].up[OO_None].cHandler = ooAxisEmulation;
    gIoKeyData[key_id].down[OO_None].isScript = false;
    gIoKeyData[key_id].down[OO_None].cHandler = ooAxisEmulation;

    IOaxisemudata *ad = malloc(sizeof(IOaxisemudata));
    ad->axisKey = strdup(key);
    ad->val = val;

    gIoKeyData[key_id].up[OO_None].data = ad;
    gIoKeyData[key_id].down[OO_None].data = ad;

    hashtable_insert(gIoAxisHandlers, key, handler);
  }
}

void
ooIoAdjustTrim(const char *key, float dtrim)
{
  OOaxishandler *handler
      = hashtable_lookup(gIoAxisHandlers, key);
  if (handler) {
    handler->trim += dtrim;
  }
}

void
ooIoSetNullZone(const char *key, float nz)
{
  OOaxishandler *handler
      = hashtable_lookup(gIoAxisHandlers, key);
  if (handler) {
    handler->nullZone = nz;
  }
}



float
ooIoGetAxis(float *val, const char *axis)
{
  OOaxishandler *handler
      = hashtable_lookup(gIoAxisHandlers, axis);

  if (!handler) {
    ooLogTrace("axis '%s' is not known", axis);
    if (val) *val = 0.0f;
    return 0.0f;
  }

  if (handler->buttonDown) {
    // Axis is emulated with a button press
    if (handler->val > 2.0) {
      ooLogError("handler value to high %f", handler->val);
    }
    if (val) *val = handler->val;
    return (float)handler->val;
  } else if (handler->joyId == NULL) {
    // No actual joystick bound to the handler, axis is handled by button
    // presses
    if (val) *val = 0.0f;
    return 0.0f;
  }

  int16_t axisVal = SDL_JoystickGetAxis(handler->joyId, handler->axisId);
  float normalisedAxis;
  if (axisVal >= 0) {
    normalisedAxis = axisVal / 32767.0f;
  } else {
    normalisedAxis = axisVal / 32768.0f;
  }

  if (fabs(normalisedAxis + handler->trim) > handler->nullZone + handler->trim) {
    if (val) *val = normalisedAxis + handler->trim;
    return normalisedAxis + handler->trim;
  }

  if (val) *val = 0.0;
  return 0.0f;
}
