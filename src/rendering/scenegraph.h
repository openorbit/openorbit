/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2008 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)openorbit.org>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
 */
#ifndef SCENEGRAPH_H_
#define SCENEGRAPH_H_

#include <ode/ode.h>

#include <vmath/vmath.h>

#include <openorbit/openorbit.h>
#include "texture.h"
//#include "physics/orbit.h"
#include "sim/simtime.h"

  typedef struct OOdrawable OOdrawable;
  typedef struct OOscene OOscene;
  typedef struct OOoverlay OOoverlay;
  typedef struct OOscenegraph OOscenegraph;

#include "camera.h"
  
//typedef void OOobject;
typedef void (*OOdrawfunc)(OOobject*);


struct OOdrawable {
  vector_t p;
  quaternion_t q;
  float s;
  
  OOobject *obj;
  OOdrawfunc draw;
};

struct OOscene {
    struct OOscene *parent;
    char *name;

    OOobject *data;
    OOdrawfunc preStepUpdate;
        
    vector_t t;
    quaternion_t q;
    scalar_t s; // scale with respect to parent s
    scalar_t si; // inverse of s
    
    OOobjvector scenes;
    OOobjvector objs;
};

struct OOoverlay {
  OOtexture *tex;
  float x, y;
  float w, h;
};

struct OOscenegraph {
  OOscene *root;
  OOcam *currentCam;
  OOobjvector cams;

  OOdrawable *sky;
  OOobjvector overlays;
};

OOdrawable* ooSgNewDrawable(OOobject *obj, OOdrawfunc df);

void ooSgSetScenePos(OOscene *sc, float x, float y, float z);
void ooSgSetSceneQuat(OOscene *sc, float x, float y, float z, float w);
void ooSgSetSceneScale(OOscene *sc, float scale);

void ooSgDrawOverlay(OOoverlay *overlay);

OOscenegraph* ooSgNewSceneGraph();
void ooSgPaint(OOscenegraph *sg);

/*!
 * Sets the current camera in the scenegraph to cam. The camera should be in the
 * list of known cameras kept by the scenegraph, but this property is not
 * checked unless -DDEBUG is specifyed when building.
 * \param sg The scenegraph in question
 * \param cam A camera pointer to a camera known by the scenegraph
 * \pre sg != NULL
 * \pre cam != NULL
 */
void ooSgSetCam(OOscenegraph *sg, OOcam *cam);

/*!
 * Create a new scene. If parent is NULL, the scene is inserted in the parent's
 * scene vector. In case of null a root scene is created. Note that
 * ooSgNewSceneGraph creates a root scene by default so you should normally pass
 * in a valid pointer to it.
 */
OOscene* ooSgNewScene(OOscene *parent, const char *name);
void ooSgSetSky(OOscenegraph *sg, OOdrawable *obj);

/*!
  Finds the root scene from the a given scene. Note that in most cases you want
  to use the OOscenegraph member root for getting the scene, since member
  access is O(1) and locating the root for a given scene is O(treeHeight).
  \param sc Scene from which to find the root scene 
  \result The root of sc
  \pre sc != NULL
*/
OOscene* ooSgSceneGetRoot(OOscene *sc);
void ooSgSceneAddChild(OOscene *parent, OOscene *child);
void ooSgSceneAddObj(OOscene *sc, OOobject *object);


typedef struct {
    matrix_t t;
    quaternion_t q;
} OOtransform;

typedef struct {
    scalar_t s;
} OOscale;

typedef struct {
    dWorldID world;
    dBodyID body;
} OOodetransform;

typedef struct {
    float uv[2];
    float rgba[4];
    float norm[3];
    float vert[3];
} OOvertex;


typedef struct {
    size_t vSize;
    size_t vCount;
    OOvertex *vertices;
    GLuint texId;
} OOmesh;

typedef struct {
    GLuint texId;
    GLUquadricObj *quadratic;
    GLfloat radius;
} OOsphere;

OOdrawable* ooSgNewSphere(float radius, const char *tex);

/* Allocators */
//OOnode* ooSgNewMesh(OOtexture *tex);
//OOnode* ooSgNewTransform(float dx, float dy, float dz,
//                         float rx, float ry, float rz, float rot);
//OOnode* ooSgNewOdeTransform(dWorldID world, dBodyID body);
//OOnode* ooSgNewSphere(OOtexture *tex);
//OOnode* ooSgNewSky(void);
//OOnode* ooSgNewScale(float scale);

//void ooSgMeshPushVert(OOnode *node, const OOvertex *v);

/* Draw functions */
//void ooSgDrawMesh(OOmesh *mesh);
//void ooSgTransform(OOtransform *t);
//void ooSgPostTransform(OOtransform *t);
//void ooSgOdeTransform(OOodetransform *t);
//void ooSgDrawSphere(OOsphere *sphere);

//void ooSgScale(OOscale *scale);
//void ooSgPostScale(OOscale *scale);

#endif /* SCENEGRAPH_H_ */
