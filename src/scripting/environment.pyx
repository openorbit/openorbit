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

import texture
import res


cdef class OrbitSys:
    def __cinit__(self, parent, char *name, float radius, float w0):
        self.orb_sys = NULL
        cdef orb_sys_t *osys
        if parent is None:
            self.orb_sys = orbit_add_sys(NULL, name, radius, w0)
        elif isinstance(parent, OrbitSys):
            osys = (<OrbitSys>parent).orb_sys
            self.orb_sys = orbit_add_sys(osys, name, radius, w0)
        else:
            raise TypeError("Parent not an OrbitObj object or None")
    def __dealloc__(self):
        # C function call to delete obj_sys object.
        pass
    def addObj(self, name, rad, w0, m):
        cdef orb_obj_t *obj
        obj = orbit_add_obj(self.orb_sys, name, rad, w0, m)
        if obj == NULL:
            raise TypeError("null returned")

cdef class Stars:
    def __cinit__(self, int maxCount):
        self.stars = ooSkyInitStars(maxCount)
    def addStar(self, ra, dec, mag, bv):
        ooSkyAddStar(self.stars, ra, dec, mag, bv)

        

def insertStar(ra, dec, mag, bv):
    ooSkyAddStar(<OOstars*>NULL, ra, dec, mag, bv);


def addOrbitalObject(dist, period, radius, mass, textureName):
    texture.load(textureName, textureName)
    planet_add(dist, 0.0, 0.0, radius, mass, textureName)

#def addOrbitalSys(name, radius, w0):
#    sys = orbit_add_sys(NULL, name, radius, w0)
#    return sys

#def addOrbitalSysWithParent(parent, name, radius, w0):
#    return orbit_add_sys(parent, name, radius, w0)


#def addOrbitalObj(sys, name, radius, w0, m):
#    return orbit_add_obj(sys, name, radius, w0, m)
