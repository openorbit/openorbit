/* Open Orbit - An interactive spacecraft simulator.
 * Copyright (C) 2006  Mattias Holm (mattias.holm@contra.nu)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Linking Open Orbit statically or dynamically with other modules is making a
 * combined work based on Open Orbit.  Thus, the terms and conditions of the
 * GNU General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of Open Orbit give you
 * permission to link Open Orbit with independent modules that communicate with
 * Open Orbit solely through the public interfaces (defined as such in the
 * documentation), regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under terms
 * of your choice, provided that every copy of the combined work is accompanied
 * by a complete copy of the source code of Open Orbit (the version of Open
 * Orbit used to produce the combined work), being distributed under the terms
 * of the GNU General Public License plus this exception. An independent module
 * is a module which is not derived from or based on Open Orbit.
 *
 * Note that people who make modified versions of Open Orbit are not obligated
 * to grant this special exception for their modified versions; it is
 * their choice whether to do so.  The GNU General Public License gives
 * permission to release a modified version without this exception; this
 * exception also makes it possible to release a modified version which
 * carries forward this exception.
 */

#ifndef __IO_MANAGER_H__
#define __IO_MANAGER_H__
#ifdef __cplusplus
extern "C" {
#endif
    

#include <stdbool.h>
#include "scripting/scripting.h"
#include "error.h"

#define IO_MAX_KEYS             (512)
#define IO_MAX_MOD_COMBOS         (4)
#define IO_MAX_MOUSE_BUTTONS     (16)
#define IO_MAX_JOYSTICKS          (2)
#define IO_MAX_JOYSTICK_AXIS      (8)
#define IO_MAX_JOYSTICK_BUTTONS  (16)

#define IO_MOD_SHIFT    (0x01)
#define IO_MOD_ALT      (0x02)
#define IO_MOD_META     (0x04)
#define IO_MOD_CTRL     (0x08)

/* Used by keyboards, joysticks and mice. With the latter, no position is
    transmitted compared to the click action.
 */
typedef void (*io_button_action_f)(void);

/* Invoked when a joystick axis changes values, param is a bipolar
 * axis value (-1.0 to 1.0)
 * */
typedef void (*io_axis_action_f)(float);

/* Used for handling mouse actions, x, y positions (normalised) or dx, dy
 * differentials (also normalised) */
typedef void (*io_click_action_f)(float, float);
typedef void (*io_drag_action_f)(float, float);

/* Used in the hash table of actions */

typedef enum {
    IO_EV_Any,
    IO_EV_Key,
    IO_EV_Click,
    IO_EV_Drag,
    IO_EV_Button,
    IO_EV_Axis
} io_event_kind_t;

typedef struct {
    io_event_kind_t kind;
    union {
        struct {
            bool down;
            int button;
            float x, y;
        } click;
        
        struct {
            int button;
            float x, y;
            float dx, dy;
        } drag;
        
        struct {
            int key_id;
            short modifiers;
            bool down;
        } key;
        
        struct {
            int joystick_id;
            int button_id;
            bool down;
        } button;
        
        struct {
            int joystick_id;
            int axis_id;
            float x;
            float dx;
        } axis;
    } u;
} io_event_t;


typedef void (*io_event_handler_c_f)(io_event_t event);

typedef struct {
    bool is_script;
    union {
        scr_func_t s;
        io_event_handler_c_f c;
    } u;
} io_event_handler_t;

typedef struct {
    io_event_kind_t kind;
    io_event_handler_t handler;
} io_event_handler_container_t;

typedef struct {
    io_event_handler_t up;
    io_event_handler_t down; 
} io_button_handler_t;

typedef struct {
    short modifier_mask;
    io_event_handler_t up;
    io_event_handler_t down;
} io_key_handler_t;


typedef struct {
	// should make this more complex in order to support unicode
	// should handle special keys like shift and return
	struct {
        io_key_handler_t key[IO_MAX_KEYS][IO_MAX_MOD_COMBOS];
	} keyboard;
	struct {
        io_button_handler_t button[IO_MAX_MOUSE_BUTTONS];
        io_event_handler_t drag[IO_MAX_MOUSE_BUTTONS];
	} mouse;
	struct {
        io_button_handler_t button[IO_MAX_JOYSTICK_BUTTONS];
        io_event_handler_t axis[IO_MAX_JOYSTICK_AXIS];
   	} joystick[IO_MAX_JOYSTICKS];
} io_bindings_t;

bool io_register_event_handler(const char *key, io_event_handler_c_f f,
                               io_event_kind_t kind);
int io_register_event_handler_script(const char *key, scr_func_t f,
                                     io_event_kind_t kind);


typedef enum {
    IO_ACTION_BUTTON,
    IO_ACTION_CLICK,
    IO_ACTION_DRAG,
    IO_ACTION_AXIS,
    IO_ACTION_BUTTON_SCRIPT,
    IO_ACTION_CLICK_SCRIPT,
    IO_ACTION_DRAG_SCRIPT,
    IO_ACTION_AXIS_SCRIPT
} io_action_type;

typedef union {
    io_button_action_f button;
    io_click_action_f click;
    io_drag_action_f drag;
    io_axis_action_f axis;
    scr_func_t script;
} io_action_u;

typedef struct {
    io_action_type t;
	io_action_u u;
} io_action_t;


oo_error_t io_manager_init(void);
bool io_register_mouse_click_handler(const char *key, io_click_action_f f);
bool io_register_button_handler(const char *key, io_button_action_f f);
bool io_register_mouse_drag_handler(const char *key, io_drag_action_f f);
bool io_register_joystic_axis_handler(const char *key, io_axis_action_f f);

int io_register_mouse_click_handler_script(const char *key, scr_func_t f);
int io_register_button_handler_script(const char *key, scr_func_t f);
int io_register_mouse_drag_handler_script(const char *key, scr_func_t f);
int io_register_joystic_axis_handler_script(const char *key, scr_func_t f);



typedef struct {
	// should make this more complex in order to support unicode
	// should handle special keys like shift and return
	struct {
        struct {
            struct {
                bool is_script;
                union {
                    scr_func_t s;
                    io_button_action_f c;
                } u;
            } up;
            struct {
                bool is_script;
                union {
                    scr_func_t s;
                    io_button_action_f c;
                } u;
            } down;
        } button[IO_MAX_KEYS];
	} keyboard;
	struct {
        struct {
            struct {
                bool is_click_action;
                union {
                    struct {
                        bool is_script;
                        union {
                            scr_func_t s;
                            io_button_action_f c;
                        } u;
                    } button;
                    struct {
                        bool is_script;
                        union {
                            scr_func_t s;
                            io_click_action_f c;
                        } u;
                    } click;
                } u;
            } up;
            struct {
                bool is_click_action;
                union {
                    struct {
                        bool is_script;
                        union {
                            scr_func_t s;
                            io_button_action_f c;
                        } u;
                    } button;
                    struct {
                        bool is_script;
                        union {
                            scr_func_t s;
                            io_click_action_f c;
                        } u;
                    } click;
                } u;
            } down;
            struct {
                bool is_script;
                union {
                    scr_func_t s;
                    io_drag_action_f c;
                } u;
            } drag;
        } button[IO_MAX_MOUSE_BUTTONS];
	} mouse;
	// two joysticks allowed
	struct {
        struct {
            struct {
                bool is_script;
                union {
                    scr_func_t s;
                    io_button_action_f c;
                } u;
            } up;
            struct {
                bool is_script;
                union {
                    scr_func_t s;
                    io_button_action_f c;
                } u;
            } down; 
        } button[IO_MAX_JOYSTICK_BUTTONS];
        struct {
            bool is_script;
            union {
                scr_func_t s;
                io_axis_action_f c;
            } u;
        } axis[IO_MAX_JOYSTICK_AXIS];
   	} joystick[IO_MAX_JOYSTICKS];
} io_manager_t;


// handles mapping of keypresses to the registered functions, note the interfaces

void io_handle_key_down(uint32_t key, uint16_t special);
void io_handle_key_up(uint32_t key, uint16_t special);
void io_handle_mouse_down(short button, float x, float y);
void io_handle_mouse_up(short button, float x, float y);
void io_handle_mouse_drag(short button, float dx, float dy);
void io_handle_joystick_button_down(short joystick, short button);
void io_handle_joystick_button_up(short joystick, short button);
void io_handle_joystick_axis(short joystick, short axis, float val);


void io_bind_key_down(char *key, short mod, char *action);
void io_bind_key_up(char *key, short mod, char *action);
void io_bind_mouse_down(short button, char *action);
void io_bind_mouse_up(short button, char *action);
void io_bind_mouse_drag(short button, char *action);
void io_bind_joystick_axis(int joystick_id, int axis, char *action);
void io_bind_joystick_button_down(int joystick_id, int button, char *action);
void io_bind_joystick_button_up(int joystick_id, int button, char *action);

#ifdef __cplusplus
}
#endif
    
#endif /* ! __IO_MANAGER_H */
