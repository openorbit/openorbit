cdef extern from "ode/ode.h":
    ctypedef struct dxBody
    ctypedef dxBody *dBodyID

cdef class OdeBody:
    cdef dBodyID body
