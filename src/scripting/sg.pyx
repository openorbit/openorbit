cimport sg, texture
cimport ode

cdef extern from "rendering/sky.h":
   ctypedef struct OOstars
   void ooSkyAddStar(OOstars *obj, double ra, double dec, double mag, double bv)
   OOstars* ooSkyInitStars(int starCount)
   OOstars *ooSkyRandomStars()
   OOdrawable *ooSkyNewDrawable()
   
cdef class Scene:
    def __cinit__(self):
        self.sc = <OOscene*>0
    
    def init(self, name):
        self.sc = ooSgNewScene(NULL, name)
        return self
        
    def setParent(self, Scene parent):
        ooSgSceneAddChild(parent.sc, self.sc)
    
    def connectToOdeObj(self, ode.OdeBody body):
        ooSgSceneAttachOdeObj(self.sc, body.body);

    
    def addChild(self, Scene sc):
        ooSgSceneAddChild(self.sc, sc.sc)
        
    def addObject(self):
        pass
    def setTranslation(self, float x, float y, float z):
        pass
    def setQuaternion(self, float x, float y, float z, float w):
        pass
    def setScale(self, float x):
        pass

cdef class Scenegraph:
    def __cinit__(self):
        self.sg = <OOscenegraph*>0
    
    def new(self):
        self.sg = ooSgNewSceneGraph()
        return self
    
    def setCam(self, Cam cam):
        ooSgSetCam(self.sg, cam.cam)
        
    def getRoot(self):
        cdef Scene sc
        sc = Scene()
        sc.sc = self.sg.root
        return sc
    
    def getScene(self, key):
        pass
    
    def setBackground(self, SkyDrawable sky):
        ooSgSetSky(self.sg, sky.sky)
    
    def setOverlay(self):
        pass

cdef class SkyDrawable:
    def __cinit__(self):
        self.sky = ooSkyNewDrawable()
    def addStar(self, ra, dec, mag, bv):
        ooSkyAddStar(<OOstars*>self.sky.obj, ra, dec, mag, bv)


cdef class Cam:
    def __cinit__(self):
        self.cam = <OOcam*>0

cdef class FreeCam(Cam):
    def __cinit__(self):#, node, x, y, z, rx, ry, rz):
        pass
    def setParams(self, Scenegraph sg, sg.Scene sc, x, y, z, rx, ry, rz):
        self.cam = ooSgNewFreeCam(sg.sg, sc.sc, x, y, z, rx, ry, rz)

cdef class FixedCam(Cam):
    def __cinit__(self, Scenegraph sg, sg.Scene sc, ode.OdeBody body, dx, dy, dz, rx, ry, rz):
        self.cam = ooSgNewFixedCam(sg.sg, sc.sc, body.body, dx, dy, dz, rx, ry, rz)

cdef class OrbitCam(Cam):
    def __cinit__(self, Scenegraph sg, sg.Scene sc, ode.OdeBody body, dx, dy, dz):
        self.cam = ooSgNewOrbitCam(sg.sg, sc.sc, body.body, dx, dy, dz)

#cdef class Mesh(Node):
#    def __cinit__(self, tex):
#        self.node = ooSgNewMesh(<texture.OOtexture*>tex.tex)
#    
#    def addVertex(uv, rgba, norm, vert):
#        cdef OOvertex vtx
#        u, v = uv
#        vtx.uv[0] = u
#        vtx.uv[1] = v
#        r,g,b,a = rgba
#        vtx.rgba[0] = r
#        vtx.rgba[1] = g
#        vtx.rgba[2] = b
#        vtx.rgba[3] = a
#        n0,n1,n2 = norm
#        vtx.norm[0] = n0
#        vtx.norm[1] = n1
#        vtx.norm[2] = n2
#        v0,v1,v2 = vert
#        vtx.vert[0] = v0
#        vtx.norm[1] = v1
#        vtx.norm[2] = v2
#        ooSgMeshPushVert(<OOnode*>self.node, &vtx)
