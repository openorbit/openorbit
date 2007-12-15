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

ctypedef int _Bool
    
cdef extern from "settings.h":
    int conf_set_bool(char *key, _Bool val)
    int conf_get_bool(char *key, _Bool *val)    
    int conf_set_int(char *key, int val)
    int conf_get_int(char *key, int *val)
    int conf_set_float(char *key, float val)
    int conf_get_float(char *key, float *val)
    int conf_set_str(char *key, char *val)
    char* conf_get_str(char *key)


def setScreenSize(short w, short h):
    conf_set_int("video.width", w)
    conf_set_int("video.height", h)

def setFullscreen(int fs):
    conf_set_bool("video.fullscreen", fs)


def toggleFullscreen():
    cdef _Bool fs
    fs = 0
    # generates a warning as fs is an int here, but we cannot really work it
    # out in any other way a c-bool will be less then or equal in size to an int
    # so we can assume that this actually is technically correct
    res = conf_get_bool("video.fullscreen", &fs) 
    if fs != 0: # not fully correct, works with gcc though
        conf_set_bool("video.fullscreen", 0)
    else:
        conf_set_bool("video.fullscreen", 1)

def setScreenDepth(short d):
    conf_set_int("video.depth", d)

