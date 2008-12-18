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

#include <stdlib.h>
#include <assert.h>
//#include <OpenGL/gl.h>
#include "SDL_opengl.h"
#include "scenegraph.h"
#include "sky.h"
#include "log.h"
#include <vmath/vmath.h>

OOcam*
ooSgNewFreeCam(OOnode *node,
               float x, float y, float z, float rx, float ry, float rz)
{
    OOcam *cam = malloc(sizeof(OOcam));
    cam->camData = malloc(sizeof(OOfreecam));
    cam->kind = OOCam_Free;
    
    cam->attachedNode = node;
    ((OOfreecam*)cam->camData)->p = v_set(x,y,z,1.0f);
    ((OOfreecam*)cam->camData)->q = q_rot(rx,ry,rz, 0.0f);
    return cam;
}

OOcam*
ooSgNewFixedCam(OOnode *node, dBodyID body,
                float dx, float dy, float dz, float rx, float ry, float rz)
{
    OOcam *cam = malloc(sizeof(OOcam));
    cam->camData = malloc(sizeof(OOfixedcam));
    cam->kind = OOCam_Fixed;

    cam->attachedNode = node;
    ((OOfixedcam*)cam->camData)->body = body;
    ((OOfixedcam*)cam->camData)->r = v_set(dx,dy,dz,1.0f);
    ((OOfixedcam*)cam->camData)->q = q_rot(rx,ry,rz, 0.0f);

    return cam;    
}

OOcam*
ooSgNewOrbitCam(OOnode *node, dBodyID body, float dx, float dy, float dz)
{
    OOcam *cam = malloc(sizeof(OOcam));
    cam->camData = malloc(sizeof(OOorbitcam));
    cam->kind = OOCam_Orbit;

    cam->attachedNode = node;
    ((OOorbitcam*)cam->camData)->body = body;
    ((OOorbitcam*)cam->camData)->r = v_set(dx,dy,dz,1.0f);

    return cam;
}

void
ooSgCamMove(OOcam *cam)
{
    assert(cam != NULL && "cam not set");
    glPushMatrix();
    
    switch (cam->kind) {
    case OOCam_Orbit:
        {
            const dReal *pos = dBodyGetPosition(((OOorbitcam*)cam->camData)->body);
            const dReal *rot = dBodyGetRotation(((OOorbitcam*)cam->camData)->body);
            gluLookAt(  ((OOorbitcam*)cam->camData)->r.x + pos[0],
         	            ((OOorbitcam*)cam->camData)->r.y + pos[1],
         	            ((OOorbitcam*)cam->camData)->r.x + pos[2],
         	            pos[0], pos[1], pos[2], // center
                        0.0, 1.0, 0.0); // up
        }
        break;
    case OOCam_Fixed:
        {
            const dReal *pos = dBodyGetPosition(((OOfixedcam*)cam->camData)->body);
            const dReal *rot = dBodyGetRotation(((OOfixedcam*)cam->camData)->body);
            glTranslatef(((OOfixedcam*)cam->camData)->r.x + pos[0],
                         ((OOfixedcam*)cam->camData)->r.y + pos[1],
                         ((OOfixedcam*)cam->camData)->r.z + pos[2]);
            
        }
        break;
    case OOCam_Free:
        glTranslatef(((OOfreecam*)cam->camData)->p.x,
                     ((OOfreecam*)cam->camData)->p.y,
                     ((OOfreecam*)cam->camData)->p.z);
        break;
    default:
        assert(0 && "illegal case statement");
    }
}

void
ooSgCamPopMove(OOcam *cam)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void
ooSgDrawFuncGnd(OOobject*obj)
{
    
}


OOnode*
ooSgNewNode(OOobject *obj, OOdrawfunc df, OOdrawfunc postDf)
{
    OOnode *node = malloc(sizeof(OOnode));
    if (node != NULL) {
        node->obj = obj;
        node->draw = (df == NULL) ? ooSgDrawFuncGnd : df;
        node->postDraw = (postDf == NULL) ? ooSgDrawFuncGnd : postDf;
        node->next = NULL;
        node->children = NULL;
    }
    return node;
}

void
ooSgAddChild(OOnode *parent, OOnode *child)
{
    if (parent->children == NULL) {
        parent->children = child;
    } else {
        OOnode *node = parent->children;
        while (node->next) {
            node = node->next;
        }
        node->next = child;
    }    
}

static void
ooSgDraw2(OOnode *node, OOcam *cam)
{
    assert(node != NULL && "node null");
    assert(cam != NULL && "cam null");
    
    while (node) {
        // Check if camera transforms should be done
        if (node == cam->attachedNode) {
            ooSgCamMove(cam);
        }
        
        node->draw(node->obj);
        if (node->children) ooSgDraw2(node->children, cam);
        node->postDraw(node->obj);

        if (node == cam->attachedNode) {
            ooSgCamPopMove(cam);
        }

        node = node->next;
    }
}

