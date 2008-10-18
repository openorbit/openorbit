cdef extern from "rendering/planet.h":
    int planet_add(float x, float y, float z, float radius, float mass, char *tex_key)

cdef extern from "rendering/sky.h":
    ctypedef struct OOstars
    void ooSkyAddStar(OOstars *obj, double ra, double dec, double mag, double bv)
    OOstars* ooSkyInitStars(int starCount)
    OOstars *ooSkyRandomStars()

cdef extern from "physics/orbit.h":
    ctypedef struct orb_sys_t
    ctypedef struct orb_obj_t
    orb_sys_t* orbit_add_sys(orb_sys_t *parent,  char *name, float radius, float w0)
    orb_obj_t* orbit_add_obj(orb_sys_t *sys,  char *name, float radius, float w0, float m)


cdef class OrbitSys:
    cdef orb_sys_t *orb_sys


cdef class Stars:
    cdef OOstars *stars
    