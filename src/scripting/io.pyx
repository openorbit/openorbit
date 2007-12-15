#   The contents of this file are subject to the Mozilla Public License
#   Version 1.1 (the "License"); you may not use this file except in compliance
#   with the License. You may obtain a copy of the License at
#   http://www.mozilla.org/MPL/
#   
#   Software distributed under the License is distributed on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#   for the specific language governing rights and limitations under the
#   License.
#   
#   The Original Code is the Open Orbit space flight simulator.
#   
#   The Initial Developer of the Original Code is Mattias Holm. Portions
#   created by the Initial Developer are Copyright (C) 2006 the
#   Initial Developer. All Rights Reserved.
#   
#   Contributor(s):
#       Mattias Holm <mattias.holm(at)contra.nu>.
#   
#   Alternatively, the contents of this file may be used under the terms of
#   either the GNU General Public License Version 2 or later (the "GPL"), or
#   the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
#   which case the provisions of GPL or the LGPL License are applicable instead
#   of those above. If you wish to allow use of your version of this file only
#   under the terms of the GPL or the LGPL and not to allow others to use your
#   version of this file under the MPL, indicate your decision by deleting the
#   provisions above and replace  them with the notice and other provisions
#   required by the GPL or the LGPL.  If you do not delete the provisions
#   above, a recipient may use your version of this file under either the MPL,
#   the GPL or the LGPL."

# Maps in the io-manager functions that are interesting to expose in python.
# Note that this interface will probably change. The registration of functions
# is necessary as IO-actions should be possible to implement in C and be
# assigned through the same interface

cdef extern from "io-manager.h":
    void io_bind_key_down(char *key, short mod, char *action)
    void io_bind_key_up(char *key, short mod, char *action)
    void io_bind_mouse_down(short button, char *action)
    void io_bind_mouse_up(short button, char *action)
    void io_bind_mouse_drag(short button, char *action_key)
    void io_bind_joystick_axis(short joystick_id, short axis, char *action)
    void io_bind_joystick_button_down(short joystick_id, short button, char *action)
    void io_bind_joystick_button_up(short joystick_id, short button, char *action)
    int io_register_mouse_click_handler_script(char *key, object f)
    int io_register_button_handler_script(char *key, object f)
    int io_register_mouse_drag_handler_script(char *key, object f)
    int io_register_joystic_axis_handler_script(char *key, object f)

# Force generation of header-file

SHIFT = 0x01
ALT = 0x02
META = 0x04
CTRL = 0x08

LEFT = 1
MIDDLE = 2
RIGHT = 3

def bindKeyUp(char *key, int modMask, char *action):
    io_bind_key_up(key, modMask, action)
	
def bindKeyDown(char *key, int modMask, char *action):
    io_bind_key_down(key, modMask, action)

def bindMouseDown(int button, char *action):
    io_bind_mouse_down(button, action)
    
def bindMouseUp(int button, char *action):
    io_bind_mouse_up(button, action)

def bindMouseDrag(int button, char *action):
    io_bind_mouse_drag(button, action)
    

def bindJoystickAxis(int joystick_id, int axis, char *action):
    io_bind_joystick_axis(joystick_id, axis, action)

def bindJoystickButtonDown(int joystickId, int button, char *action):
    io_bind_joystick_button_down(joystickId, button, action)

def bindJoystickButtonUp(int joystickId, int button, char *action):    
    io_bind_joystick_button_up(joystickId, button, action)

# Should throw exceptions instead...
def registerButtonHandler(char *key, object f):
    if io_register_button_handler_script(key, f) == 0:
        return False
    return True
    
def registerClickHandler(char *key, object f):
    if io_register_mouse_click_handler_script(key, f) == 0:
        return False
    return True
    
def registerDragHandler(char *key, object f):
    if io_register_mouse_drag_handler_script(key, f) == 0:
        return False
    return True
    
def registerAxisHandler(char *key, object f):
    if io_register_joystick_axis_handler_script(key, f) == 0:
        return False
    return True
