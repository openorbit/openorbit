cdef extern from "settings.h":
    void set_screen_size(short w, short h)
    void set_fullscreen(int fs)
    void toggle_fullscreen()
    void set_screen_depth(short d)

cdef public int dummy

def setScreenSize(short w, short h):
    set_screen_size(w, h)

def setFullscreen(int fs):
    set_fullscreen(fs)

def toggleFullscreen():
    toggle_fullscreen()

def setScreenDepth(short d):
    set_screen_depth(d)