void
ooSgDraw(OOnode *node, OOcam *cam)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
    ooSgDraw2(node, cam);
}

void
ooSgDrawMesh(OOmesh *mesh)
{
    assert(mesh != NULL);
    glBindTexture(GL_TEXTURE_2D, mesh->texId);
    glInterleavedArrays(GL_T2F_C4F_N3F_V3F, 0, mesh->vertices);
    glDrawArrays(GL_TRIANGLES, 0, mesh->vCount);
}


void
ooSgTransform(OOtransform *t)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

void
ooSgPostTransform(OOtransform *t)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void
ooSgOdeTransform(OOodetransform *t)
{
    const dReal *pos = dBodyGetPosition(t->body);
    const dReal *rot = dBodyGetRotation(t->body);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glTranslatef(pos[0], pos[1], pos[2]);
}





OOnode*
ooSgNewMesh(OOtexture *tex)
{
    OOmesh *mesh = malloc(sizeof(OOmesh));
    if (mesh == NULL) {return NULL;}
    
    OOnode *node = ooSgNewNode(mesh, (OOdrawfunc)ooSgDrawMesh, NULL);
    mesh->vCount = 0;
    mesh->texId = tex->texId;
    
    mesh->vertices = calloc(32, sizeof(OOvertex));
    mesh->vSize = 32;
    
    return node;
}

void
ooSgMeshPushVert(OOnode *node, const OOvertex *v)
{
    OOmesh *mesh = node->obj;
    
    if (mesh->vCount = mesh->vSize) {
        OOvertex *newVerts = realloc(mesh->vertices,
                                     mesh->vSize * 2 * sizeof(OOvertex));
        
        if (!newVerts) ooLogFatal("vertex buffer expansion failed");
        mesh->vSize = mesh->vSize * 2;
        mesh->vertices = newVerts;
    }
    
    mesh->vertices[mesh->vCount] = *v;
    mesh->vCount ++;
}



OOnode*
ooSgNewTransform(float dx, float dy, float dz, float rx, float ry, float rz, float rot)
{
    OOtransform *transf = malloc(sizeof(OOtransform));
    if (transf == NULL) {return NULL;}
    OOnode *node = ooSgNewNode(transf,
                               (OOdrawfunc)ooSgTransform,
                               (OOdrawfunc)ooSgPostTransform);
        
    return node;
}

OOnode*
ooSgNewOdeTransform(dWorldID world, dBodyID body)
{
    OOodetransform *transf = malloc(sizeof(OOodetransform));
    if (transf == NULL) {return NULL;}
    OOnode *node = ooSgNewNode(transf,
                               (OOdrawfunc)ooSgOdeTransform,
                               (OOdrawfunc)ooSgPostTransform);
    
    transf->world = world;
    transf->body = body;
    
    return node;
}

OOnode*
ooSgNewSphere(OOtexture *tex)
{
    OOsphere *sphere = malloc(sizeof(OOsphere));
    if (sphere == NULL) {return NULL;}
    
    OOnode *node = ooSgNewNode(sphere, (OOdrawfunc)ooSgDrawSphere, NULL);
    sphere->texId = tex->texId;
    
    return node;
}

void
ooSgDrawSphere(OOsphere *sphere)
{
//    matrix_t m;
//    glPushMatrix();
    
    glBindTexture(GL_TEXTURE_2D, sphere->texId);
        
//    matrix_t rot_orig;
//    matrix_t rot_ax;
//    q_m_convert(&rot_orig, planet->rot_orig);
//    q_m_convert(&rot_ax, planet->rot_ax);
    
//    glMultMatrixf((GLfloat*)rot_orig.a);
//    glTranslatef(planet->pos.x, planet->pos.y, planet->pos.z );
//    glMultMatrixf((GLfloat*)rot_ax.a);
    
    gluSphere(sphere->quadratic, sphere->radius, 64, 64);
    
    glPopMatrix();
    
}

OOnode*
ooSgNewSky(void)
{
    OOstars *stars = ooSkyInitStars(8000);
    
    if (stars == NULL) {return NULL;}
    
    OOnode *node = ooSgNewNode(stars, (OOdrawfunc)ooSkyDrawStars, NULL);

    return node;
}


void
ooSgScale(OOscale *scale)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glScalef(scale->s, scale->s, scale->s);
}

void
ooSgPostScale(OOscale *scale)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


OOnode*
ooSgNewScale(float scale)
{
    OOscale *scaleNode = (OOscale*) malloc(sizeof(OOscale));
    scaleNode->s = scale;
    
    OOnode *node = ooSgNewNode(scaleNode, (OOdrawfunc)ooSgScale, (OOdrawfunc)ooSgPostScale);
    return node;
}
