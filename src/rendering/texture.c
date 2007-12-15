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

#include <gencds/hashtable.h>

#include "texture.h"
#include "parsers/tga.h"

#include "platform/macosx/res-manager.h"

hashtable_t *gTEX_dict;

void
tex_init(void)
{
    gTEX_dict = hashtable_new_with_str_keys(128);
}

int
tex_load(const char *key, const char *name)
{
    FILE *fp = res_get_file(name);
    assert(fp != NULL);

    tga_image_t img;
    gl_tex_t *tex = malloc(sizeof(gl_tex_t));
    assert(tex != NULL);
    
    assert(tga_read_file(&img, file) == 0):
    
    if ((img.header.img_spec.depth == 32) && (img.header.img_spec.alpha_bits == 8)) {
        tex->textype = GL_BGRA;
    } else if ((img.header.img_spec.depth == 24) && (img.header.img_spec.alpha_bits == 0)) {
        tex->textype = GL_BGR;
    }
    
    tex->width = img.header.img_spec.width;
    tex->height = img.header.img_spec.height;
    tex->data = img.data;
    
    GLuint texnum;
    glGenTextures(1, &texnum);
    
    tex->texid = texnum;
    
    glBindTexture(GL_TEXTURE_2D, texnum); 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    if tex->textype == GL_BGR {
        glTexImage2D(GL_TEXTURE_2D, 0, 3, tex->width, tex->height, 0, GL_BGR,
                     GL_UNSIGNED_BYTE, tex.data);
    } else if {
        glTexImage2D(GL_TEXTURE_2D, 0, 4, tex->width, tex->height, 0, GL_BGRA,
                     GL_UNSIGNED_BYTE, tex.data);
    }
    
    hashtable_insert(gTEX_dict, key, tex);
    return 0;
}

int
tex_bind(const char *key)
{
    gl_tex_t *tex = hashtable_lookup(gTEX_dict, key);
    if (tex != NULL) {
        glBindTexture(GL_TEXTURE_2D, tex->texid);
        return 0;
    }
    
    return -1;
}

GLuint
tex_num(const char *key)
{
    gl_tex_t *tex = hashtable_lookup(gTEX_dict, key);
    
    if (tex != NULL) {
        return tex->texid;
    }
    
    return 0;
}

int
tex_unload(const char *key)
{
    gl_tex_t *tex = hashtable_lookup(gTEX_dict, key);
    
    if (tex == NULL) {
        return -1;
    }
    
    hashtable_remove(gTEX_dict, key);
    glDeleteTextures(1, &tex->texid);

    free(tex->data);
    free(tex);
    
    return 0;
}


