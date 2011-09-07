/*
  Copyright 2008,2009,2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdlib.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include "scenegraph.h"
#include "scenegraph-private.h"
#include "sky.h"
#include "log.h"
#include <vmath/vmath.h>
#include "geo/geo.h"
#include "physics/orbit.h"
#include "texture.h"
#include "common/lwcoord.h"
#include "parsers/model.h"
#include "render.h"
#include "shader-manager.h"

SGcam*
sgGetCam(SGscenegraph *sg)
{
  assert(sg != NULL);

  return sg->currentCam;
}


SGscenegraph*
sgNewSceneGraph()
{
  SGscenegraph *sg = malloc(sizeof(SGscenegraph));
  glGetIntegerv(GL_MAX_LIGHTS, &sg->maxLights);
  sg->usedLights = 0;
  obj_array_init(&sg->cams);
  obj_array_init(&sg->overlays);
  obj_array_init(&sg->scenes);

  sg->overlay_shader = sgGetProgram("overlay");
  glUseProgram(sg->overlay_shader);

  sg->modelview_id = glGetUniformLocation(sg->overlay_shader,
                                          "ModelViewMatrix");
  sg->projection_id = glGetUniformLocation(sg->overlay_shader,
                                           "ProjectionMatrix");
  sg->tex_id = glGetUniformLocation(sg->overlay_shader,
                                    "Tex");

  assert(sg->modelview_id != -1);
  assert(sg->projection_id != -1);
  assert(sg->tex_id != -1);

  glUseProgram(0);

  return sg;
}



void
sgSetCam(SGscenegraph *sg, SGcam *cam)
{
  assert(sg != NULL);
  assert(cam != NULL);

  #ifdef DEBUG
  bool found = false;
  for (size_t i = 0 ; i < sg->cams.length ; i ++) {
    if (cam == sg->cams.elems[i]) found = true;
  }
  assert(found == true);
  #endif

  sg->currentCam = cam;
}


SGscene*
sgNewScene(SGscenegraph *sg, const char *name)
{
  assert(name != NULL);

  SGscene *sc = malloc(sizeof(SGscene));
  sc->name = strdup(name);

  sc->amb[0] = 0.2;
  sc->amb[1] = 0.2;
  sc->amb[2] = 0.2;
  sc->amb[3] = 1.0;

  sc->sg = sg;
  sc->lights = calloc(sg->maxLights, sizeof(SGlight*));

  obj_array_init(&sc->objs);
  obj_array_push(&sg->scenes, sc);

  return sc;
}


void
sgSetSceneAmb4f(SGscene *sc, float r, float g, float b, float a)
{
  sc->amb[0] = r;
  sc->amb[1] = g;
  sc->amb[2] = b;
  sc->amb[3] = a;
}


void
sgSceneAddObj(SGscene *sc, SGdrawable *object)
{
  assert(sc != NULL);
  assert(object != NULL);

  object->scene = sc;
  obj_array_push(&sc->objs, object);
}

SGscene*
sgGetScene(SGscenegraph *sg, const char *sceneName)
{
  assert(sg != NULL);

  for (int i = 0 ; i < sg->scenes.length ; ++ i) {
    if (!strcmp(sceneName, ((SGscene*)(sg->scenes.elems[i]))->name)) {
      return (SGscene*)sg->scenes.elems[i];
    }
  }

  assert(0 && "dont ask for non existant things");
  return NULL;
}

void
sgAddOverlay(SGscenegraph *sg, SGoverlay *overlay)
{
  obj_array_push(&sg->overlays, overlay);
}
void
sgInitOverlay(SGoverlay *overlay, SGdrawoverlay drawfunc,
              float x, float y, float w, float h, unsigned rw, unsigned rh)
{
  overlay->enabled = true;
  overlay->draw = drawfunc;
  overlay->x = x;
  overlay->y = y;
  overlay->w = w;
  overlay->h = h;

  glGenFramebuffers(1, &overlay->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, overlay->fbo);
  glGenTextures(1, &overlay->tex);
  glBindTexture(GL_TEXTURE_2D, overlay->tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rw, rh, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  SG_CHECK_ERROR;

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         overlay->tex, 0);
  SG_CHECK_ERROR;

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  assert(status == GL_FRAMEBUFFER_COMPLETE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void
sgDrawOverlays(SGscenegraph *sg)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, sgRenderInfo.w, 0.0, sgRenderInfo.h, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glPushAttrib(GL_ENABLE_BIT);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);

  for (size_t i = 0 ; i < sg->overlays.length ; i ++) {
    SGoverlay *overlay = ARRAY_ELEM(sg->overlays, i);

    if (overlay->enabled) {
      // Bind the fbo texture so that the mfd rendering ends up in the texture
      glBindFramebuffer(GL_FRAMEBUFFER, overlay->fbo);
      glPushAttrib(GL_VIEWPORT_BIT);

      glViewport(0,0, overlay->w, overlay->h);

      ooLogTrace("draw overlay %d", i);
      // Here we draw the overlay into a texture
      overlay->draw(overlay);

      glPopAttrib();

      // Re-attach the real framebuffer as rendering target, and draw the
      // texture using a quad.
      glUseProgram(sg->overlay_shader);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      SG_CHECK_ERROR;

      // Set shader arguments
      GLfloat modelview[16], projection[16];
      glGetFloatv(GL_MODELVIEW_MATRIX , modelview);
      glGetFloatv(GL_PROJECTION_MATRIX , projection);

      glUniformMatrix4fv(sg->modelview_id, 1, GL_FALSE, modelview);
      glUniformMatrix4fv(sg->projection_id, 1, GL_FALSE, projection);

      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, overlay->tex);
      glUniform1i(sg->tex_id, 0);

      GLint shaderIsValid;
      glValidateProgram(sg->overlay_shader);
      glGetProgramiv(sg->overlay_shader, GL_VALIDATE_STATUS, &shaderIsValid);
      assert(shaderIsValid);

      SG_CHECK_ERROR;
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBegin(GL_QUADS);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f (overlay->x, overlay->y, 0.0);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(overlay->x, overlay->y + overlay->h, 0.0);

        glTexCoord2f(1.0, 1.0);
        glVertex3f(overlay->x + overlay->w, overlay->y + overlay->h, 0.0);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(overlay->x + overlay->w, overlay->y, 0.0);
      glEnd();

      SG_CHECK_ERROR;

      glUseProgram(0);
    }
  }


  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib();
}

int
compareDistances(SGdrawable const **o0, SGdrawable const **o1)
{
  bool gt = vf3_gt((*o0)->p, (*o1)->p);

  if (gt) return -1;
  else return 1;
}

void
sgSceneDraw(SGscene *sc)
{
  assert(sc != NULL);
  // Sort objects based on distance from camera, since we are moving
  // objects and not the camera, this is trivial.
  //  qsort(sc->objs.elems, sc->objs.length, sizeof(OOdrawable*),
  //        (int (*)(void const *, void const *))compareDistances);

  SG_CHECK_ERROR;

  glClear(GL_DEPTH_BUFFER_BIT);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sc->amb);
  glDepthFunc(GL_LEQUAL);

  // Apply scene transforms
  glPushAttrib(GL_ENABLE_BIT);
  glPushMatrix();
  int localLights = 0;
  for (int i = 0 ; i < sc->sg->maxLights ; ++ i) {
    if (sc->lights[i] != NULL) {
     if (sc->sg->usedLights < sc->sg->maxLights) {
       GLenum lightId = sgLightNumberMap[sc->sg->usedLights];
       SGlight *light = sc->lights[i];

       light->enable(light, lightId);

       sc->sg->usedLights ++;
       localLights ++;
     } else {
       ooLogWarn("to many light sources in scene hierarchy, current scene = '%s'",
                 sc->name);
     }
    }
  }

  SG_CHECK_ERROR;
  // Render objects
  for (size_t i = 0 ; i < sc->objs.length ; i ++) {
    SGdrawable *obj = sc->objs.elems[i];
    ooLogTrace("drawing object %s", obj->name);
    sgPaintDrawable(obj);
  }
  SG_CHECK_ERROR;

  // Pop scene transform
  glPopAttrib();
  glPopMatrix();


  for (int i = 0 ; i < localLights ; ++ i) {
    if (sc->lights[i] != NULL) {
      SGlight *light = sc->lights[i];
      light->disable(light);
    } else {
      ooLogWarn("null found in light vector");
    }
  }

  sc->sg->usedLights -= localLights;
  SG_CHECK_ERROR;
}


void
sgSetSky(SGscenegraph *sg, SGdrawable *obj)
{
  assert(sg != NULL);
  assert(obj != NULL);
  sg->sky = obj;
}
// Drawing is done as follows:
//   when drawing is commanded with a camera, we get the cameras scene and
//   paint that, after that, we go to the cam scenes parent and inverse
//   any rotations and transforms and paint that
void
sgPaint(SGscenegraph *sg)
{
  ooSetPerspective(sgRenderInfo.fovy, sgRenderInfo.w, sgRenderInfo.h);

  assert(sg != NULL);
  ooLogTrace("SgPaint");

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // Near clipping 1 m away, far clipping 20 au away
  gluPerspective(sgRenderInfo.fovy, sgRenderInfo.aspect,
                 /*near*/0.9, /*far*/149598000000.0*20.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPushMatrix();
  glPushAttrib(GL_ENABLE_BIT);
  sgCamRotate(sg->currentCam);
  // Draw the sky
  sg->sky->draw(sg->sky);
  glPopAttrib();
  glPopMatrix();

  glPushMatrix();

  sgCamRotate(sg->currentCam);
  sgCamMove(sg->currentCam);
  sgSceneDraw(sg->currentCam->scene);

  glPopMatrix();

  // Draw overlays
  glPushAttrib(GL_ENABLE_BIT);
  sgDrawOverlays(sg);
  glPopAttrib();
}

