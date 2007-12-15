/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
 */

#include <stdbool.h>

#include "io-manager.h"
#include "SDL.h"
#include "libgencds/hashtable.h"

// Default grounding functions
static void io_init_actiontable(void);
static void io_init_keytable(void);


static io_manager_t IO_manager;

static hashtable_t *IO_action_table;
static hashtable_t *IO_event_handler_table;
static hashtable_t *IO_key_table;

static io_bindings_t IO_bindings;

static void
io_event_def(sim_event_t eid, void *data)
{
    assert(eid == SIM_io_event);
    io_event_t *e = (io_event_t*)data;
}

void
io_bindings_init(void)
{
    for (int i = 0 ; i < IO_MAX_KEYS ; i ++) {
        for (int j = 0 ; j < IO_MAX_MOD_COMBOS ; j ++) {
            IO_bindings.keyboard.key[i][j].modifier_mask = false;
            IO_bindings.keyboard.key[i][j].up.is_script = false;
            IO_bindings.keyboard.key[i][j].up.u.c = io_event_def;
            IO_bindings.keyboard.key[i][j].down.is_script = false;
            IO_bindings.keyboard.key[i][j].down.u.c = io_event_def;
        }
    }
    for (int i = 0 ; i < IO_MAX_MOUSE_BUTTONS ; i ++) {
        IO_bindings.mouse.button[i].up.is_script = false;
        IO_bindings.mouse.button[i].up.u.c = io_event_def;
        IO_bindings.mouse.button[i].down.is_script = false;
        IO_bindings.mouse.button[i].down.u.c = io_event_def;
        
        IO_bindings.mouse.drag[i].is_script = false;
        IO_bindings.mouse.drag[i].u.c = io_event_def;
    }
    
    for (int i = 0 ; i < IO_MAX_JOYSTICKS ; i ++) {
		for (int j = 0 ; j < IO_MAX_JOYSTICK_BUTTONS ; j ++) { 
			IO_bindings.joystick[i].button[j].up.is_script = false;
            IO_bindings.joystick[i].button[j].up.u.c = io_event_def;
            
            IO_bindings.joystick[i].button[j].down.is_script = false;
            IO_bindings.joystick[i].button[j].down.u.c = io_event_def;
        }
		for (int j = 0 ; j < IO_MAX_JOYSTICK_AXIS ; j ++) { 
            IO_bindings.joystick[i].axis[j].is_script = false;
            IO_bindings.joystick[i].axis[j].u.c = io_event_def;
        }
	}
}

bool
io_register_event_handler(const char *key, sim_event_handler_t f,
                          io_event_kind_t kind)
{
    io_event_handler_container_t *cont;
    
    cont = malloc(sizeof(io_event_handler_container_t));
    if (! cont) return false;
    
    cont->kind = kind;
    cont->handler.is_script = false;
    cont->handler.u.c = f;
    
    hashtable_insert(IO_event_handler_table, key, cont);
    return true;
}

int
io_register_event_handler_script(const char *key, scr_func_t f,
                                 io_event_kind_t kind)
{
    io_event_handler_container_t *cont;
    
    cont = malloc(sizeof(io_event_handler_container_t));
    if (! cont) return false;

    Py_INCREF(f);
    
    cont->kind = kind;
    cont->handler.is_script = true;
    cont->handler.u.s = f;
    
    hashtable_insert(IO_event_handler_table, key, cont);
    return true;
}

