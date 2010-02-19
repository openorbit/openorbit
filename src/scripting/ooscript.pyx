
cdef extern from "rendering/scenegraph.h":
  ctypedef void OOobject
  ctypedef void (*OOdrawfunc)(OOobject*)

  ctypedef struct OOscene

  ctypedef struct OOcam:
    OOscene *scene

  ctypedef struct OOscenegraph


cdef extern from "rendering/drawable.h":
  ctypedef struct SGdrawable:
    OOobject *obj
    OOdrawfunc draw

  OOscene* ooSgGetRoot(OOscenegraph *sg)

  void ooSgSetSky(OOscenegraph *sg, SGdrawable *obj)
  void ooSgSetCam(OOscenegraph *sg, OOcam *cam)

  OOscenegraph* ooSgNewSceneGraph()
  void ooSgSetCam(OOscenegraph *sg, OOcam *cam)

  OOscene* ooSgNewScene(OOscene *parent, char *name)

  void ooSgSceneAddChild(OOscene *parent, OOscene *child)
  OOscene* ooSgGetScene(OOscenegraph *sg, char *sceneName)


  OOcam* ooSgNewFreeCam(OOscenegraph *sg, OOscene *sc,
                       float x, float y, float z,
                       float rx, float ry, float rz)

  OOcam* ooSgNewOrbitCam(OOscenegraph *sg, OOscene *sc,
                         float dx, float dy, float dz)
  SGdrawable* sgLoadModel(char *file)


cdef extern from "rendering/sky.h":
    ctypedef struct OOstars
    void ooSkyAddStar(OOstars *obj, double ra, double dec, double mag, double bv)
    OOstars* ooSkyInitStars(int starCount)
    OOstars *ooSkyRandomStars()
    SGdrawable *ooSkyNewDrawable(char *fileName)
    OOstars* ooSkyLoadStars(char *fileName)


cdef extern from "physics/orbit.h":
  ctypedef struct PLworld

  PLworld* ooOrbitLoad(OOscenegraph *sg, char *fileName)
  void plGetPosForName3f(PLworld *world, char *name,
                         float *x, float *y, float *z)

cdef extern from "sim/spacecraft.h":
  ctypedef struct OOspacecraft
  OOspacecraft* ooScLoad(PLworld *world, char *fileName)
  void ooScSetPos(OOspacecraft *sc, double x, double y, double z)
  void ooScSetSystemAndPos(OOspacecraft *sc, char *sysName, double x, double y, double z)
  void ooScSetSysAndCoords(OOspacecraft *sc, char *sysName, double longitude, double latitude, double altitude)

cdef extern from "rendering/texture.h":
    ctypedef struct OOtexture:
        unsigned texId

    int ooTexLoad(char *key, char *name)
    int ooTexBind(char *key)
    int ooTexNum(char *key)
    int ooTexUnload(char *key)
    OOtexture* ooTexGet(char *key)

cdef extern from "sim.h":
    void ooSimSetSg(OOscenegraph *sg)
    void ooSimSetCam(OOcam *cam)
    void ooSimSetOrbWorld(PLworld *world)

    cdef struct SIMstate:
      OOscenegraph *sg

cdef extern from "res-manager.h":
    char* ooResGetPath(char *fileName)

cdef extern from "stdlib.h":
    void free(void*)

cdef extern from "parsers/model.h":
  ctypedef struct model_t
  model_t * model_load(char * fileName)

cdef extern from "plugin-handler.h":
    int ooPluginLoadAll()
    char *ooPluginLoad(char *filename)
    void ooPluginUnload(char *key)


cdef extern from "settings.h":
  int ooConfSetBoolAsInt(char *key, int val)
  int ooConfGetBoolAsInt(char *key, int *val)    
  int ooConfSetInt(char *key, int val)
  int ooConfGetInt(char *key, int *val)
  int ooConfSetFloat(char *key, float val)
  int ooConfGetFloat(char *key, float *val)
  int ooConfSetStr(char *key, char *val)
  char* ooConfGetStr(char *key)


