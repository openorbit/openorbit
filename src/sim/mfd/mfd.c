/*
 Copyright 2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit.

 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include "mfd.h"

#include <openorbit/log.h>
#include "common/moduleinit.h"
#include "common/palloc.h"
#include "io-manager.h"
#include "rendering/overlay.h"
#include "sim.h"

void simMfdInit(SIMmfd *mfd, unsigned orig_x, unsigned orig_y,
                unsigned width, unsigned height);

static obj_array_t mfd_pages;

static SIMmfd mfd0, mfd1, mfd2, mfd3;
static SIMhud hud;

MODULE_INIT(mfd, NULL) {
  log_trace("initialising 'mfd' module");
  obj_array_init(&mfd_pages);
}

static void
mfd_draw(sg_overlay_t *overlay, void *obj)
{
  SG_CHECK_ERROR;
  SIMmfd *mfd = (SIMmfd*)obj;

  if (mfd_pages.length > 0) {
    SIMmfdpage *page = ARRAY_ELEM(mfd_pages, mfd->page_no);
    page->draw(page);
  } else {
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  SG_CHECK_ERROR;
}



// For registring sample mfd

static void
samplemfddraw(SIMmfdpage *mfd)
{
  SG_CHECK_ERROR;

  glDisable(GL_BLEND);
  glClearColor(0.0625, 0.05078125, 0.2265625, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  SG_CHECK_ERROR;
}

static SIMmfdpage sampleMfdPage = {"sample-mfd", samplemfddraw};


MODULE_INIT(samplemfd, "mfd", NULL) {
  log_trace("initialising 'samplemfd' module");
  simMfdPageRegister(&sampleMfdPage);
}

//typedef void (*OObuttonhandlerfunc)(bool buttonDown, void *data);

static void
mfdCycleNext(int state, void *data)
{
  if (state) {
    assert(mfd_pages.length > 0);
    SIMmfd *mfd = data;
    mfd->page_no = (mfd->page_no + 1) % mfd_pages.length;
  }
}

static void
mfdCyclePrev(int state, void *data)
{
  if (state) {
    assert(mfd_pages.length > 0);
    SIMmfd *mfd = data;
    if (mfd->page_no == 0) mfd->page_no = mfd_pages.length - 1;
    else mfd->page_no --;
  }
}


static void
mfdToggle(int state, void *data)
{
  if (state) {
    SIMmfd *mfd = data;
    (void)mfd; // TODO
    //mfd->super.enabled = !mfd->super.enabled;
  }
}

void
simMfdPageRegister(SIMmfdpage *page)
{
  assert(page != NULL);
  obj_array_push(&mfd_pages, page);
}


void
simCycleMfd(SIMmfd *mfd)
{
  assert(mfd != NULL);
}

void
simSelectMfdByKey(SIMmfd *mfd, const char *key)
{
  assert(mfd != NULL);
  assert(key != NULL);
  ARRAY_FOR_EACH(i, mfd_pages) {
    SIMmfdpage *page = ARRAY_ELEM(mfd_pages, i);
    if (!strcmp(page->key, key)) {
      mfd->page_no = i;
      break;
    }
  }
}

void
simSelectMfd(SIMmfd *mfd, unsigned mfdId)
{
  assert(mfd != NULL);
  assert(mfdId < mfd_pages.length);

  if (mfdId < mfd_pages.length) {
    mfd->page_no = mfdId;
  }
}

void
test_hud_draw(sg_overlay_t *overlay, void *obj)
{
  SG_CHECK_ERROR;

  SIMhud *hud = (SIMhud*)obj;

  // TODO
  sim_spacecraft_t *sc = sim_get_spacecraft();
  //float3 gv = simGetGravityVector(sc);

  //quaternion_t q = simGetQuaternion(sc);
  const double3x3 *R = sim_spacecraft_get_rotmat(sc);
  double3x3 R_inv;
  md3_transpose2(R_inv, *R); // Invert rotation matrix, we need this to
                             // transform the vectors into view relative values
  //float3 v = simGetVelocityVector(sc);
  //float3 as = simGetAirspeedVector(sc);
  //float3 p = simGetRelPos(sc);
  //float3 rv = simGetRelVel(sc);

  //float3 v_view = mf3_v_mul(R_inv, v);
  //float3 g_view = mf3_v_mul(R_inv, gv);
  //float3 rv_view = mf3_v_mul(R_inv, rv);
  //float3 as_view = mf3_v_mul(R_inv, as);
  //float3 p_view = mf3_v_mul(R_inv, p);

  //float h = simGetAltitude(sc);
  // The hud should display the following data: airspeed, altitude, horizon,
  // heading, turn / bank
  // Angle of attack, accelleration, boresight, flight path vector
  // Hud should adopt to whether it is used in space or not.
  // Hud need to be able to select reference object in space as data is always
  // relative.
  //glPushAttrib(GL_ENABLE_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glClear(GL_COLOR_BUFFER_BIT);

  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);

  //glMatrixMode(GL_PROJECTION);
  //glPushMatrix();
  //glLoadIdentity();
  //gluOrtho2D(-1.0*sgRenderInfo.aspect, 1.0*sgRenderInfo.aspect,
  //           -1.0, 1.0); // TODO: Fix to viewport aspect

  //glMatrixMode(GL_MODELVIEW);
  //glPushMatrix();
  //glLoadIdentity();

  glLineWidth(1.0);

  //glBegin(GL_LINES);
  //glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
  //glVertex2f(-0.5f, 0.0f);
  //glVertex2f(0.5f, 0.0f);

  //glVertex2f(0.0f, -0.5f);
  // glVertex2f(0.0f, 0.5f);

  //glEnd();
  SG_CHECK_ERROR;

  glActiveTexture(GL_TEXTURE0);
  SG_CHECK_ERROR;

  glBindTexture(GL_TEXTURE_2D, sgTextTexture(hud->text));

  SG_CHECK_ERROR;
  //GLuint tex_num = glGetUniformLocation(, )
  //glUniform1i(, 0);
  //sgBindTextBuffer(hud->text);
  //glDisable(GL_LIGHTING);
  //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

  //glColor4f(0.0, 1.0, 0.0, 0.0);
  //glBegin(GL_QUADS);

  //glTexCoord2f(0.0, 0.0);
  //glVertex2f(-0.025f, 0.025f);

  //glTexCoord2f(1.0, 0.0);
  //glVertex2f(0.025f, 0.025f);

  //glTexCoord2f(1.0, 1.0);
  //glVertex2f(0.025f, -0.025f);

  //glTexCoord2f(0.0, 1.0);
  //glVertex2f(-0.025f, -0.025f);

  //glEnd();

  //glPopMatrix();
  // glMatrixMode(GL_PROJECTION);
  //glPopMatrix();
  // glMatrixMode(GL_MODELVIEW);
  //glPopAttrib();
  SG_CHECK_ERROR;
}


void
simHudDraw(SIMhud *hud)
{
  assert(hud != NULL);
  // TODO: Draw frame
  hud->draw(hud);
}


void
simMfdInitAll(sg_viewport_t *vp)
{
  mfd0.page_no = 0;
  mfd1.page_no = 0;
  mfd2.page_no = 0;
  mfd3.page_no = 0;

  sg_overlay_t *overlay0    = sg_new_overlay(),
               *overlay1    = sg_new_overlay(),
               *overlay2    = sg_new_overlay(),
               *overlay3    = sg_new_overlay(),
               *overlay_hud = sg_new_overlay();

  sg_overlay_init(overlay0, mfd_draw, &mfd0, 0.0,   0.0, 128.0, 128.0, 128, 128);
  sg_overlay_init(overlay1, mfd_draw, &mfd1, sg_viewport_get_width(vp) - 128.0,   0.0,
                  128.0, 128.0, 128, 128);
  sg_overlay_init(overlay2, mfd_draw, &mfd2, sg_viewport_get_width(vp)- 128.0,
                 sg_viewport_get_height(vp) - 128.0, 128.0, 128.0, 128, 128);
  sg_overlay_init(overlay3, mfd_draw, &mfd3, 0.0, sg_viewport_get_height(vp) - 128.0,
                128.0, 128.0, 128, 128);

  sg_overlay_init(overlay_hud, test_hud_draw, &hud,  0.0, 0.0,
                  sg_viewport_get_width(vp), sg_viewport_get_height(vp),
                  sg_viewport_get_width(vp), sg_viewport_get_height(vp));

  hud.text = sgNewTextLabel("Helvetica", 14.0, "20");
  SG_CHECK_ERROR;
  hud.lines = 360 / 10;
  hud.lineVerts = calloc(hud.lines, sizeof(GLfloat)*4);
  for (int i = 0 ; i < hud.lines ; i++) {
    hud.lineVerts[i*4+0] = -0.5f;
    hud.lineVerts[i*4+1] = i*0.1f; // TODO: What will we set the lines to?

    hud.lineVerts[i*4+2] = 0.5f;
    hud.lineVerts[i*4+3] = i*0.1f;
  }

  for (int i = 0 ; i < SIM_HUD_MARKS ; i++) {
    hud.marks[i] = sgNewTextLabelf("Helvetica", 14, "%d", i * SIM_HUD_STEP);
  }

  glGenBuffers(1, &hud.lineBuff);
  SG_CHECK_ERROR;
  glBufferData(GL_ARRAY_BUFFER, hud.lines*4*sizeof(GLfloat), hud.lineVerts,
               GL_STATIC_DRAW);

  sg_viewport_add_overlay(vp, overlay0);
  sg_viewport_add_overlay(vp, overlay1);
  sg_viewport_add_overlay(vp, overlay2);
  sg_viewport_add_overlay(vp, overlay3);
  sg_viewport_add_overlay(vp, overlay_hud);
}

MODULE_INIT(mfdio, "mfd", "iomanager", NULL) {
  log_trace("initialising 'mfdio' module");

  io_reg_action_handler("mfd0-cycle-next", mfdCycleNext, IO_BUTTON_PUSH, &mfd0);
  io_reg_action_handler("mfd0-cycle-prev", mfdCyclePrev, IO_BUTTON_PUSH, &mfd0);
  io_reg_action_handler("mfd0-toggle", mfdToggle, IO_BUTTON_PUSH, &mfd0);
  io_reg_action_handler("mfd1-cycle-next", mfdCycleNext, IO_BUTTON_PUSH, &mfd1);
  io_reg_action_handler("mfd1-cycle-prev", mfdCyclePrev, IO_BUTTON_PUSH, &mfd1);
  io_reg_action_handler("mfd1-toggle", mfdToggle, IO_BUTTON_PUSH, &mfd1);
  io_reg_action_handler("mfd2-cycle-next", mfdCycleNext, IO_BUTTON_PUSH, &mfd2);
  io_reg_action_handler("mfd2-cycle-prev", mfdCyclePrev, IO_BUTTON_PUSH, &mfd2);
  io_reg_action_handler("mfd2-toggle", mfdToggle, IO_BUTTON_PUSH, &mfd2);
  io_reg_action_handler("mfd3-cycle-next", mfdCycleNext, IO_BUTTON_PUSH, &mfd3);
  io_reg_action_handler("mfd3-cycle-prev", mfdCyclePrev, IO_BUTTON_PUSH, &mfd3);
  io_reg_action_handler("mfd3-toggle", mfdToggle, IO_BUTTON_PUSH, &mfd3);
}
