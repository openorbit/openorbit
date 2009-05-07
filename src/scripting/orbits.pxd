cimport ode
cimport sg
import sg

cdef extern from "rendering/scenegraph.h":
   ctypedef void OOobject
   ctypedef void (*OOdrawfunc)(OOobject*)

   ctypedef struct OOscene

   ctypedef struct OOcam:
      OOscene *scene

   ctypedef struct OOscenegraph:
      OOscene *root

   ctypedef struct OOdrawable:
      OOobject *obj
      OOdrawfunc draw


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
        ode.dBodyID id
      
    OOorbsys* ooOrbitNewSys(char *name, float m, float period, float semiMaj, float semiMin)

    OOorbobj* ooOrbitNewObj(OOorbsys *sys, char *name, float m,
                           float x, float y, float z,
                           float vx, float vy, float vz,
                           float qx, float qy, float qz, float qw,
                           float rqx, float rqy, float rqz, float rqw)
    
    void ooOrbitAddChildSys(OOorbsys *sys, OOorbsys *child)
    void ooOrbitStep(OOorbsys *sys, float stepsize)
    void ooOrbitClear(OOorbsys *sys)
    void ooOrbitSetScene(OOorbsys *sys, OOscene *scene)

cdef class OrbitSys:
    cdef OOorbsys *osys
