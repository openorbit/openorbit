#   Copyright 2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

cdef extern from "plugin-handler.h":
    int ooPluginLoadAll()
    char *ooPluginLoad(char *filename)
    void ooPluginUnload(char *key)
	
def LoadAll():
    ooPluginLoadAll()
    
def Load(name):
    return ooPluginLoad(name)
    
def Unload(key):
    ooPluginUnload(key)
