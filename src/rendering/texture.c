/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include <gencds/hashtable.h>
#include "SDL_opengl.h"

#include "log.h"
#include "texture.h"
#include "parsers/tga.h"

#include "res-manager.h"

hashtable_t *gOOtexDict;

void
ooTexInit(void)
{
    gOOtexDict = hashtable_new_with_str_keys(128);
}

int
ooTexLoad(const char *key, const char *name)
{
    if (hashtable_lookup(gOOtexDict, key)) {
        ooLogWarn("Tried to load texture '%s' which is already loaded", key);
        return -1;
    }
    FILE *fp = ooResGetFile(name);
    if (fp == NULL) return -1;

    tga_image_t img;
    
    OOtexture *tex = malloc(sizeof(OOtexture));
    if (tex == NULL) return -1;
    
    int res = tga_read_file(&img, fp); 
    if (res != 0) return -1;
    
    
    if ((img.header.img_spec.depth == 32) && (img.header.img_spec.alpha_bits == 8)) {
        tex->texType = GL_BGRA;
    } else if ((img.header.img_spec.depth == 24) && (img.header.img_spec.alpha_bits == 0)) {
        tex->texType = GL_BGR;
    }
    
    tex->width = img.header.img_spec.width;
    tex->height = img.header.img_spec.height;
    tex->data = img.data;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &tex->texId);
    
    glBindTexture(GL_TEXTURE_2D, tex->texId); 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    if (tex->texType == GL_BGR) {
        glTexImage2D(GL_TEXTURE_2D, 0, 3, tex->width, tex->height, 0, GL_BGR,
                     GL_UNSIGNED_BYTE, tex->data);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, 4, tex->width, tex->height, 0, GL_BGRA,
                     GL_UNSIGNED_BYTE, tex->data);
    }
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    GL_NEAREST);
    
    hashtable_insert(gOOtexDict, key, tex);
    return 0;
}

int
ooTexBind(const char *key)
{
    OOtexture *tex = hashtable_lookup(gOOtexDict, key);
    if (tex != NULL) {
        glBindTexture(GL_TEXTURE_2D, tex->texId);
        return 0;
    }
    
    return -1;
}

GLuint
ooTexNum(const char *key)
{
    OOtexture *tex = hashtable_lookup(gOOtexDict, key);
    
    if (tex != NULL) {
        return tex->texId;
    }
    
    return 0;
}

int
ooTexUnload(const char *key)
{
    OOtexture *tex = hashtable_lookup(gOOtexDict, key);
    
    if (tex == NULL) {
        return -1;
    }
    
    hashtable_remove(gOOtexDict, key);
    glDeleteTextures(1, &tex->texId);

    free(tex->data);
    free(tex);
    
    return 0;
}

OOtexture*
ooTexGet(const char *key)
{
    return hashtable_lookup(gOOtexDict, key);
}

