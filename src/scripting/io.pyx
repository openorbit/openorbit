#   Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

#   This file is part of Open Orbit.
#
#   Open Orbit is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Open Orbit is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

# Maps in the io-manager functions that are interesting to expose in python.
# Note that this interface will probably change. The registration of functions
# is necessary as IO-actions should be possible to implement in C and be
# assigned through the same interface

cdef extern from "io-manager.h":
    void ooIoRegPyKeyHandler(char *name, object handlerFunc)
    void ooIoBindKeyHandler(char *keyName, char *keyAction, int up,
                            unsigned short mask)

# Force generation of header-file

OO_IO_MOD_NONE   = 0x0000
OO_IO_MOD_LSHIFT = 0x0001
OO_IO_MOD_RSHIFT = 0x0002
OO_IO_MOD_LCTRL  = 0x0040
OO_IO_MOD_RCTRL  = 0x0080
OO_IO_MOD_LALT   = 0x0100
OO_IO_MOD_RALT   = 0x0200
OO_IO_MOD_LMETA  = 0x0400
OO_IO_MOD_RMETA  = 0x0800
OO_IO_MOD_NUM    = 0x1000
OO_IO_MOD_CAPS   = 0x2000
OO_IO_MOD_MODE   = 0x4000

LEFT = 1
MIDDLE = 2
RIGHT = 3

def bindKeyUp(char *key, int modMask, char *action):
    ooIoBindKeyHandler(key, action, 1, modMask)
#    io_bind_key_up(key, modMask, action)
	
def bindKeyDown(char *key, int modMask, char *action):
    ooIoBindKeyHandler(key, action, 0, modMask)

# Should throw exceptions instead...
def registerButtonHandler(char *key, object f):
    ooIoRegPyKeyHandler(key, f)