void
io_dispatch_event(const io_event_t *e)
{
    switch (e->kind) {
    case IO_ev_axis:
        if (IO_bindings.joystick[e->u.axis.joystick_id]
                       .axis[e->u.axis.axis_id].is_script) {
            
        } else {
            IO_bindings.joystick[e->u.axis.joystick_id]
                       .axis[e->u.axis.axis_id].u.c(SIM_io_event, (void*)e);
        }
        break;
    case IO_ev_button:
        if (e->u.button.down) {
            if (IO_bindings.joystick[e->u.button.joystick_id]
                .button[e->u.button.button_id].down.is_script) {
                
            } else {
                IO_bindings.joystick[e->u.button.joystick_id]
                .button[e->u.button.button_id].down.u.c(SIM_io_event, (void*)e);
            }                    
        } else {
            if (IO_bindings.joystick[e->u.button.joystick_id]
                .button[e->u.button.button_id].up.is_script) {
                
            } else {
                IO_bindings.joystick[e->u.button.joystick_id]
                .button[e->u.button.button_id].up.u.c(SIM_io_event, (void*)e);
            }                                
        }
        break;
    case IO_ev_key:
        for (int i = 0 ; i < IO_MAX_MOD_COMBOS ; i ++) {
            if (! (IO_bindings.keyboard.key[e->u.key.key_id][i].modifier_mask
                   ^ e->u.key.modifiers)) {
                if (e->u.key.down) {
                    if (IO_bindings.keyboard.key[e->u.key.key_id][i]
                                            .down.is_script) {
                    
                    } else {
                        IO_bindings.keyboard.key[e->u.key.key_id][i].down.u.c(SIM_io_event, (void*)e);
                    }
                } else {
                    if (IO_bindings.keyboard.key[e->u.key.key_id][i]
                                            .up.is_script) {
                        
                    } else {
                        IO_bindings.keyboard.key[e->u.key.key_id][i].up.u.c(SIM_io_event, (void*)e);
                    }
                }
                break;
            }
        }
        break;
    case IO_ev_click:
        if (e->u.click.down) {
            if (IO_bindings.mouse.button[e->u.click.button].down.is_script) {
                
            } else {
                IO_bindings.mouse.button[e->u.click.button].down.u.c(SIM_io_event, (void*)e);
            }
        } else {
            if (IO_bindings.mouse.button[e->u.click.button].up.is_script) {
                
            } else {
                IO_bindings.mouse.button[e->u.click.button].up.u.c(SIM_io_event, (void*)e);
            }
        }
        break;
    case IO_ev_drag:
        if (IO_bindings.mouse.drag[e->u.drag.button].is_script) {
            
        } else {
            IO_bindings.mouse.drag[e->u.drag.button].u.c(SIM_io_event, (void*)e);
        }
        break;
    case IO_ev_any:
        /* fall through, ambiguous events may not be dispatched */
    default:
        assert(0);
    }
}

static void
io_button_action_def(void)
{
	return;
}

static void
io_click_action_def(float x, float y)
{
	return;
}

static void
io_drag_action_def(float dx, float dy)
{
	return;
}

static void
io_axis_action_def(float value)
{
	return;
}


static void
io_init_actiontable(void)
{
    IO_action_table = hashtable_new_with_str_keys(4096);
}


