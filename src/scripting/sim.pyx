cimport sg, environment
import sg, environment


cdef extern from "sim.h":
    void ooSimSetSg(sg.OOnode *sg)
    void ooSimSetOrbSys(environment.orb_sys_t *osys)
    
    ctypedef struct SIMstate:
        environment.orb_sys_t *orbSys
        sg.OOnode *sg

def setSg(sg.Node node):
    ooSimSetSg(node.node)
    
def setOrbSys(environment.OrbitSys sys):
    ooSimSetOrbSys(sys.orb_sys)
