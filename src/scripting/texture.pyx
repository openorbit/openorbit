cimport texture

def load(char *key, char *fileName):
    status = ooTexLoad(key, fileName)
    if status != 0:
        print "Texture not found"

def unload(char *key):
    ooTexUnload(key)
    
cdef class Texture:
    def __cinit__(self, key):
        self.tex = ooTexGet(key)