// when porting to something else than SDL, you have to replace the constants here with the new keycodes
// we store a integers in this hashtable, so we cast this number to a pointer in order to save heap space
static void
io_init_keytable(void)
{
    IO_key_table = hashtable_new_with_str_keys(2048);
    
    hashtable_insert(IO_key_table, "lshift", (void*)SDLK_LSHIFT);
    hashtable_insert(IO_key_table, "rshift", (void*)SDLK_RSHIFT);
    hashtable_insert(IO_key_table, "lmeta",  (void*)SDLK_LMETA);
    hashtable_insert(IO_key_table, "rmeta",  (void*)SDLK_RMETA);
    hashtable_insert(IO_key_table, "lctrl",  (void*)SDLK_LCTRL);
    hashtable_insert(IO_key_table, "rctrl",  (void*)SDLK_RCTRL);
    hashtable_insert(IO_key_table, "lsuper", (void*)SDLK_LSUPER);
    hashtable_insert(IO_key_table, "rsuper", (void*)SDLK_RSUPER);

    hashtable_insert(IO_key_table, "mode",           (void*)SDLK_MODE);
    hashtable_insert(IO_key_table, "help",           (void*)SDLK_HELP);
    hashtable_insert(IO_key_table, "print-screen",   (void*)SDLK_PRINT);
    hashtable_insert(IO_key_table, "sys-req",        (void*)SDLK_SYSREQ);
    hashtable_insert(IO_key_table, "break",          (void*)SDLK_BREAK);
    hashtable_insert(IO_key_table, "menu",           (void*)SDLK_MENU);
    hashtable_insert(IO_key_table, "power",          (void*)SDLK_POWER);
    hashtable_insert(IO_key_table, "euro",           (void*)SDLK_EURO);
        
    hashtable_insert(IO_key_table, "return",     (void*)SDLK_RETURN);
    hashtable_insert(IO_key_table, "space",      (void*)SDLK_SPACE);
    hashtable_insert(IO_key_table, "tab",        (void*)SDLK_TAB);
    hashtable_insert(IO_key_table, "backspace",  (void*)SDLK_BACKSPACE);
    hashtable_insert(IO_key_table, "esc",        (void*)SDLK_ESCAPE);
    hashtable_insert(IO_key_table, ".",          (void*)SDLK_PERIOD);
    hashtable_insert(IO_key_table, ",",          (void*)SDLK_COMMA);
    hashtable_insert(IO_key_table, "`",          (void*)SDLK_BACKQUOTE);
    hashtable_insert(IO_key_table, "clear",      (void*)SDLK_CLEAR);
    hashtable_insert(IO_key_table, "pause",      (void*)SDLK_PAUSE);
    hashtable_insert(IO_key_table, "!",          (void*)SDLK_EXCLAIM);
    hashtable_insert(IO_key_table, "\"",         (void*)SDLK_QUOTEDBL);
    hashtable_insert(IO_key_table, "#",          (void*)SDLK_HASH);
    hashtable_insert(IO_key_table, "$",          (void*)SDLK_DOLLAR);
    hashtable_insert(IO_key_table, "&",          (void*)SDLK_AMPERSAND);
    hashtable_insert(IO_key_table, "'",          (void*)SDLK_QUOTE);
    hashtable_insert(IO_key_table, "(",          (void*)SDLK_LEFTPAREN);
    hashtable_insert(IO_key_table, ")",          (void*)SDLK_RIGHTPAREN);
    hashtable_insert(IO_key_table, "*",          (void*)SDLK_ASTERISK);
    hashtable_insert(IO_key_table, "+",          (void*)SDLK_PLUS);
    hashtable_insert(IO_key_table, "-",          (void*)SDLK_MINUS);
    hashtable_insert(IO_key_table, "/",          (void*)SDLK_SLASH);
            
    hashtable_insert(IO_key_table, ":",  (void*)SDLK_COLON);
    hashtable_insert(IO_key_table, ";",  (void*)SDLK_SEMICOLON);
    hashtable_insert(IO_key_table, "<",  (void*)SDLK_LESS);
    hashtable_insert(IO_key_table, "=",  (void*)SDLK_EQUALS);
    hashtable_insert(IO_key_table, ">",  (void*)SDLK_GREATER);
    hashtable_insert(IO_key_table, "?",  (void*)SDLK_QUESTION);
    hashtable_insert(IO_key_table, "@",  (void*)SDLK_AT);
    hashtable_insert(IO_key_table, "[",  (void*)SDLK_LEFTBRACKET);
    hashtable_insert(IO_key_table, "\\", (void*)SDLK_BACKSLASH);
    hashtable_insert(IO_key_table, "]",  (void*)SDLK_RIGHTBRACKET);
    hashtable_insert(IO_key_table, "^",  (void*)SDLK_CARET);
    hashtable_insert(IO_key_table, "_",  (void*)SDLK_UNDERSCORE);
    
    hashtable_insert(IO_key_table, "0", (void*)SDLK_0);
    hashtable_insert(IO_key_table, "1", (void*)SDLK_1);
    hashtable_insert(IO_key_table, "2", (void*)SDLK_2);
    hashtable_insert(IO_key_table, "3", (void*)SDLK_3);
    hashtable_insert(IO_key_table, "4", (void*)SDLK_4);
    hashtable_insert(IO_key_table, "5", (void*)SDLK_5);
    hashtable_insert(IO_key_table, "6", (void*)SDLK_6);
    hashtable_insert(IO_key_table, "7", (void*)SDLK_7);
    hashtable_insert(IO_key_table, "8", (void*)SDLK_8);
    hashtable_insert(IO_key_table, "9", (void*)SDLK_9);

    hashtable_insert(IO_key_table, "kp 0", (void*)SDLK_KP0);
    hashtable_insert(IO_key_table, "kp 1", (void*)SDLK_KP1);
    hashtable_insert(IO_key_table, "kp 2", (void*)SDLK_KP2);
    hashtable_insert(IO_key_table, "kp 3", (void*)SDLK_KP3);
    hashtable_insert(IO_key_table, "kp 4", (void*)SDLK_KP4);
    hashtable_insert(IO_key_table, "kp 5", (void*)SDLK_KP5);
    hashtable_insert(IO_key_table, "kp 6", (void*)SDLK_KP6);
    hashtable_insert(IO_key_table, "kp 7", (void*)SDLK_KP7);
    hashtable_insert(IO_key_table, "kp 8", (void*)SDLK_KP8);
    hashtable_insert(IO_key_table, "kp 9", (void*)SDLK_KP9);    
 
    hashtable_insert(IO_key_table, "kp .",   (void*)SDLK_KP_PERIOD);
    hashtable_insert(IO_key_table, "kp /",   (void*)SDLK_KP_DIVIDE);
    hashtable_insert(IO_key_table, "kp *",   (void*)SDLK_KP_MULTIPLY);
    hashtable_insert(IO_key_table, "kp -",   (void*)SDLK_KP_MINUS);
    hashtable_insert(IO_key_table, "kp +",   (void*)SDLK_KP_PLUS);
    hashtable_insert(IO_key_table, "enter",  (void*)SDLK_KP_ENTER);
    hashtable_insert(IO_key_table, "kp =",   (void*)SDLK_KP_EQUALS);
    hashtable_insert(IO_key_table, "delete", (void*)SDLK_DELETE);
        
    hashtable_insert(IO_key_table, "a", (void*)SDLK_a);
    hashtable_insert(IO_key_table, "b", (void*)SDLK_b);
    hashtable_insert(IO_key_table, "c", (void*)SDLK_c);
    hashtable_insert(IO_key_table, "d", (void*)SDLK_d);
    hashtable_insert(IO_key_table, "e", (void*)SDLK_e);
    hashtable_insert(IO_key_table, "f", (void*)SDLK_f);
    hashtable_insert(IO_key_table, "g", (void*)SDLK_g);
    hashtable_insert(IO_key_table, "h", (void*)SDLK_h);
    hashtable_insert(IO_key_table, "i", (void*)SDLK_i);
    hashtable_insert(IO_key_table, "j", (void*)SDLK_j);
    hashtable_insert(IO_key_table, "k", (void*)SDLK_k);
    hashtable_insert(IO_key_table, "l", (void*)SDLK_l);
    hashtable_insert(IO_key_table, "m", (void*)SDLK_m);
    hashtable_insert(IO_key_table, "n", (void*)SDLK_n);
    hashtable_insert(IO_key_table, "o", (void*)SDLK_o);
    hashtable_insert(IO_key_table, "p", (void*)SDLK_p);
    hashtable_insert(IO_key_table, "q", (void*)SDLK_q);
    hashtable_insert(IO_key_table, "r", (void*)SDLK_r);
    hashtable_insert(IO_key_table, "s", (void*)SDLK_s);
    hashtable_insert(IO_key_table, "t", (void*)SDLK_t);
    hashtable_insert(IO_key_table, "u", (void*)SDLK_u);
    hashtable_insert(IO_key_table, "v", (void*)SDLK_v);
    hashtable_insert(IO_key_table, "w", (void*)SDLK_w);
    hashtable_insert(IO_key_table, "x", (void*)SDLK_x);
    hashtable_insert(IO_key_table, "y", (void*)SDLK_y);
    hashtable_insert(IO_key_table, "z", (void*)SDLK_z);
    
    hashtable_insert(IO_key_table, "f1",     (void*)SDLK_F1);
    hashtable_insert(IO_key_table, "f2",     (void*)SDLK_F2);
    hashtable_insert(IO_key_table, "f3",     (void*)SDLK_F3);
    hashtable_insert(IO_key_table, "f4",     (void*)SDLK_F4);
    hashtable_insert(IO_key_table, "f5",     (void*)SDLK_F5);
    hashtable_insert(IO_key_table, "f6",     (void*)SDLK_F6);
    hashtable_insert(IO_key_table, "f7",     (void*)SDLK_F7);
    hashtable_insert(IO_key_table, "f8",     (void*)SDLK_F8);
    hashtable_insert(IO_key_table, "f9",     (void*)SDLK_F9);
    hashtable_insert(IO_key_table, "f10",    (void*)SDLK_F10);
    hashtable_insert(IO_key_table, "f11",    (void*)SDLK_F11);
    hashtable_insert(IO_key_table, "f12",    (void*)SDLK_F12);
    hashtable_insert(IO_key_table, "f13",    (void*)SDLK_F13);
    hashtable_insert(IO_key_table, "f14",    (void*)SDLK_F14);
    hashtable_insert(IO_key_table, "f15",    (void*)SDLK_F15);

    hashtable_insert(IO_key_table, "up",     (void*)SDLK_UP);
    hashtable_insert(IO_key_table, "down",   (void*)SDLK_DOWN);
    hashtable_insert(IO_key_table, "left",   (void*)SDLK_LEFT);
    hashtable_insert(IO_key_table, "right",  (void*)SDLK_RIGHT);

    hashtable_insert(IO_key_table, "insert",     (void*)SDLK_INSERT);
    hashtable_insert(IO_key_table, "home",       (void*)SDLK_HOME);
    hashtable_insert(IO_key_table, "end",        (void*)SDLK_END);
    hashtable_insert(IO_key_table, "page up",    (void*)SDLK_PAGEUP);
    hashtable_insert(IO_key_table, "page down",  (void*)SDLK_PAGEDOWN);
        
}

