
cdef extern from "rendering/scenegraph.h":
  ctypedef void OOobject
  ctypedef void (*SGdrawfunc)(OOobject*)

  ctypedef struct SGscene

  ctypedef struct SGcam:
    SGscene *scene

  ctypedef struct SGscenegraph

cdef extern from "physics/object.h":
  ctypedef struct PLobject

cdef extern from "rendering/drawable.h":
  ctypedef struct SGdrawable:
    OOobject *obj
    SGdrawfunc draw


  void sgSetSky(SGscenegraph *sg, SGdrawable *obj)
  void sgSetCam(SGscenegraph *sg, SGcam *cam)

  SGscenegraph* sgNewSceneGraph()
  void sgSetCam(SGscenegraph *sg, SGcam *cam)

  SGscene* sgNewScene(SGscene *parent, char *name)

  void sgSceneAddChild(SGscene *parent, SGscene *child)
  SGscene* sgGetScene(SGscenegraph *sg, char *sceneName)


  SGcam* sgNewFreeCam(SGscenegraph *sg, SGscene *sc,
                       float x, float y, float z,
                       float rx, float ry, float rz)

  SGcam* sgNewOrbitCam(SGscenegraph *sg, SGscene *sc, PLobject *body,
                         float ra, float dec, float dz)
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
  ctypedef struct PLsystem
  ctypedef struct PLastrobody

  void plGetPosForName3f(PLworld *world, char *name,
                         float *x, float *y, float *z)
  PLsystem* plGetSystem(PLworld *world, char *name)
  PLastrobody* plGetObject(PLworld *world, char *name)
  PLobject* plObjForAstroBody(PLastrobody *abody)

cdef extern from "sim/world-loader.h":
  PLworld* ooOrbitLoad(SGscenegraph *sg, char *fileName)


cdef extern from "sim/spacecraft.h":
  ctypedef struct OOspacecraft
  OOspacecraft* ooScLoad(PLworld *world, SGscene *scene, char *fileName)
  void ooScSetPos(OOspacecraft *sc, double x, double y, double z)
  void ooScSetSystemAndPos(OOspacecraft *sc, char *sysName, double x, double y, double z)
  void ooScSetSysAndCoords(OOspacecraft *sc, char *sysName, double longitude, double latitude, double altitude)
  PLobject* ooScGetPLObjForSc(OOspacecraft *sc)
  OOspacecraft* simNewSpacecraft(char *className, char *scName)


cdef extern from "rendering/texture.h":
    ctypedef struct OOtexture:
        unsigned texId

    int ooTexLoad(char *key, char *name)
    int ooTexBind(char *key)
    int ooTexNum(char *key)
    int ooTexUnload(char *key)
    OOtexture* ooTexGet(char *key)

cdef extern from "sim.h":
    void ooSimSetSg(SGscenegraph *sg)
    void ooSimSetCam(SGcam *cam)
    void ooSimSetOrbWorld(PLworld *world)
    void simSetSpacecraft(OOspacecraft *sc)
    cdef struct SIMstate:
      SGscenegraph *sg

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
   cdef SGscene *sc

cdef class Cam:
  cdef SGcam *cam
  def __cinit__(self):
    self.cam = <SGcam*>0

cdef class Scenegraph:
  cdef SGscenegraph *sg
  def __cinit__(self):
      self.sg = <SGscenegraph*>0

  def new(self):
      self.sg = sgNewSceneGraph()
      return self

  def setCam(self, Cam cam):
      sgSetCam(self.sg, cam.cam)

  def getScene(self, key):
      cdef Scene sc
      sc.sc = sgGetScene(self.sg, key)
      return sc

  def setBackground(self, SkyDrawable sky):
      sgSetSky(self.sg, sky.sky)

  def setOverlay(self):
      pass

cdef class PLObject:
  cdef PLobject *obj
  def __cinit__(self):
    self.obj = <PLobject*>0
  cdef setObj(self, PLobject *obj):
    self.obj = obj

cdef class FreeCam(Cam):
  def __cinit__(self):#, node, x, y, z, rx, ry, rz):
    pass
  def setParams(self, Scenegraph sg, Scene sc, x, y, z, rx, ry, rz):
    self.cam = sgNewFreeCam(sg.sg, sc.sc, x, y, z, rx, ry, rz)

cdef class OrbitCam(Cam):
  def __cinit__(self):
    pass
  def setParams(self, Scenegraph sg, Scene sc, PLObject plo, float dec, float ra, float r):
    self.cam = sgNewOrbitCam(sg.sg, sc.sc, plo.obj, dec, ra, r)

cdef class OrbitWorld:
  cdef PLworld *world
  def __cinit__(self):
    self.world = <PLworld*>0
  def __dealloc__(self):
    # C function call to delete obj_sys object.
    pass
  def new(self, Scenegraph scg, char *fileName):
    self.world = ooOrbitLoad(scg.sg, fileName)
    if self.world == <PLworld*>0:
      return None
    return self
  def getPosForObjName(self, char *name):
    cdef float x, y, z
    plGetPosForName3f(self.world, name, &x, &y, &z)
    return (x, y, z)
  def getPLObjForName(self, char *name):
    cdef PLastrobody *abody
    abody = plGetObject(self.world, name)
    cdef PLObject plo
    plo = PLObject()
    plo.obj = plObjForAstroBody(abody)
    return plo


cdef class Spacecraft:
  cdef OOspacecraft *sc
  def __init__(self, OrbitWorld world, Scene scene, scClass, scName):
    self.sc = simNewSpacecraft(scClass, scName)
    #ooScLoad(world.world, scene.sc, path)
  def setPos(self, double x, double y, double z):
    ooScSetPos(self.sc, x, y, z)
  def setSysAndPos(self, char *sysPath, double x, double y, double z):
    ooScSetSystemAndPos(self.sc, sysPath, x, y, z)
  def setSysAndEqCoords(self, char *sysName, double longitude, double latitude, double altitude):
    ooScSetSysAndCoords(self.sc, sysName, longitude, latitude, altitude)

  def getPLObject(self):
    cdef PLObject plo
    plo = PLObject()
    plo.obj = ooScGetPLObjForSc(self.sc)
    return plo


def setSg(Scenegraph scg):
  ooSimSetSg(scg.sg)

def setOrbWorld(OrbitWorld world):
  ooSimSetOrbWorld(world.world)

def setCurrentSc(Spacecraft sc):
  simSetSpacecraft(sc.sc)

def getResPath(str):
    cdef char *path
    path = ooResGetPath(str)
    pstr = path
    free(path)
    return pstr
