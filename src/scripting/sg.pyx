cimport sg

cdef extern from "rendering/sky.h":
   ctypedef struct OOstars
   void ooSkyAddStar(OOstars *obj, double ra, double dec, double mag, double bv)
   OOstars* ooSkyInitStars(int starCount)
   OOstars *ooSkyRandomStars()

cdef class Node:
    def __cinit__(self):
        pass
    
cdef class SkyNode(Node):
    def __cinit__(self):
        self.node = ooSgNewSky()
    def addStar(self, ra, dec, mag, bv):
        ooSkyAddStar(<OOstars*>self.node.obj, ra, dec, mag, bv)