oo_error_t
io_manager_init(void)
{
	// ground all keyboard actions
	for (int i = 0 ; i < IO_MAX_KEYS ; i ++) {
		IO_manager.keyboard.button[i].up.is_script = false;
        IO_manager.keyboard.button[i].up.u.c = io_button_action_def;
        
        IO_manager.keyboard.button[i].down.is_script = false;
        IO_manager.keyboard.button[i].down.u.c = io_button_action_def;
   	}
    
	// ground all mouse actions	
	for (int i = 0 ; i < IO_MAX_MOUSE_BUTTONS ; i ++) {
		IO_manager.mouse.button[i].up.is_click_action = true;
        IO_manager.mouse.button[i].up.u.click.is_script = false;
        IO_manager.mouse.button[i].up.u.click.u.c = io_click_action_def;
        
        IO_manager.mouse.button[i].down.is_click_action = true;
        IO_manager.mouse.button[i].down.u.click.is_script = false;
        IO_manager.mouse.button[i].down.u.click.u.c = io_click_action_def;
        
        IO_manager.mouse.button[i].drag.is_script = false;
        IO_manager.mouse.button[i].drag.u.c = io_drag_action_def;        
	}
	
	// ground all joystick actions
	
	for (int i = 0 ; i < IO_MAX_JOYSTICKS ; i ++) {
		for (int j = 0 ; j < IO_MAX_JOYSTICK_BUTTONS ; j ++) { 
			IO_manager.joystick[i].button[j].up.is_script = false;
            IO_manager.joystick[i].button[j].up.u.c = io_button_action_def;

            IO_manager.joystick[i].button[j].down.is_script = false;
            IO_manager.joystick[i].button[j].down.u.c = io_button_action_def;
        }
		for (int j = 0 ; j < IO_MAX_JOYSTICK_AXIS ; j ++) { 
            IO_manager.joystick[i].axis[j].is_script = false;
            IO_manager.joystick[i].axis[j].u.c = io_axis_action_def;
        }
	}
    
    io_init_keytable();
    io_init_actiontable();
    
	return ERROR_NONE;
}


