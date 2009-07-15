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

#include <stdbool.h>
#include <err.h>
#include <gencds/hashtable.h>
#include "sim.h"
#include "io-manager.h"
#include "SDL.h"



static const char * gIoSdlKeyStringMap[SDLK_LAST];
static const char * gIoSdlMouseStringMap[7];
static hashtable_t *gIoBoundKeyTable;  // of type OOkeyevent
static hashtable_t *gIoButtonHandlers; // of type OObuttonhandler


void
ooButtonHandlerGnd(bool buttonUp, void *data)
{
    /*Nothing*/
}

typedef struct {
    bool isScript;
    union {
        OObuttonhandlerfunc cHandler;
        PyObject *pyHandler;
    };
} OObuttonhandler;

typedef struct _OOkeyevent {
    struct {
        bool upIsScript;
        union {
            OObuttonhandlerfunc cUp;
            PyObject *pyUp;
        };
    };
    
    struct {
        bool downIsScript;
        union {
            OObuttonhandlerfunc cDown;
            PyObject *pyDown;
        };
    };
    
    uint16_t modMask;
    void *data;
    struct _OOkeyevent *next;
} OOkeyevent;

void
ooIoRegCKeyHandler(const char *name, OObuttonhandlerfunc handlerFunc)
{
    OObuttonhandler *handler
        = (OObuttonhandler*)hashtable_lookup(gIoButtonHandlers, name);
    if (handler != NULL) {
        warnx("%s already registered as button handler", name);
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
        warnx("%s already registered as button handler", name);
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
    OOkeyevent *ev = hashtable_lookup(gIoBoundKeyTable, keyName);
    OOkeyevent *firstEv = ev;

    OObuttonhandler *keyHandler = hashtable_lookup(gIoButtonHandlers, keyAction);
    if (keyHandler == NULL) {
        warnx("%s not found in button handler dictionary", keyAction);
        return;
    }
    
    while (ev && (ev->modMask != mask)) {
        ev = ev->next;
    }
    
    if (ev != NULL) {
        // does the event handler already exist, if so just replace the mapping
        if (up) {
            if (ev->upIsScript) Py_XDECREF(ev->pyUp);
            ev->upIsScript = keyHandler->isScript;
            if (keyHandler->isScript) ev->pyUp = keyHandler->pyHandler;
            else ev->cUp = keyHandler->cHandler;
        } else {
            if (ev->downIsScript) Py_XDECREF(ev->pyDown);
            ev->downIsScript = keyHandler->isScript;
            if (keyHandler->isScript) ev->pyDown = keyHandler->pyHandler;
            else ev->cDown = keyHandler->cHandler;
        }
    } else {
        ev = malloc(sizeof(OOkeyevent));
        
        if (up) {
            ev->upIsScript = keyHandler->isScript;
            if (keyHandler->isScript) ev->pyUp = keyHandler->pyHandler;
            else ev->cUp = keyHandler->cHandler;
            
            ev->modMask = mask;
            ev->data = NULL;
            ev->downIsScript = false;
            ev->cDown = ooButtonHandlerGnd;
            ev->next = NULL;
        } else {
            ev->downIsScript = keyHandler->isScript;
            if (keyHandler->isScript) ev->pyDown = keyHandler->pyHandler;
            else ev->cDown = keyHandler->cHandler;
            ev->modMask = mask;
            ev->data = NULL;
            ev->upIsScript = false;
            ev->cUp = ooButtonHandlerGnd;
            ev->next = NULL;
        }
        
        /* Insert in hashtable */
        if (firstEv == NULL) {
            hashtable_insert(gIoBoundKeyTable, keyName, ev);
        } else {
            // After first event since this is the simplest
            ev->next = firstEv->next;
            firstEv->next = ev;
        }
    }
}


void
ooIoDispatchKeyUp(const char *name, uint16_t mask)
{
    OOkeyevent *ev = (OOkeyevent*)hashtable_lookup(gIoBoundKeyTable, name);

    while (ev && (ev->modMask & mask) != mask) {
        ev = ev->next;
    }
    
    if (ev != NULL) {
        if (ev->upIsScript) {
            //PyObject *res = PyObject_CallFunction(f, "(dd)", (double)dx, (double)dy);
            //if (! res) PyErr_Print();
            //Py_XDECREF(res);
            ;
        } else {
            ev->cUp(true, ev->data);
        }
    }
}


void
ooIoDispatchKeyDown(const char *name, uint16_t mask)
{
    OOkeyevent *ev = (OOkeyevent*)hashtable_lookup(gIoBoundKeyTable, name);

    while (ev && (ev->modMask & mask) != mask) {
        ev = ev->next;
    }
    
    if (ev != NULL) {
        if (ev->downIsScript) {
            ;
        } else {
            ev->cDown(false, ev->data);
        }
    }
}

const char *
ooIoSdlKeyNameLookup(SDLKey keyId)
{
    if (keyId >= SDLK_LAST) return NULL;
    return gIoSdlKeyStringMap[keyId];
}

const char *
ooIoSdlMouseButtonNameLookup(unsigned buttonId)
{
    if (buttonId > 7) return NULL;
    return gIoSdlMouseStringMap[buttonId-1];
}


void ooIoInitSdlStringMap(void)
{
    memset(gIoSdlKeyStringMap, 0, SDLK_LAST * sizeof(const char*));
    
    gIoBoundKeyTable = hashtable_new_with_str_keys(128);
    gIoButtonHandlers = hashtable_new_with_str_keys(256);

    gIoSdlMouseStringMap[SDL_BUTTON_LEFT-1]	= "mouse-left";
    gIoSdlMouseStringMap[SDL_BUTTON_MIDDLE-1] =	"mouse-middle";
    gIoSdlMouseStringMap[SDL_BUTTON_RIGHT-1] = "mouse-right";
    gIoSdlMouseStringMap[SDL_BUTTON_WHEELUP-1] = "mouse-up";
    gIoSdlMouseStringMap[SDL_BUTTON_WHEELDOWN-1] = "mouse-down";
    gIoSdlMouseStringMap[SDL_BUTTON_X1-1] = "mouse-x1";
    gIoSdlMouseStringMap[SDL_BUTTON_X2-1] = "mouse-x2";
    
    
    gIoSdlKeyStringMap[SDLK_LSHIFT] = "lshift";
    gIoSdlKeyStringMap[SDLK_RSHIFT] = "rshift";
    gIoSdlKeyStringMap[SDLK_LMETA] = "lmeta";
    gIoSdlKeyStringMap[SDLK_RMETA] = "rmeta";
    gIoSdlKeyStringMap[SDLK_LCTRL] = "lctrl";
    gIoSdlKeyStringMap[SDLK_RCTRL] = "rctrl";
    gIoSdlKeyStringMap[SDLK_LSUPER] = "lsuper";
    gIoSdlKeyStringMap[SDLK_RSUPER] = "rsuper";
    
    gIoSdlKeyStringMap[SDLK_MODE] = "mode";
    gIoSdlKeyStringMap[SDLK_HELP] = "help";
    gIoSdlKeyStringMap[SDLK_PRINT] = "print-screen";
    gIoSdlKeyStringMap[SDLK_SYSREQ] = "sys-req";
    gIoSdlKeyStringMap[SDLK_BREAK] = "break";
    gIoSdlKeyStringMap[SDLK_MENU] = "menu";
    gIoSdlKeyStringMap[SDLK_POWER] = "power";
    gIoSdlKeyStringMap[SDLK_EURO] = "euro";

    gIoSdlKeyStringMap[SDLK_RETURN] = "return";
    gIoSdlKeyStringMap[SDLK_SPACE] = "space";
    gIoSdlKeyStringMap[SDLK_TAB] = "tab";
    gIoSdlKeyStringMap[SDLK_BACKSPACE] = "backspace";
    gIoSdlKeyStringMap[SDLK_ESCAPE] = "esc";
    gIoSdlKeyStringMap[SDLK_PERIOD] = ".";
    gIoSdlKeyStringMap[SDLK_COMMA] = ",";
    gIoSdlKeyStringMap[SDLK_BACKQUOTE] = "`";
    gIoSdlKeyStringMap[SDLK_CLEAR] = "clear";
    gIoSdlKeyStringMap[SDLK_PAUSE] = "pause";
    gIoSdlKeyStringMap[SDLK_EXCLAIM] = "!";
    gIoSdlKeyStringMap[SDLK_QUOTEDBL] = "\"";
    gIoSdlKeyStringMap[SDLK_HASH] = "#";
    gIoSdlKeyStringMap[SDLK_DOLLAR] = "$";
    gIoSdlKeyStringMap[SDLK_AMPERSAND] = "&";
    gIoSdlKeyStringMap[SDLK_QUOTE] = "'";
    gIoSdlKeyStringMap[SDLK_LEFTPAREN] = "(";
    gIoSdlKeyStringMap[SDLK_RIGHTPAREN] = ")";
    gIoSdlKeyStringMap[SDLK_ASTERISK] = "*";
    gIoSdlKeyStringMap[SDLK_PLUS] = "+";
    gIoSdlKeyStringMap[SDLK_MINUS] = "-";
    gIoSdlKeyStringMap[SDLK_SLASH] = "/";

    gIoSdlKeyStringMap[SDLK_COLON] = ":";
    gIoSdlKeyStringMap[SDLK_SEMICOLON] = ";";
    gIoSdlKeyStringMap[SDLK_LESS] = "<";
    gIoSdlKeyStringMap[SDLK_EQUALS] = "=";
    gIoSdlKeyStringMap[SDLK_GREATER] = ">";
    gIoSdlKeyStringMap[SDLK_QUESTION] = "?";
    gIoSdlKeyStringMap[SDLK_AT] = "@";
    gIoSdlKeyStringMap[SDLK_LEFTBRACKET] = "[";
    gIoSdlKeyStringMap[SDLK_BACKSLASH] = "\\";
    gIoSdlKeyStringMap[SDLK_RIGHTBRACKET] = "]";
    gIoSdlKeyStringMap[SDLK_CARET] = "^";
    gIoSdlKeyStringMap[SDLK_UNDERSCORE] = "_";

    gIoSdlKeyStringMap[SDLK_0] = "0";
    gIoSdlKeyStringMap[SDLK_1] = "1";
    gIoSdlKeyStringMap[SDLK_2] = "2";
    gIoSdlKeyStringMap[SDLK_3] = "3";
    gIoSdlKeyStringMap[SDLK_4] = "4";
    gIoSdlKeyStringMap[SDLK_5] = "5";
    gIoSdlKeyStringMap[SDLK_6] = "6";
    gIoSdlKeyStringMap[SDLK_7] = "7";
    gIoSdlKeyStringMap[SDLK_8] = "8";
    gIoSdlKeyStringMap[SDLK_9] = "9";
    
    gIoSdlKeyStringMap[SDLK_KP0] = "kp 0";
    gIoSdlKeyStringMap[SDLK_KP1] = "kp 1";
    gIoSdlKeyStringMap[SDLK_KP2] = "kp 2";
    gIoSdlKeyStringMap[SDLK_KP3] = "kp 3";
    gIoSdlKeyStringMap[SDLK_KP4] = "kp 4";
    gIoSdlKeyStringMap[SDLK_KP5] = "kp 5";
    gIoSdlKeyStringMap[SDLK_KP6] = "kp 6";
    gIoSdlKeyStringMap[SDLK_KP7] = "kp 7";
    gIoSdlKeyStringMap[SDLK_KP8] = "kp 8";
    gIoSdlKeyStringMap[SDLK_KP9] = "kp 9";    

    gIoSdlKeyStringMap[SDLK_KP_PERIOD] = "kp .";
    gIoSdlKeyStringMap[SDLK_KP_DIVIDE] = "kp /";
    gIoSdlKeyStringMap[SDLK_KP_MULTIPLY] = "kp *";
    gIoSdlKeyStringMap[SDLK_KP_MINUS] = "kp -";
    gIoSdlKeyStringMap[SDLK_KP_PLUS] = "kp +";
    gIoSdlKeyStringMap[SDLK_KP_ENTER] = "enter";
    gIoSdlKeyStringMap[SDLK_KP_EQUALS] = "kp =";
    gIoSdlKeyStringMap[SDLK_DELETE] = "delete";

    gIoSdlKeyStringMap[SDLK_a] = "a";
    gIoSdlKeyStringMap[SDLK_b] = "b";
    gIoSdlKeyStringMap[SDLK_c] = "c";
    gIoSdlKeyStringMap[SDLK_d] = "d";
    gIoSdlKeyStringMap[SDLK_e] = "e";
    gIoSdlKeyStringMap[SDLK_f] = "f";
    gIoSdlKeyStringMap[SDLK_g] = "g";
    gIoSdlKeyStringMap[SDLK_h] = "h";
    gIoSdlKeyStringMap[SDLK_i] = "i";
    gIoSdlKeyStringMap[SDLK_j] = "j";
    gIoSdlKeyStringMap[SDLK_k] = "k";
    gIoSdlKeyStringMap[SDLK_l] = "l";
    gIoSdlKeyStringMap[SDLK_m] = "m";
    gIoSdlKeyStringMap[SDLK_n] = "n";
    gIoSdlKeyStringMap[SDLK_o] = "o";
    gIoSdlKeyStringMap[SDLK_p] = "p";
    gIoSdlKeyStringMap[SDLK_q] = "q";
    gIoSdlKeyStringMap[SDLK_r] = "r";
    gIoSdlKeyStringMap[SDLK_s] = "s";
    gIoSdlKeyStringMap[SDLK_t] = "t";
    gIoSdlKeyStringMap[SDLK_u] = "u";
    gIoSdlKeyStringMap[SDLK_v] = "v";
    gIoSdlKeyStringMap[SDLK_w] = "w";
    gIoSdlKeyStringMap[SDLK_x] = "x";
    gIoSdlKeyStringMap[SDLK_y] = "y";
    gIoSdlKeyStringMap[SDLK_z] = "z";

    gIoSdlKeyStringMap[SDLK_F1] = "f1";
    gIoSdlKeyStringMap[SDLK_F2] = "f2";
    gIoSdlKeyStringMap[SDLK_F3] = "f3";
    gIoSdlKeyStringMap[SDLK_F4] = "f4";
    gIoSdlKeyStringMap[SDLK_F5] = "f5";
    gIoSdlKeyStringMap[SDLK_F6] = "f6";
    gIoSdlKeyStringMap[SDLK_F7] = "f7";
    gIoSdlKeyStringMap[SDLK_F8] = "f8";
    gIoSdlKeyStringMap[SDLK_F9] = "f9";
    gIoSdlKeyStringMap[SDLK_F10] = "f10";
    gIoSdlKeyStringMap[SDLK_F11] = "f11";
    gIoSdlKeyStringMap[SDLK_F12] = "f12";
    gIoSdlKeyStringMap[SDLK_F13] = "f13";
    gIoSdlKeyStringMap[SDLK_F14] = "f14";
    gIoSdlKeyStringMap[SDLK_F15] = "f15";

    gIoSdlKeyStringMap[SDLK_UP] = "up";
    gIoSdlKeyStringMap[SDLK_DOWN] = "down";
    gIoSdlKeyStringMap[SDLK_LEFT] = "left";
    gIoSdlKeyStringMap[SDLK_RIGHT] = "right";

    gIoSdlKeyStringMap[SDLK_INSERT] = "insert";
    gIoSdlKeyStringMap[SDLK_HOME] = "home";
    gIoSdlKeyStringMap[SDLK_END] = "end";
    gIoSdlKeyStringMap[SDLK_PAGEUP] = "page up";
    gIoSdlKeyStringMap[SDLK_PAGEDOWN] = "page down";
}


