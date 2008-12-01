cimport sg

cdef extern from "rendering/sky.h":
   ctypedef struct OOstars
   void ooSkyAddStar(OOstars *obj, double ra, double dec, double mag, double bv)
   OOstars* ooSkyInitStars(int starCount)
   OOstars *ooSkyRandomStars()

cdef class Node:
    def __cinit__(self):
        self.node = <OOnode*>0
    
    def addChild(Node n):
        ooSgAddChild(<OOnode*>self.node, n.node)
    
cdef class SkyNode(Node):
    def __cinit__(self):
        self.node = ooSgNewSky()
    def addStar(self, ra, dec, mag, bv):
        ooSkyAddStar(<OOstars*>self.node.obj, ra, dec, mag, bv)
        
cdef class ScaleNode(Node):
    def __cinit__(self, s):
        self.node = ooSgNewScale(s)


cdef class TransformNode(Node):
    def __cinit__(self, dx, dy, dz, rx, ry, rz, rot):
        self.node = ooSgNewTransform(dx, dy, dz, rx, ry, rz, rot)
