cimport texture

cdef extern from "ode/ode.h":
    ctypedef struct dxBody
    ctypedef dxBody *dBodyID

cdef extern from "rendering/scenegraph.h":
   ctypedef void OOobject
   ctypedef struct OOnode:
      OOobject *obj
   ctypedef struct OOcam:
      OOnode *attachedNode

   ctypedef struct OOvertex:
      float uv[2]
      float rgba[4]
      float norm[3]
      float vert[3]
        

   void ooSgAddChild(OOnode *parent, OOnode *child)

   OOnode* ooSgNewMesh(texture.OOtexture *tex)

   void ooSgMeshPushVert(OOnode *node, OOvertex *v)

   OOnode* ooSgNewSky()
   OOnode* ooSgNewTransform(float dx, float dy, float dz,
                            float rx, float ry, float rz, float rot)
   OOnode* ooSgNewScale(float scale)
   OOcam* ooSgNewFreeCam(OOnode *node,
                        float x, float y, float z,
                        float rx, float ry, float rz)
   OOcam* ooSgNewFixedCam(OOnode *node, dBodyID body,
                          float dx, float dy, float dz, 
                          float rx, float ry, float rz)

   OOcam* ooSgNewOrbitCam(OOnode *node, dBodyID body,
                          float dx, float dy, float dz)
   

cdef class Node:
   cdef OOnode *node

cdef class Cam:
   cdef OOcam *cam

cdef class OdeBody:
   cdef void *body
