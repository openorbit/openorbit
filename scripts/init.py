# This is a sample configuration file for Open Orbit. The configfile is actually
# written in Python, so although this one is pretty booring, it is possible to
# write a fully fledged program here that will be executed during boot of Open
# Orbit. The following defined variables have a special meaning though as they
# are the ones extracted after the script has been executed. Changing them later
# on is not guaranteed to have any effect.
# More about the config file can be read in the documentation.

# For the moment there is a slight problem with running this, since the
# graphics system is not initialised, this will change in the future however

print "Loading config..."

import sys
import os

import io
import config


config.setScreenSize(800, 600)
#config.setFullscreen(1)
#config.setScreenDepth*32)

def foo():
    print "button pressed"

def bar(a, b):
    print a, b

def drag(a, b):
    print "drag" + str((a, b))

# Register the function above as an action handler
io.registerButtonHandler("foo", foo)
io.registerClickHandler("bar", bar)
io.registerDragHandler("drag", drag)


# io.bindKeyUp can bind both C-function and Python functions to button presses
# This is why you have to register the button handler function, i.e. to make
# the interface identical, independent of whether you are using C or Python.    

#io.bindKeyDown("a", 0, "foo")
#io.bindMouseDown(io.LEFT, "bar")
#io.bindMouseDown(io.RIGHT, "foo")
#io.bindMouseDrag(io.LEFT, "drag")

# Translation movements
io.bindKeyDown("e", 0, "cam-fwd")
io.bindKeyDown("d", 0, "cam-back")
io.bindKeyDown("s", 0, "cam-left")
io.bindKeyDown("f", 0, "cam-right")
io.bindKeyDown("a", 0, "cam-up")
io.bindKeyDown("z", 0, "cam-down")

# Rotation
io.bindKeyDown("g", 0, "cam-pitch-up")
io.bindKeyDown("t", 0, "cam-pitch-down")
io.bindKeyDown("v", 0, "cam-yaw-right")
io.bindKeyDown("x", 0, "cam-yaw-left")
io.bindKeyDown("w", 0, "cam-roll-left")
io.bindKeyDown("r", 0, "cam-roll-right")
