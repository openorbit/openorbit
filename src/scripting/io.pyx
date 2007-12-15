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
cdef public int dummy 

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
