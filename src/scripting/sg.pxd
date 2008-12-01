cdef extern from "rendering/scenegraph.h":
   ctypedef void OOobject
   ctypedef struct OOnode:
      OOobject *obj

   void ooSgAddChild(OOnode *parent, OOnode *child)
      
   OOnode* ooSgNewSky()
   OOnode* ooSgNewTransform(float dx, float dy, float dz,
                            float rx, float ry, float rz, float rot)
   OOnode* ooSgNewScale(float scale)
   

cdef class Node:
   cdef OOnode *node
