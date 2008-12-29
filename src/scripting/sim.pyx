cimport sg, orbits
import sg, orbits


cdef extern from "sim.h":
    void ooSimSetSg(sg.OOscenegraph *sg)
    void ooSimSetCam(sg.OOcam *cam)
    void ooSimSetOrbSys(orbits.OOorbsys *osys)
    
    ctypedef struct SIMstate:
        orbits.OOorbsys *orbSys
        sg.OOcam *cam
        sg.OOscenegraph *sg

def setSg(sg.Scenegraph sg):
    ooSimSetSg(sg.sg)
    
def setOrbSys(orbits.OrbitSys sys):
    ooSimSetOrbSys(sys.osys)

def setCam(sg.Cam cam):
    ooSimSetCam(cam.cam)
