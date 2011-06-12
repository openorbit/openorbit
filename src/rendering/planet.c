/*
  Copyright 2007 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "planet.h"
#include "texture.h"
#include <vmath/vmath.h>


#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

void ooPlanetDraw(OOplanet *planet)
{
//  if (planet->)
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, planet->texId);

  //  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glColor3f(1.0f, 1.0f, 1.0f);
  gluSphere(planet->quadratic, planet->radius, 128, 128);

  glMatrixMode(GL_MODELVIEW);

  // Switch rendering options based on distance to camera
  // Get 
//  if ()
}

/* This code is temporary */



static int planet_count;
planet_t gPlanets[MAX_PLANETS];

int
planet_add(float x, float y, float z, float radius, float mass, const char *tex_key)
{
  if (planet_count >= MAX_PLANETS) {
    return -1;
  }

  gPlanets[planet_count].texid = ooTexNum(tex_key);
  glBindTexture(GL_TEXTURE_2D, gPlanets[planet_count].texid);

  gPlanets[planet_count].quadratic = gluNewQuadric();
  gluQuadricNormals(gPlanets[planet_count].quadratic, GLU_SMOOTH);
  gluQuadricTexture(gPlanets[planet_count].quadratic, GL_TRUE);
  // Assume this will work...
  gPlanets[planet_count].pos = vf3_set(x, y, z);

  Q_ROT_X(gPlanets[planet_count].rot_ax, DEG_TO_RAD(-90.0f));
  Q_ROT_X(gPlanets[planet_count].rot_orig, DEG_TO_RAD(0.0f));

  gPlanets[planet_count].radius = radius;
    
  planet_count ++;
  return 0;
}

void
planet_rot_ax(planet_t *planet, float rot_rad)
{
  quaternion_t res, rot;
  Q_ROT_Y(rot, rot_rad);

  res = q_mul(planet->rot_ax, rot);
  planet->rot_ax = res;
}

void
planet_rot_orig(planet_t *planet, float rot_rad)
{
  quaternion_t res, rot;
  Q_ROT_Y(rot, rot_rad);

  res = q_mul(planet->rot_orig, rot);
  planet->rot_orig = res;
}

//void
//planet_draw2(planet2_t *planet)
//{
 //   
//}

void
planet_draw(planet_t *planet)
{
  matrix_t m;
  glPushMatrix();

  glBindTexture(GL_TEXTURE_2D, planet->texid);

  matrix_t rot_orig;
  matrix_t rot_ax;
  q_m_convert(&rot_orig, planet->rot_orig);
  matrix_t rot_orig_t;
  m_transpose(&rot_orig_t, &rot_orig);

  q_m_convert(&rot_ax, planet->rot_ax);
  matrix_t rot_ax_t;
  m_transpose(&rot_ax_t, &rot_ax);

  glMultMatrixf((GLfloat*)rot_orig_t.a);
  glTranslatef(vf3_x(planet->pos), vf3_y(planet->pos), vf3_z(planet->pos) );
  glMultMatrixf((GLfloat*)rot_ax_t.a);

  gluSphere(planet->quadratic, planet->radius, 64, 64);

  glPopMatrix();
}

void
planet_draw_all(void)
{
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glPolygonMode(GL_FRONT,GL_FILL);
  glPolygonMode(GL_BACK,GL_LINE);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  GLfloat lightCol[] = {1.0,1.0,1.0,1.0};
  GLfloat pos[] = {0.0, 0.0, 0.0, 1.0};
  glShadeModel(GL_SMOOTH);

  //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  GLfloat col[] = {1.0f, 1.0f, 1.0f, 0.0f};
  glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
  //glMaterialfv(GL_FRONT, GL_SPECULAR, col);
  glColor3f(1.0f, 1.0f, 1.0f);

  for (int i = 1 ; i < planet_count ; i ++) {
    planet_draw(&gPlanets[i]);
  }

  glDisable(GL_LIGHTING);

  planet_draw(&gPlanets[0]);
  glDisable(GL_TEXTURE_2D);
}