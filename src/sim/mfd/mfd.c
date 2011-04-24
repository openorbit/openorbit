/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <assert.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include "mfd.h"

#include "common/moduleinit.h"
#include "common/palloc.h"
#include "io-manager.h"
#include "rendering/render.h"
#include "sim.h"

void simMfdInit(SIMmfd *mfd, unsigned orig_x, unsigned orig_y,
                unsigned width, unsigned height);

static obj_array_t mfd_pages;

static SIMmfd mfd0, mfd1, mfd2, mfd3;
static SIMhud hud;

INIT_PRIMARY_MODULE {
  obj_array_init(&mfd_pages);
}

static void
mfd_draw(SGoverlay *overlay)
{
  SIMmfd *mfd = (SIMmfd*)overlay;

  if (mfd_pages.length > 0) {
    SIMmfdpage *page = ARRAY_ELEM(mfd_pages, mfd->page_no);
    page->draw(page);
  } else {
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
  }
}



// For registring sample mfd

static void
samplemfddraw(SIMmfdpage *mfd)
{
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);

  glClearColor(0.0625, 0.05078125, 0.2265625, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

static SIMmfdpage sampleMfdPage = {"sample-mfd", samplemfddraw};


INIT_MFD {
  simMfdPageRegister(&sampleMfdPage);
}

//typedef void (*OObuttonhandlerfunc)(bool buttonDown, void *data);

static void
mfdCycleNext(bool buttonDown, void *data)
{
  assert(mfd_pages.length > 0);
  SIMmfd *mfd = data;
  mfd->page_no = (mfd->page_no + 1) % mfd_pages.length;
}

static void
mfdCyclePrev(bool buttonDown, void *data)
{
  assert(mfd_pages.length > 0);
  SIMmfd *mfd = data;
  if (mfd->page_no == 0) mfd->page_no = mfd_pages.length - 1;
  else mfd->page_no --;
}


static void
mfdToggle(bool buttonDown, void *data)
{
  SIMmfd *mfd = data;
  mfd->super.enabled = !mfd->super.enabled;
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
test_hud_draw(SGoverlay *overlay)
{
  SIMhud *hud = (SIMhud*)overlay;

  OOspacecraft *sc = simGetSpacecraft();
  float3 gv = simGetGravityVector(sc);
  quaternion_t q = simGetQuaternion(sc);

  glPushAttrib(GL_ENABLE_BIT);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LINE_SMOOTH);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(-1.0*sgRenderInfo.aspect, 1.0*sgRenderInfo.aspect,
             -1.0, 1.0); // TODO: Fix to viewport aspect

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glLineWidth(1.0);

  glBegin(GL_LINES);
  glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
  glVertex2f(-0.5f, 0.0f);
  glVertex2f(0.5f, 0.0f);

  glVertex2f(0.0f, -0.5f);
  glVertex2f(0.0f, 0.5f);

  glEnd();

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopAttrib();
}


void
simHudDraw(SIMhud *hud)
{
  assert(hud != NULL);
  // TODO: Draw frame
  hud->draw(hud);
}


void
simMfdInitAll(SGscenegraph *sg)
{
  mfd0.page_no = 0;
  mfd1.page_no = 0;
  mfd2.page_no = 0;
  mfd3.page_no = 0;

  sgInitOverlay(&mfd0.super, mfd_draw,   0.0,   0.0, 128.0, 128.0, 128, 128);
  sgInitOverlay(&mfd1.super, mfd_draw, sgRenderInfo.w - 128.0,   0.0,
                128.0, 128.0, 128, 128);
  sgInitOverlay(&mfd2.super, mfd_draw, sgRenderInfo.w - 128.0,
                sgRenderInfo.h - 128.0, 128.0, 128.0, 128, 128);
  sgInitOverlay(&mfd3.super, mfd_draw,   0.0, sgRenderInfo.h - 128.0,
                128.0, 128.0, 128, 128);

  sgInitOverlay(&hud.super, test_hud_draw,   0.0, 0.0,
                sgRenderInfo.w, sgRenderInfo.h, 128, 128);


  sgAddOverlay(sg, &mfd0.super);
  sgAddOverlay(sg, &mfd1.super);
  sgAddOverlay(sg, &mfd2.super);
  sgAddOverlay(sg, &mfd3.super);
  sgAddOverlay(sg, &hud.super);
}


INIT_IO {
  ioRegActionHandler("mfd0-cycle-next", mfdCycleNext, &mfd0);
  ioRegActionHandler("mfd0-cycle-prev", mfdCyclePrev, &mfd0);
  ioRegActionHandler("mfd0-toggle", mfdToggle, &mfd0);
  ioRegActionHandler("mfd1-cycle-next", mfdCycleNext, &mfd1);
  ioRegActionHandler("mfd1-cycle-prev", mfdCyclePrev, &mfd1);
  ioRegActionHandler("mfd1-toggle", mfdToggle, &mfd1);
  ioRegActionHandler("mfd2-cycle-next", mfdCycleNext, &mfd2);
  ioRegActionHandler("mfd2-cycle-prev", mfdCyclePrev, &mfd2);
  ioRegActionHandler("mfd2-toggle", mfdToggle, &mfd2);
  ioRegActionHandler("mfd3-cycle-next", mfdCycleNext, &mfd3);
  ioRegActionHandler("mfd3-cycle-prev", mfdCyclePrev, &mfd3);
  ioRegActionHandler("mfd3-toggle", mfdToggle, &mfd3);
}