void
io_handle_key_down(uint32_t key, uint16_t special)
{
    // TODO: Error on to high numbers...
    if (IO_manager.keyboard.button[key].down.is_script) {
        PyObject *f = IO_manager.keyboard.button[key].down.u.s;
            
            PyObject *res = PyObject_CallFunction(f, "");
            if (! res) PyErr_Print();
            Py_XDECREF(res);
    } else {
        IO_manager.keyboard.button[key].down.u.c();
    }
}

void
io_handle_key_up(uint32_t key, uint16_t special)
{
    // TODO: Error on to high numbers...
    if (IO_manager.keyboard.button[key].up.is_script) {
            PyObject *f = IO_manager.keyboard.button[key].up.u.s;
            
            PyObject *res = PyObject_CallFunction(f, "");
            if (! res) PyErr_Print();
            Py_XDECREF(res);
    } else {
        IO_manager.keyboard.button[key].up.u.c();
    }
}


void
io_handle_mouse_down(short button, float x, float y)
{
    // TODO: Error on to high numbers...
    if (IO_manager.mouse.button[button].down.is_click_action) {
        if (IO_manager.mouse.button[button].down.u.click.is_script) {
            
            PyObject *f = IO_manager.mouse.button[button].down.u.click.u.s;
            
            PyObject *res = PyObject_CallFunction(f, "(dd)", (double)x, (double)y);
            if (! res) PyErr_Print();
            Py_XDECREF(res);
        } else {     
            IO_manager.mouse.button[button].down.u.click.u.c(x, y);            
        }
    } else {
        if (IO_manager.mouse.button[button].down.u.button.is_script) {
            PyObject *f = IO_manager.mouse.button[button].down.u.button.u.s;
            PyObject *res = PyObject_CallFunction(f, "");
            if (! res) PyErr_Print();
            Py_XDECREF(res);
        } else {
            IO_manager.mouse.button[button].down.u.button.u.c();            
        }
    }
}

