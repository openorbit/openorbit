ctypedef int _Bool
    
cdef extern from "rendering/texture.h":
    int ooTexLoad(char *key, char *name)
    int ooTexBind(char *key)
    int ooTexNum(char *key)
    int ooTexUnload(char *key)


def load(char *key, char *fileName):
    status = ooTexLoad(key, fileName)
    if status != 0:
        print "Texture not found"

def unload(char *key):
    ooTexUnload(key)
