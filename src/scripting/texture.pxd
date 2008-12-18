cdef extern from "rendering/texture.h":
    ctypedef struct OOtexture:
        unsigned texId
        
    int ooTexLoad(char *key, char *name)
    int ooTexBind(char *key)
    int ooTexNum(char *key)
    int ooTexUnload(char *key)
    OOtexture* ooTexGet(char *key)

cdef class Texture:
    cdef OOtexture *tex
