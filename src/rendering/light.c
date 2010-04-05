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


#include "light.h"
#include "rendering/reftypes.h"
#include "scenegraph-private.h"
#include "camera.h"
#include "physics/object.h"
#include <stdlib.h>
void
sgSetLightPos3f(SGlight *light, float x, float y, float z)
{
  light->pos[0] = vf3_x(x);
  light->pos[1] = vf3_y(y);
  light->pos[2] = vf3_z(z);
  light->pos[3] = 1.0;
}

void
sgSetLightPosv(SGlight *light, float3 v)
{
  light->pos[0] = vf3_x(v);
  light->pos[1] = vf3_y(v);
  light->pos[2] = vf3_z(v);
  light->pos[3] = 1.0;
}

void
sgSetLightPosLW(SGlight *light, OOlwcoord *lwc)
{
  SGscene *sc = light->scene;
  SGscenegraph *sg = sc->sg;
  SGcam *cam = sg->currentCam;
  
  if (cam->kind == SGCam_Free) {
    float3 relPos = ooLwcRelVec(lwc, ((SGfreecam*)cam)->lwc.seg);
    
    light->pos[0] = vf3_x(relPos);
    light->pos[1] = vf3_y(relPos);
    light->pos[2] = vf3_z(relPos);
    light->pos[3] = 1.0;
  } else if (cam->kind == SGCam_Orbit) {
    float3 relPos = ooLwcRelVec(lwc, ((SGorbitcam*)cam)->body->p.seg);
    
    light->pos[0] = vf3_x(relPos);
    light->pos[1] = vf3_y(relPos);
    light->pos[2] = vf3_z(relPos);
    light->pos[3] = 1.0;
  }
}


SGlight*
sgNewSpotlight(SGscenegraph *sg, float3 p, float3 dir)
{
  SGspotlight *light = malloc(sizeof(SGspotlight));
  
  return (SGlight*)light;
}

void
sgEnablePointLight(SGpointlight *light, GLenum lightId)
{
  glEnable(GL_LIGHTING);
  glEnable(lightId);
  light->super.lightId = lightId;
  
  glLightfv(lightId, GL_POSITION, light->super.pos);
  glLightfv(lightId, GL_AMBIENT, light->super.ambient);
  glLightfv(lightId, GL_DIFFUSE, light->super.diffuse);
  glLightfv(lightId, GL_SPECULAR, light->super.specular);
  //  glLightf(light->super.lightId, GL_CONSTANT_ATTENUATION, 1.0f);
  //  glLightf(light->super.lightId, GL_LINEAR_ATTENUATION, 0.2f);
  //  glLightf(light->super.lightId, GL_QUADRATIC_ATTENUATION, 0.08f);
  
}

void
sgDisablePointLight(SGpointlight *light)
{
  glDisable(light->super.lightId);
}

void
sgLightSetAmbient4f(SGlight *light, float r, float g, float b, float a)
{
  light->ambient[0] = r;
  light->ambient[1] = g;
  light->ambient[2] = b;
  light->ambient[3] = a;
  
}
void
sgLightSetSpecular4f(SGlight *light, float r, float g, float b, float a)
{
  light->specular[0] = r;
  light->specular[1] = g;
  light->specular[2] = b;
  light->specular[3] = a;
  
}

void
sgLightSetDiffuse4f(SGlight *light, float r, float g, float b, float a)
{
  light->diffuse[0] = r;
  light->diffuse[1] = g;
  light->diffuse[2] = b;
  light->diffuse[3] = a;
}



SGlight*
sgNewPointlight3f(SGscene *sc, float x, float y, float z)
{
  SGpointlight *light = malloc(sizeof(SGpointlight));
  light->super.enable = (SGenable_light_func)sgEnablePointLight;
  light->super.disable = (SGdisable_light_func)sgDisablePointLight;
  light->super.scene = sc;
  
  light->super.pos[0] = x;
  light->super.pos[1] = y;
  light->super.pos[2] = z;
  light->super.pos[3] = 1.0;
  
  light->super.ambient[0] = 0.0;
  light->super.ambient[1] = 0.0;
  light->super.ambient[2] = 0.0;
  light->super.ambient[3] = 1.0;
  
  light->super.specular[0] = 1.0;
  light->super.specular[1] = 1.0;
  light->super.specular[2] = 1.0;
  light->super.specular[3] = 1.0;
  
  light->super.diffuse[0] = 1.0;
  light->super.diffuse[1] = 1.0;
  light->super.diffuse[2] = 1.0;
  light->super.diffuse[3] = 1.0;
  
  sgSceneAddLight(sc, &light->super);
  
  return (SGlight*)light;
}

SGlight*
sgNewPointlight(SGscene *sc, float3 p)
{
  return sgNewPointlight3f(sc, vf3_x(p), vf3_y(p), vf3_z(p));
}
