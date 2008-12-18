cimport sg, texture

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
        pass
        #super.node = ooSgNewSky()
    def __init__(self):
        self.node = ooSgNewSky()

    def addStar(self, ra, dec, mag, bv):
        ooSkyAddStar(<OOstars*>self.node.obj, ra, dec, mag, bv)
        
cdef class ScaleNode(Node):
    def __cinit__(self, s):
        self.node = ooSgNewScale(s)


cdef class TransformNode(Node):
    def __cinit__(self, dx, dy, dz, rx, ry, rz, rot):
        self.node = ooSgNewTransform(dx, dy, dz, rx, ry, rz, rot)

cdef class Cam:
    def __cinit__(self):
        self.cam = <OOcam*>0

cdef class FreeCam(Cam):
    def __cinit__(self):#, node, x, y, z, rx, ry, rz):
        pass
        #
    def __init__(self):#, node, x, y, z, rx, ry, rz):
        super(Cam, self)
        
    def setParams(self, sg.Node node, x, y, z, rx, ry, rz):
        self.cam = ooSgNewFreeCam(node.node, x, y, z, rx, ry, rz)

cdef class FixedCam(Cam):
    def __cinit__(self, sg.Node node, body, dx, dy, dz, rx, ry, rz):
        self.cam = ooSgNewFixedCam(<OOnode*>node.node, <dBodyID>body, dx, dy, dz, rx, ry, rz)
    def __init__(self, node, body, dx, dy, dz, rx, ry, rz):
        pass

cdef class OrbitCam(Cam):
    def __cinit__(self, node, body, dx, dy, dz):
        self.cam = ooSgNewOrbitCam(<OOnode*>node.node, <dBodyID>body, dx, dy, dz)

cdef class Mesh(Node):
    def __cinit__(self, tex):
        self.node = ooSgNewMesh(<texture.OOtexture*>tex.tex)
    
    def addVertex(uv, rgba, norm, vert):
        cdef OOvertex vtx
        u, v = uv
        vtx.uv[0] = u
        vtx.uv[1] = v
        r,g,b,a = rgba
        vtx.rgba[0] = r
        vtx.rgba[1] = g
        vtx.rgba[2] = b
        vtx.rgba[3] = a
        n0,n1,n2 = norm
        vtx.norm[0] = n0
        vtx.norm[1] = n1
        vtx.norm[2] = n2
        v0,v1,v2 = vert
        vtx.vert[0] = v0
        vtx.norm[1] = v1
        vtx.norm[2] = v2
        ooSgMeshPushVert(<OOnode*>self.node, &vtx)
        
    