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

#ifndef SIM_MFD_MFD_H
#define SIM_MFD_MFD_H

#include <gencds/array.h>
#include "rendering/scenegraph.h"
#include "rendering/text.h"

typedef struct SIMmfd SIMmfd;
typedef struct SIMmfdpage SIMmfdpage;

typedef void (*SIMmfddraw)(SIMmfdpage *mfd);

struct SIMmfdpage {
  const char *key;
  // drawing context
  SIMmfddraw draw;
};

struct SIMmfd {
  SGoverlay super;
  unsigned page_no;
};

void simMfdPageRegister(SIMmfdpage *page);

void simMfdInitAll(SGscenegraph *sg);

// Draw top level MFD and the MFD screen
void simMfdDraw(SIMmfd *mfd);
void simCycleMfd(SIMmfd *mfd);
void simSelectMfdByKey(SIMmfd *mfd, const char *key);
void simSelectMfd(SIMmfd *mfd, unsigned mfdId);

typedef struct SIMhud SIMhud;

typedef void (*SIMhuddraw)(SIMhud *hud);

#define SIM_HUD_STEP 10
#define SIM_HUD_MARKS (360/SIM_HUD_STEP)

struct SIMhud {
  SGoverlay super;

  // drawing context
  SIMhuddraw draw;
  SGtextbuffer *text;
  SGtextbuffer *marks[SIM_HUD_MARKS]; // Degree marks
  int lines;
  GLuint lineBuff;
  GLfloat *lineVerts;
};

void simHudDraw(SIMhud *hud);


#endif /* !SIM_MFD_MFD_H */