void
io_handle_mouse_up(short button, float x, float y)
{
    // TODO: Error on to high numbers...
    if (IO_manager.mouse.button[button].up.is_click_action) {
        if (IO_manager.mouse.button[button].up.u.click.is_script) {
            PyObject *f = IO_manager.mouse.button[button].up.u.click.u.s;
            
            PyObject *res = PyObject_CallFunction(f, "(dd)", (double)x, (double)y);
            if (! res) PyErr_Print();

            Py_XDECREF(res);
        } else {
            IO_manager.mouse.button[button].up.u.click.u.c(x, y);            
        }
    } else {
        if (IO_manager.mouse.button[button].up.u.button.is_script) {
            PyObject *f = IO_manager.mouse.button[button].up.u.button.u.s;
            PyObject *res = PyObject_CallFunction(f, "");
            if (! res) PyErr_Print();
            Py_XDECREF(res);
        } else {
            IO_manager.mouse.button[button].up.u.button.u.c();
        }
    }
}


void
io_handle_mouse_drag(short button, float dx, float dy)
{
    if (IO_manager.mouse.button[button].drag.is_script) {
            PyObject *f = IO_manager.mouse.button[button].drag.u.s;

            PyObject *res = PyObject_CallFunction(f, "(dd)", (double)dx, (double)dy);
            if (! res) PyErr_Print();
            Py_XDECREF(res);
        } else {
            IO_manager.mouse.button[button].drag.u.c(dx, dy);            
        }
}

void
io_handle_joystick_button_down(short joystick, short button)
{
    if (IO_manager.joystick[joystick].button[button].down.is_script) {
        PyObject *f = IO_manager.joystick[joystick].button[button].down.u.s;
        PyObject *res = PyObject_CallFunction(f, "");
        if (! res) PyErr_Print();
        Py_XDECREF(res);
    } else {
        IO_manager.joystick[joystick].button[button].down.u.c();
    }
}

void
io_handle_joystick_button_up(short joystick, short button)
{
    if (IO_manager.joystick[joystick].button[button].up.is_script) {
        PyObject *f = IO_manager.joystick[joystick].button[button].up.u.s;
        PyObject *res = PyObject_CallFunction(f, "");
        if (! res) PyErr_Print();
        Py_XDECREF(res);
    } else {
        IO_manager.joystick[joystick].button[button].up.u.c();
    }
}

void
io_handle_joystick_axis(short joystick, short axis, float val)
{
    if (IO_manager.joystick[joystick].axis[axis].is_script) {
        PyObject *f = IO_manager.joystick[joystick].axis[axis].u.s;

        PyObject *res = PyObject_CallFunction(f, "(d)", (double)val);
        if (! res) PyErr_Print();

        Py_XDECREF(res);
    } else {
        IO_manager.joystick[joystick].axis[axis].u.c(val);
    }
}
    
bool
io_register_mouse_click_handler(const char *key, io_click_action_f f) {
    io_action_t *ah;
    if (! (ah = malloc(sizeof(io_action_t))) ) {
        return false;
    }
    
    ah->t = IO_ACTION_CLICK;
    ah->u.click = f;
    
    hashtable_insert(IO_action_table, key, ah);
    
    return true;
}
bool
io_register_button_handler(const char *key, io_button_action_f f)
{
    io_action_t *ah;
    if (! (ah = malloc(sizeof(io_action_t))) ) {
        return false;
    }

    ah->t = IO_ACTION_BUTTON;
    ah->u.button = f;

    if (hashtable_insert(IO_action_table, key, ah)) return false;
        
    return true;
}

