cdef extern from "rendering/scenegraph.h":
   ctypedef void OOobject
   ctypedef struct OOnode:
      OOobject *obj
      
   OOnode* ooSgNewSky()


cdef class Node:
   cdef OOnode *node