void
sgDrawFuncGnd(SGdrawable *obj)
{

}



void
sgSceneAddLight(SGscene *sc, SGlight *light)
{
  for (int i = 0 ; i < sc->sg->maxLights ; ++ i) {
    if (sc->lights[i] == NULL) {
      sc->lights[i] = light;
      return;
    }
  }

  ooLogWarn("to many lights added to scenes '%s'", sc->name);
}

void
sgAssertNoGLError(void)
{
  assert(glGetError() == GL_NO_ERROR);
}
void
sgClearGLError(void)
{
  glGetError();
}

void
sgCheckGLError(const char *file, int line)
{
  GLenum err = glGetError();
  switch (err) {
  case GL_NO_ERROR:
    // All OK
    break;
  case GL_INVALID_ENUM:
    ooLogError("GL invalid enum at %s:%d", file, line);
    break;
  case GL_INVALID_VALUE:
    ooLogError("GL invalid value at %s:%d", file, line);
    break;
  case GL_INVALID_OPERATION:
    ooLogError("GL invalid operation at %s:%d", file, line);
    break;
  case GL_STACK_OVERFLOW:
    ooLogError("GL stack overflow at %s:%d", file, line);
    break;
  case GL_STACK_UNDERFLOW:
    ooLogError("GL stack underflow at %s:%d", file, line);
    break;
  case GL_OUT_OF_MEMORY:
    ooLogError("GL out of memory at %s:%d", file, line);
    break;
  case GL_TABLE_TOO_LARGE:
    ooLogError("GL invalid enum at %s:%d", file, line);
    break;
  default:
    ooLogError("unknown GL error at %s:%d", file, line);
  }
}
