cdef extern from "rendering/planet.h":
    int planet_add(float x, float y, float z, float radius, float mass, char *tex_key)

cdef extern from "rendering/sky.h":
    ctypedef struct OOstars
    void ooSkyAddStar(OOstars *obj, double ra, double dec, double mag, double bv)
    OOstars* ooSkyInitStars(int starCount)
    OOstars *ooSkyRandomStars()

cdef extern from "physics/orbit.h":
    ctypedef struct OOorbsys:
        char *name
    ctypedef struct OOorbobj:
        char *name
    
    OOorbsys* ooOrbitNewSys(char *name, float radius, float w0)
    OOorbobj* ooOrbitAddObj(OOorbsys *sys, char *name, float radius, float w0, float m)
    void ooOrbitAddChildSys(OOorbsys *sys, OOorbsys *child)
    void ooOrbitStep(OOorbsys *sys, float stepsize)
    void ooOrbitClear(OOorbsys *sys)
    void ooOrbitSetRoot(OOorbsys *sys)

cdef class OrbitSys:
    cdef OOorbsys *osys
    
