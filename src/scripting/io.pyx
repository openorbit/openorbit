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

