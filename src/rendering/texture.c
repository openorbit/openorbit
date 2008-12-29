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
    created by the Initial Developer are Copyright (C) 2006 the
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
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
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

