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

import io       # I/O module, allowing the binding of key handlers
import config   # config, allows one to set config values


config.setScreenSize(800, 600)
#config.setFullscreen(1)
#config.setScreenDepth(32)

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
