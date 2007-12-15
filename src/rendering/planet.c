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
 created by the Initial Developer are Copyright (C) 2007 the
 Initial Developer. All Rights Reserved.
 
 Contributor(s):
 Mattias Holm <mattias.holm(at)contra.nu>.
 
 Alternatively, the contents of this file may be used under the terms of
 either the GNU General Public License Version 2 or later (the "GPL"), or
 the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
 which case the provisions of GPL or the LGPL License are applicable instead
 of those above. If you wish to allow use of your version of this file only
 under the terms of the GPL or the LGPL and not to allow others to use your
 version of this file under the MPL, indicate your decision by deleting the
 provisions above and replace them with the notice and other provisions
 required by the GPL or the LGPL. If you do not delete the provisions
 above, a recipient may use your version of this file under either the MPL,
 the GPL or the LGPL."
 */

#include "planet.h"
#include "texture.h"
#include "math/quaternions.h"
#include "math/convert.h"


#include "SDL_opengl.h"
/* This code is temporary */

static int planet_count;
planet_t gPlanets[MAX_PLANETS];

int
planet_add(float x, float y, float z, float radius, float mass, const char *tex_key)
{
    if (planet_count >= MAX_PLANETS) {
        return -1;
    }
    
    gPlanets[planet_count].texid = tex_num(tex_key);
    glBindTexture(GL_TEXTURE_2D, gPlanets[planet_count].texid);
    
    
    gPlanets[planet_count].quadratic = gluNewQuadric();
    gluQuadricNormals(gPlanets[planet_count].quadratic, GLU_SMOOTH);
    gluQuadricTexture(gPlanets[planet_count].quadratic, GL_TRUE);
    // Assume this will work...
    gPlanets[planet_count].pos.s.x = x;
    gPlanets[planet_count].pos.s.y = y;
    gPlanets[planet_count].pos.s.z = z;
    gPlanets[planet_count].pos.s.w = 1.0;

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

    Q_MUL(res, planet->rot_ax, rot);
    Q_CPY(planet->rot_ax, res);
}

void
planet_rot_orig(planet_t *planet, float rot_rad)
{
    quaternion_t res, rot;
    Q_ROT_Y(rot, rot_rad);
    
    Q_MUL(res, planet->rot_orig, rot);
    Q_CPY(planet->rot_orig, res);
}



void
planet_draw(planet_t *planet)
{
    matrix_t m;
    glPushMatrix();
    
    glBindTexture(GL_TEXTURE_2D, planet->texid);
        
    matrix_t rot_orig;
    matrix_t rot_ax;
    Q_M_CONVERT(rot_orig, planet->rot_orig);
    Q_M_CONVERT(rot_ax, planet->rot_ax);
    
    glMultMatrixf((GLfloat*)rot_orig.a);
    glTranslatef(planet->pos.s.x, planet->pos.s.y, planet->pos.s.z );
    glMultMatrixf((GLfloat*)rot_ax.a);
    
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