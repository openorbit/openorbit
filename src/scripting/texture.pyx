ctypedef int _Bool
    
cdef extern from "rendering/texture.h":
    int tex_load(char *key, char *name)
    int tex_bind(char *key)
    int tex_num(char *key)
    int tex_unload(char *key)


def load(char *key, char *fileName):
    status = tex_load(key, fileName)
    if status != 0:
        print "Texture not found"

def unload(char *key):
    tex_unload(key)
