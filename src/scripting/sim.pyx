cimport sg, orbits
import sg, orbits


cdef extern from "sim.h":
    void ooSimSetSg(sg.OOnode *sg)
    void ooSimSetCam(sg.OOcam *cam)
    void ooSimSetOrbSys(orbits.OOorbsys *osys)
    
    ctypedef struct SIMstate:
        orbits.OOorbsys *orbSys
        sg.OOnode *sg
        sg.OOcam *cam

def setSg(sg.Node node):
    ooSimSetSg(node.node)
    
def setOrbSys(orbits.OrbitSys sys):
    ooSimSetOrbSys(sys.osys)

def setCam(sg.Cam cam):
    ooSimSetCam(cam.cam)
