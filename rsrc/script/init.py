#   Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

#   This file is part of Open Orbit.
#
#   Open Orbit is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Open Orbit is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

# This is a sample configuration file for Open Orbit. The configfile is actually
# written in Python, so although this one is pretty booring, it is possible to
# write a fully fledged program here that will be executed during boot of Open
# Orbit. The following defined variables have a special meaning though as they
# are the ones extracted after the script has been executed. Changing them later
# on is not guaranteed to have any effect.
# More about the config file can be read in the documentation.

# For the moment there is a slight problem with running this, since the
# graphics system is not initialised, this will change in the future however

print "Running init script..."

#import io       # I/O module, allowing the binding of key handlers
#import config   # config, allows one to set config values
#import res

import math
#import ConfigParser
import sys
import os
from os import environ

home = environ["HOME"]
#prefsSearchPaths = [home + "/Library/Preferences/", home + "/.openorbit/"]
#resSearchPaths = [home + "/Library/Application Support/Open Orbit/", home + "/.openorbit/"]
# Find path that contains the openorbit.conf file, this path is our default dir
# for preferences, we should create the directory if it does not exist
#for path in prefsSearchPaths:
#    prefsPath = None
#    if os.path.isfile(os.path.join(path, "openorbit.conf")):
#        prefsPath = path
#        break

#if prefsPath == None:
    # not found
#    pass

# Find path with open orbit resources
#for path in resSearchPaths:
#    resPath = None
#    if os.path.isfile(os.path.join(path, "")):
#        resPath = path
#        break

#if resPath == None:
    # not found
#    pass

    
# A few default values for the config parser
#defaultValues = {'HOME' : home, 'RES': resPath}

#prefs = ConfigParser.ConfigParser(defaultValues)
#prefs.read([path + "openorbit.conf" for path in prefsSearchPaths])

# Populate the default preferences
#if not prefs.has_section("VIDEO"):
#    prefs.add_section("VIDEO")
#    prefs.set("VIDEO", "width", 800)
#    prefs.set("VIDEO", "height", 600)
#    prefs.set("VIDEO", "fullscreen", False)
#    prefs.set("VIDEO", "depth", 32)
#    prefs.set("VIDEO", "fovy", 45.0)
#    prefs.set("VIDEO", "aspect", 1.33)
#    
#if not prefs.has_section("AUDIO"):
#    prefs.add_section("AUDIO")
#    
#if not prefs.has_section("KEYBOARD"):
#    prefs.add_section("KEYBOARD")
#    prefs.set("KEYBOARD", "cam-fwd",        "e")
#    prefs.set("KEYBOARD", "cam-back",       "d")
#    prefs.set("KEYBOARD", "cam-left",       "s")
#    prefs.set("KEYBOARD", "cam-right",      "f")
#    prefs.set("KEYBOARD", "cam-up",         "a")
#    prefs.set("KEYBOARD", "cam-down",       "z")
#    prefs.set("KEYBOARD", "cam-pitch-up",   "g")
#    prefs.set("KEYBOARD", "cam-pitch-down", "t")
#    prefs.set("KEYBOARD", "cam-yaw-right",  "v")
#    prefs.set("KEYBOARD", "cam-yaw-left",   "x")
#    prefs.set("KEYBOARD", "cam-roll-left",  "w")
#    prefs.set("KEYBOARD", "cam-roll-right", "r")
#    
#if not prefs.has_section("MOUSE"):
#    prefs.add_section("MOUSE")
#
#if not prefs.has_section("SYS"):
#    prefs.add_section("SYS")
#    prefs.set("SYS", "log-level", "info")
#    
#


# Write all defaults to disk (maybe we should not write anything if there
# have been no modifications?)
# One thing that is todo here, is that the current usage of the options is
# rather messy. We want to read in any kind of option and insert it
# independent of type and name. We do not want to add config.setXXXYYY for
# every option needed, but for now, this is quite OK.

# I guess that the best is to (in the long run), write the options parser
# in yacc/flex/C and have it tightly connected with the internals.
#fp = open(os.path.join(prefsSearchPaths[1], "openorbit.conf"), "w")
#prefs.write(fp)
#fp.close()

#prefs = ConfigParser.ConfigParser(defaultValues)
#prefs.read([path + "openorbit.conf" for path in prefsSearchPaths])

#try:
#    videoWidth = prefs.getint("VIDEO", "width")
#    videoHeight = prefs.getint("VIDEO", "height")
#    videoFullscreen = prefs.getboolean("VIDEO", "fullscreen")
#    videoDepth = prefs.getint("VIDEO", "depth")
#    videoGlFovy = prefs.getfloat("VIDEO", "gl.fovy")
#    videoGlAspect = prefs.getfloat("VIDEO", "gl.aspect")
#except ConfigParser.NoOptionError:
#    print "Options missing parameter in section VIDEO"
#    sys.exit(1)
#except ConfigParser.NoSectionError:
#    print "VIDEO section missing"
#    sys.exit(1)
    
#config.setScreenSize(videoWidth, videoHeight)
#config.setFullscreen(videoFullscreen)
#config.setScreenDepth(videoDepth)
#config.setGLAspect(videoGlAspect)
#config.setGLFovy(videoGlFovy)

#camControlActionKeys = ["cam-fwd", "cam-back", "cam-left", "cam-right",
#                        "cam-up", "cam-down", "cam-pitch-up", "cam-pitch-down",
#                        "cam-yaw-right", "cam-yaw-left", "cam-roll-left",
#                        "cam-roll-right"]

#for key in camControlActionKeys:
#    try:
#        keyCode = prefs.get("KEYBOARD", key)
#        io.bindKeyDown(keyCode, 0, key)
#    except ConfigParser.NoOptionError:
#        # ignore missing bindings...
#        pass
#    except ConfigParser.NoSectionError:
#       print "fatal error, no KEYBOARD section"
#        sys.exit(1)