#def LoadAllPlugins():
#   ooPluginLoadAll()

#def LoadPlugin(name):
#    return ooPluginLoad(name)

#def UnloadPlugin(key):
#    ooPluginUnload(key)

cdef class Model:
  cdef model_t * model
  def __init__(self, char *key):
    cdef char *path
    path = ooResGetPath(key)
    self.model = model_load(path)
    free(path)

cdef class Texture:
    cdef OOtexture *tex
    def __cinit__(self, key):
        self.tex = ooTexGet(key)

cdef class SkyDrawable:
    cdef SGdrawable *sky
    def __cinit__(self, char *fileName):
      self.sky = ooSkyNewDrawable(fileName)

def loadTexture(char *key, char *fileName):
    status = ooTexLoad(key, fileName)
    if status != 0:
      print "Texture not found"

def unloadTexture(char *key):
    ooTexUnload(key)

cdef class Scene:
   cdef OOscene *sc

cdef class Cam:
  cdef OOcam *cam
  def __cinit__(self):
    self.cam = <OOcam*>0

cdef class Scenegraph:
  cdef OOscenegraph *sg
  def __cinit__(self):
      self.sg = <OOscenegraph*>0
  
  def new(self):
      self.sg = ooSgNewSceneGraph()
      return self

  def setCam(self, Cam cam):
      ooSgSetCam(self.sg, cam.cam)
      
  def getRoot(self):
      cdef Scene sc
      sc = Scene()
      sc.sc = ooSgGetRoot(self.sg)
      return sc
  
  def getScene(self, key):
      cdef Scene sc
      sc.sc = ooSgGetScene(self.sg, key)
      return sc
  
  def setBackground(self, SkyDrawable sky):
      ooSgSetSky(self.sg, sky.sky)
  
  def setOverlay(self):
      pass

cdef class FreeCam(Cam):
  def __cinit__(self):#, node, x, y, z, rx, ry, rz):
    pass
  def setParams(self, Scenegraph sg, Scene sc, x, y, z, rx, ry, rz):
    self.cam = ooSgNewFreeCam(sg.sg, sc.sc, x, y, z, rx, ry, rz)

cdef class OrbitCam(Cam):
  def __cinit__(self, Scenegraph sg, Scene sc, dx, dy, dz):
    self.cam = ooSgNewOrbitCam(sg.sg, sc.sc, dx, dy, dz)

cdef class OrbitWorld:
  cdef PLworld *world
  def __cinit__(self):
    self.world = <PLworld*>0
  def __dealloc__(self):
    # C function call to delete obj_sys object.
    pass
  def new(self, Scenegraph scg, char *fileName):
    self.world = ooOrbitLoad(scg.sg, fileName)
    return self
  def getPosForObjName(self, char *name):
    cdef float x, y, z
    plGetPosForName3f(self.world, name, &x, &y, &z)
    return (x, y, z)


cdef class Spacecraft:
  cdef OOspacecraft *sc
  def __init__(self, OrbitWorld world, path):
    self.sc = ooScLoad(world.world, path)
  def setPos(self, double x, double y, double z):
    ooScSetPos(self.sc, x, y, z)
  def setSysAndPos(self, char *sysPath, double x, double y, double z):
    ooScSetSystemAndPos(self.sc, sysPath, x, y, z)
  def setSysAndEqCoords(self, char *sysName, double longitude, double latitude, double altitude):
    ooScSetSysAndCoords(self.sc, sysName, longitude, latitude, altitude)



def setSg(Scenegraph scg):
    ooSimSetSg(scg.sg)

def setOrbWorld(OrbitWorld world):
  ooSimSetOrbWorld(world.world)


def getResPath(str):
    cdef char *path
    path = ooResGetPath(str)
    pstr = path
    free(path)
    return pstr