bool
io_register_mouse_drag_handler(const char *key, io_drag_action_f f)
{
    io_action_t *ah;
    if (! (ah = malloc(sizeof(io_action_t))) ) {
        return false;
    }
    
    ah->t = IO_ACTION_DRAG;
    ah->u.drag = f;
    
    hashtable_insert(IO_action_table, key, ah);
    return true;
}

bool
io_register_joystic_axis_handler(const char *key, io_axis_action_f f)
{
    io_action_t *ah;
    if (! (ah = malloc(sizeof(io_action_t))) ) {
        return false;
    }
    
    ah->t = IO_ACTION_AXIS;
    ah->u.axis = f;
    
    hashtable_insert(IO_action_table, key, ah);
    return true;
}

int
io_register_mouse_click_handler_script(const char *key, scr_func_t f)
{
    io_action_t *ah;    
    if (! (ah = malloc(sizeof(io_action_t))) ) {
        return 1;
    }
    
    if (! f) return 1;
    
    Py_INCREF(f);
    
    ah->t = IO_ACTION_CLICK_SCRIPT;
    ah->u.script = f;
    
    hashtable_insert(IO_action_table, key, ah);
    return 0;
}
int
io_register_button_handler_script(const char *key, scr_func_t f)
{
    io_action_t *ah;    
    if (! (ah = malloc(sizeof(io_action_t))) ) {
        return 1;
    }
    
    if (! f) return 1;
    
    Py_INCREF(f);
    ah->t = IO_ACTION_BUTTON_SCRIPT;
    ah->u.script = f;
   
    hashtable_insert(IO_action_table, key, ah);
    return 0;
}

int
io_register_mouse_drag_handler_script(const char *key, scr_func_t f)
{
    io_action_t *ah;    
    if (! (ah = malloc(sizeof(io_action_t))) ) {
        return 1;
    }
    
    if (! f) return 1;
    
    Py_INCREF(f);
    ah->t = IO_ACTION_DRAG_SCRIPT;
    ah->u.script = f;
   
    hashtable_insert(IO_action_table, key, ah);
    return 0;
}

int
io_register_joystic_axis_handler_script(const char *key, scr_func_t f)
{
    io_action_t *ah;    
    if (! (ah = malloc(sizeof(io_action_t))) ) {
        return 1;
    }
    
    if (! f) return 1;
    
    Py_INCREF(f);
    ah->t = IO_ACTION_AXIS_SCRIPT;
    ah->u.script = f;
   
    hashtable_insert(IO_action_table, key, ah);
    return 0;
}



void
io_bind_key_down(char *key, short mod, char *action_key)
{
    int key_code;
    io_action_t *action;
    
    key_code = (int) hashtable_lookup(IO_key_table, key);
    action = (io_action_t*) hashtable_lookup(IO_action_table, action_key);
    // silently fail on key code 0 (hashtable returns null if it cannot find the key)
    // this mean that keycode 0 cannot be mapped, though if we use SDL, this represent unknown keys anyway
    if (! key_code)  printf("could not find key code \"%s\"\n", key);
    if (! action)  printf("could not find action key code \"%s\"\n", action_key);
    
    if ((! key_code) || (! action)) {
        return;
    }
    
    if (action->t == IO_ACTION_BUTTON) {
        IO_manager.keyboard.button[key_code].down.is_script = false;
        IO_manager.keyboard.button[key_code].down.u.c = action->u.button;
    } else if (action->t == IO_ACTION_BUTTON_SCRIPT) {
        IO_manager.keyboard.button[key_code].down.is_script = true;
        IO_manager.keyboard.button[key_code].down.u.s = action->u.script;
    }
}

