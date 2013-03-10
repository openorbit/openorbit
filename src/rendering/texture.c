/*
  Copyright 2006,2009,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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



#include <stdlib.h>
#include <assert.h>
#include <gencds/hashtable.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
//#include <OpenGL/glu.h>
#else
#include <GL3/gl3.h>
//#include <GL/glu.h>
#endif

#include <openorbit/log.h>
#include "texture.h"
#include <imgload/image.h>

#include "res-manager.h"
#include "rendering/scenegraph.h"
#include <string.h>
#include "palloc.h"

struct sg_texture_t {
  GLint width, height;
  GLuint texId;
  GLenum internalType;
  GLenum texType;
  GLint bytesPerTex;
  char *path;
  void *data;
};


static hashtable_t *gOOtexDict;

static void __attribute__((constructor))
texInit(void)
{
    gOOtexDict = hashtable_new_with_str_keys(128);
}

sg_texture_t*
sg_load_texture(const char *key)
{
  SG_CHECK_ERROR;
  if (!key) return NULL;

  sg_texture_t *tex = NULL;
  if ((tex = hashtable_lookup(gOOtexDict, key))) {
    // Don't reload textures, return memoized value
    return tex;
  }

  char *fname = rsrc_get_path(key);
  if (fname == NULL) return 0;

  image_t img;

  tex = smalloc(sizeof(sg_texture_t));
  if (tex == NULL) {free(fname); return 0;}

  int res = img_load(&img, fname);
  if (res != 0) {free(fname);free(tex);return 0;}
  tex->path = strdup(key);
  switch (img.kind) {
  case IMG_BGRA:
    tex->internalType = GL_RGBA8;
    tex->texType = GL_BGRA;
    tex->bytesPerTex = 4;
    break;
  case IMG_BGR:
    tex->internalType = GL_RGB8;
    tex->texType = GL_BGR;
    tex->bytesPerTex = 3;
    break;
  case IMG_RGB:
    tex->internalType = GL_RGB8;
    tex->texType = GL_RGB;
    tex->bytesPerTex = 3;
    break;
  case IMG_RGBA:
    tex->internalType = GL_RGBA8;
    tex->texType = GL_RGBA;
    tex->bytesPerTex = 4;
    break;
  case IMG_GRAY8:
    // IN GL < 3 we could use Luminance for grayscale images, but not anymore
    //  we need shader support to handle this mess now...
    tex->internalType = GL_R8;
    tex->texType = GL_RED;
    tex->bytesPerTex = 1;
    break;
  default:
    assert(0 && "invalid case");
  }

  tex->width = img.w;
  tex->height = img.h;
  tex->data = img.data;

  SG_CHECK_ERROR;

  glGenTextures(1, &tex->texId);
  SG_CHECK_ERROR;
  glBindTexture(GL_TEXTURE_2D, tex->texId);
  SG_CHECK_ERROR;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  SG_CHECK_ERROR;
  glTexImage2D(GL_TEXTURE_2D, 0, tex->internalType,
               tex->width, tex->height, 0,
               tex->texType, GL_UNSIGNED_BYTE, tex->data);
  SG_CHECK_ERROR;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  SG_CHECK_ERROR;

  glGenerateMipmap(GL_TEXTURE_2D);
  //if ((err = gluBuild2DMipmaps(GL_TEXTURE_2D,
  //                            tex->bytesPerTex,
  //                            tex->width, tex->height,
  //                            tex->texType,
  //                            GL_UNSIGNED_BYTE,
  //                            tex->data)))
  //{
  //  log_fatal("failed mipmap generation %s", gluErrorString(err));
  //}


  // Do not retain texture, is is loaded into GL texture memory
  free(tex->data);
  tex->data = NULL;

  // Memoize loaded texture
  hashtable_insert(gOOtexDict, key, tex);
  free(fname);

  SG_CHECK_ERROR;

  return tex;
}

GLuint
sg_texture_get_id(sg_texture_t *tex)
{
  return tex->texId;
}

void
sg_texture_unload(sg_texture_t *tex)
{
  hashtable_remove(gOOtexDict, tex->path);
  glDeleteTextures(1, &tex->texId);

  free(tex->data);
  free(tex->path);
  free(tex);
}

sg_texture_t*
sg_find_texture(const char *key)
{
  return hashtable_lookup(gOOtexDict, key);
}
