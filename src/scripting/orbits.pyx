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

import texture
import res

import ode
cimport ode
cimport sg
cimport orbits

cdef class OrbitSys:
    def __init__(self):
        self.osys = <OOorbsys*>0
    def __dealloc__(self):
        # C function call to delete obj_sys object.
        pass
    def new(self, scg, char *fileName):
      scenegraph = scg.getSg()
      self.osys = ooOrbitLoad(<OOscenegraph*>scenegraph, fileName)
      return self
