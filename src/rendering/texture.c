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
#include "texture.h"
#include "parsers/tga.h"


texture_t
*load_texture(const char *name)
{
    tga_image_t img;
    FILE *file = fopen(name, "r");
    
    if (!file) {
        fprintf(stderr, "Can not open %s for reading\n", name);
        return NULL;
    }

    texture_t *tex = malloc(sizeof(texture_t));
    if (!tex) {
        fprintf(stderr, "Can not allocate memory for texture %s\n", name);
        fclose(file);
        return NULL;
    }
    
    if (!tga_read_file(&img, file)) {
        if ((img.header.img_type != TGA_UNCOMP_TRUE_COL_IMG)
            && (img.header.img_type != TGA_RLE_TRUE_COL_IMG)) {
            
            fprintf(stderr, "Does not support B/W or CLUTted textures\n");
            free(tex);
            free(img.data);
            fclose(file);
            return NULL;
        }

        if ((img.header.img_spec.depth == 32) && (img.header.img_spec.alpha_bits == 8)) {
            tex->type = TEX_RGBA;
        } else if ((img.header.img_spec.depth == 24) && (img.header.img_spec.alpha_bits == 0)) {
            tex->type = TEX_RGB;
        } else {
            fprintf(stderr, "%s: Texture of unsupported type\n", name);
            free(tex);
            free(img.data);
            fclose(file);
            return NULL;
        }

        tex->width = img.header.img_spec.width;
        tex->height = img.header.img_spec.height;
        
        // Unfortiounatelly, TGAs are ABGR/BGR, so we need to byte swap these on loads
        tex->data = img.data;
        for (int i = 0 ; i < tex->height ; i ++) {
            for (int j = 0 ; j < tex->width ; j ++) {
                if (tex->type = TEX_RGBA) {
                    uint8_t *pixel = (uint8_t*)(tex->data) + (i * tex->width + j) * 4;
                    uint8_t tmp[4];
                    tmp[0] = pixel[0];
                    tmp[1] = pixel[1];
                    tmp[2] = pixel[2];
                    tmp[3] = pixel[3];
                    
                    pixel[0] = tmp[3];
                    pixel[1] = tmp[2];
                    pixel[2] = tmp[1];
                    pixel[3] = tmp[0];                    
                } else {
                    uint8_t *pixel = (uint8_t*)(tex->data) + (i * tex->width + j) * 3;
                    uint8_t tmp[3];
                    tmp[0] = pixel[0];
                    tmp[1] = pixel[1];
                    tmp[2] = pixel[2];
                    
                    pixel[0] = tmp[2];
                    pixel[1] = tmp[1];
                    pixel[2] = tmp[0];
                }
            }
        }
    } else {
        fprintf(stderr, "Not a valid TGA file\n");
        free(tex);
        free(img.data);
        fclose(file);
        return NULL;
    }
    
    return tex;
}

void
destroy_texture(texture_t *texture)
{
    if (texture) {
        free(texture->data);
        free(texture);
    }
}
