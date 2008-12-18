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
#include "texture.h"

typedef void OOobject;
typedef void (*OOdrawfunc)(OOobject*);

typedef struct OOnode_ {
    OOdrawfunc draw;
    OOdrawfunc postDraw;
    OOobject *obj;
    struct OOnode_ *next;
    struct OOnode_ *children;
} OOnode;

typedef enum {
    OOCam_Free,
    OOCam_Fixed,
    OOCam_Orbit
} OOcamtype;

typedef struct {
    vector_t p;
    quaternion_t q;
} OOfreecam;

typedef struct {
    dBodyID body;
    
    vector_t r;
    quaternion_t q;   
} OOfixedcam;

typedef struct {
    dBodyID body;
    
    vector_t r;
} OOorbitcam;

typedef struct {
    OOcamtype kind;
    OOnode *attachedNode;
    void *camData;
} OOcam;

OOnode* ooSgNewNode(OOobject *obj, OOdrawfunc df, OOdrawfunc postDf);
void ooSgAddChild(OOnode *parent, OOnode *child);
void ooSgDraw(OOnode *node, OOcam *cam);

OOcam* ooSgNewFreeCam(OOnode *node,
                      float x, float y, float z, 
                      float rx, float ry, float rz);
                      
OOcam* ooSgNewFixedCam(OOnode *node, dBodyID body,
                       float dx, float dy, float dz, 
                       float rx, float ry, float rz);

OOcam* ooSgNewOrbitCam(OOnode *node, dBodyID body,
                       float dx, float dy, float dz);

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

/* Allocators */
OOnode* ooSgNewMesh(OOtexture *tex);
OOnode* ooSgNewTransform(float dx, float dy, float dz,
                         float rx, float ry, float rz, float rot);
OOnode* ooSgNewOdeTransform(dWorldID world, dBodyID body);
OOnode* ooSgNewSphere(OOtexture *tex);
OOnode* ooSgNewSky(void);
OOnode* ooSgNewScale(float scale);

void ooSgMeshPushVert(OOnode *node, const OOvertex *v);

/* Draw functions */
void ooSgDrawMesh(OOmesh *mesh);
void ooSgTransform(OOtransform *t);
void ooSgPostTransform(OOtransform *t);
void ooSgOdeTransform(OOodetransform *t);
void ooSgDrawSphere(OOsphere *sphere);

void ooSgScale(OOscale *scale);
void ooSgPostScale(OOscale *scale);

#endif /* SCENEGRAPH_H_ */