void
io_bind_key_up(char *key, short mod, char *action_key)
{
    int key_code;
    io_action_t *action;
    
    key_code = (int) hashtable_lookup(IO_key_table, key);
    action = (io_action_t*) hashtable_lookup(IO_action_table, action_key);
    // silently fail on key code 0 (hashtable returns null if it cannot find the key)
    // this mean that keycode 0 cannot be mapped, though if we use SDL, this represent unknown keys anyway
    if ((! key_code) || (! action)) {
        return;
    }
    
    if (action->t == IO_ACTION_BUTTON) {
        IO_manager.keyboard.button[key_code].up.is_script = false;
        IO_manager.keyboard.button[key_code].up.u.c = action->u.button;
    } else if (action->t == IO_ACTION_BUTTON_SCRIPT) {
        IO_manager.keyboard.button[key_code].up.is_script = true;
        IO_manager.keyboard.button[key_code].up.u.s = action->u.script;
    }
}

void
io_bind_mouse_down(short button, char *action_key)
{
    io_action_t *action;
    
    action = (io_action_t*) hashtable_lookup(IO_action_table, action_key);

    
    if ( ! action ) {
        return;
    }
      
    if (action->t == IO_ACTION_BUTTON) {        
        IO_manager.mouse.button[button].down.is_click_action = false;
        IO_manager.mouse.button[button].down.u.button.is_script = false;
        IO_manager.mouse.button[button].down.u.button.u.c = action->u.button;
    } else if (action->t == IO_ACTION_BUTTON_SCRIPT) {
        IO_manager.mouse.button[button].down.is_click_action = false;
        IO_manager.mouse.button[button].down.u.button.is_script = true;
        IO_manager.mouse.button[button].down.u.button.u.s = action->u.script;
    } else if (action->t == IO_ACTION_CLICK) {
        IO_manager.mouse.button[button].down.is_click_action = true;
        IO_manager.mouse.button[button].down.u.click.is_script = false;
        IO_manager.mouse.button[button].down.u.click.u.c = action->u.click;
    } else if (action->t == IO_ACTION_CLICK_SCRIPT) {
        IO_manager.mouse.button[button].down.is_click_action = true;
        IO_manager.mouse.button[button].down.u.click.is_script = true;
        IO_manager.mouse.button[button].down.u.click.u.s = action->u.script;
    }
}
void
io_bind_mouse_up(short button, char *action_key)
{
    io_action_t *action;
    
    action = (io_action_t*) hashtable_lookup(IO_action_table, action_key);

    if ( ! action ) {
        return;
    }
    
    if (action->t == IO_ACTION_BUTTON) {
        IO_manager.mouse.button[button].up.is_click_action = false;
        IO_manager.mouse.button[button].up.u.button.is_script = false;
        IO_manager.mouse.button[button].up.u.button.u.c = action->u.button;
    } else if (action->t == IO_ACTION_BUTTON_SCRIPT) {
        IO_manager.mouse.button[button].up.is_click_action = false;
        IO_manager.mouse.button[button].up.u.button.is_script = true;
        IO_manager.mouse.button[button].up.u.button.u.s = action->u.script;
    } else if (action->t == IO_ACTION_CLICK) {
        IO_manager.mouse.button[button].up.is_click_action = true;
        IO_manager.mouse.button[button].up.u.click.is_script = false;
        IO_manager.mouse.button[button].up.u.click.u.c = action->u.click;
    } else if (action->t == IO_ACTION_CLICK_SCRIPT) {
        IO_manager.mouse.button[button].up.is_click_action = true;
        IO_manager.mouse.button[button].up.u.click.is_script = true;
        IO_manager.mouse.button[button].up.u.click.u.s = action->u.script;
    }
}

void
io_bind_mouse_drag(short button, char *action_key)
{
    io_action_t *action;
    
    action = (io_action_t*) hashtable_lookup(IO_action_table, action_key);

    if ( ! action ) {
        return;
    }
    
    if (action->t == IO_ACTION_DRAG) {
        IO_manager.mouse.button[button].drag.is_script = false;
        IO_manager.mouse.button[button].drag.u.c = action->u.drag;
    } else if (action->t == IO_ACTION_DRAG_SCRIPT) {
        IO_manager.mouse.button[button].drag.is_script = true;
        IO_manager.mouse.button[button].drag.u.s = action->u.script;
    }
}

void
io_bind_joystick_axis(int joystick_id, int axis, char *action)
{
}
void
io_bind_joystick_button_down(int joystick_id, int button, char *action)
{
}
void
io_bind_joystick_button_up(int joystick_id, int button, char *action)
{
}
