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

    
cdef extern from "settings.h":
    int ooConfSetBoolAsInt(char *key, int val)
    int ooConfGetBoolAsInt(char *key, int *val)    
    int ooConfSetInt(char *key, int val)
    int ooConfGetInt(char *key, int *val)
    int ooConfSetFloat(char *key, float val)
    int ooConfGetFloat(char *key, float *val)
    int ooConfSetStr(char *key, char *val)
    char* ooConfGetStr(char *key)


def setScreenSize(short w, short h):
    ooConfSetInt("video.width", w)
    ooConfSetInt("video.height", h)

def setFullscreen(int fs):
    if fs:
        ooConfSetBoolAsInt("video.fullscreen", 1)
    else:
        ooConfSetBoolAsInt("video.fullscreen", 0)

def toggleFullscreen():
    cdef int fs
    fs = 0
    res = ooConfGetBoolAsInt("video.fullscreen", &fs) 
    if fs != 0:
        ooConfSetBoolAsInt("video.fullscreen", 0)
    else:
        ooConfSetBoolAsInt("video.fullscreen", 1)

def setScreenDepth(short d):
    ooConfSetInt("video.depth", d)

def setGLFovy(float f):
    ooConfSetFloat("video.gl.fovy", f)
    
def setGLAspect(float f):
    ooConfSetFloat("video.gl.aspect", f)

    