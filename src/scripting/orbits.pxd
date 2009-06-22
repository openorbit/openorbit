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

    OOorbsys* ooOrbitLoad(OOscenegraph *scg, char *file)


cdef class OrbitSys:
    cdef OOorbsys *osys
