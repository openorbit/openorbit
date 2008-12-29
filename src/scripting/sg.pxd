cimport texture
cimport ode

cdef extern from "rendering/scenegraph.h":
   ctypedef void OOobject
   ctypedef void (*OOdrawfunc)(OOobject*)
      
   
   ctypedef struct OOvertex:
      float uv[2]
      float rgba[4]
      float norm[3]
      float vert[3]

   ctypedef struct OOscene

   ctypedef struct OOcam:
      OOscene *scene

   ctypedef struct OOscenegraph:
      OOscene *root

   ctypedef struct OOdrawable:
      OOobject *obj
      OOdrawfunc draw

   void ooSgSceneAttachOdeObj(OOscene *sc, ode.dBodyID body)

   void ooSgSetSky(OOscenegraph *sg, OOdrawable *obj)
   void ooSgSetCam(OOscenegraph *sg, OOcam *cam)

   OOscenegraph* ooSgNewSceneGraph()
   void ooSgSetCam(OOscenegraph *sg, OOcam *cam)

   OOscene* ooSgNewScene(OOscene *parent, char *name)

   void ooSgSceneAddChild(OOscene *parent, OOscene *child)

   
   OOcam* ooSgNewFreeCam(OOscenegraph *sg, OOscene *sc,
                        float x, float y, float z,
                        float rx, float ry, float rz)
   OOcam* ooSgNewFixedCam(OOscenegraph *sg, OOscene *sc, ode.dBodyID body,
                          float dx, float dy, float dz, 
                          float rx, float ry, float rz)

   OOcam* ooSgNewOrbitCam(OOscenegraph *sg, OOscene *sc, ode.dBodyID body,
                          float dx, float dy, float dz)
   


cdef class SkyDrawable:
    cdef OOdrawable *sky
    
cdef class Scenegraph:
    cdef OOscenegraph *sg

cdef class Scene:
    cdef OOscene *sc

cdef class Cam:
   cdef OOcam *cam
