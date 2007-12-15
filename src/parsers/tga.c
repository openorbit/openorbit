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



#include "tga.h"
#include <stdio.h>
#include <sysexits.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TGA_HEADER_SIZE 18

int
tga_read_header(tga_header_t *head, FILE *file)
{
    fseek(file, 0L, SEEK_SET);
    
    head->id_len = fgetc(file) & 0xff;
    head->col_map_type = fgetc(file) & 0xff;
    head->img_type = (tga_img_type_t) fgetc(file) & 0xff;
    
    int a = fgetc(file);
    int b = fgetc(file);
    head->col_map_spec.first_entry_index = a | (b << 8);
    a = fgetc(file);
    b = fgetc(file);
    head->col_map_spec.col_map_len = a | (b << 8);
    head->col_map_spec.col_map_entry_size = fgetc(file) & 0xff;

    a = fgetc(file);
    b = fgetc(file);
    head->img_spec.x_orig = a | (b << 8);
    a = fgetc(file);
    b = fgetc(file);
    head->img_spec.y_orig = a | (b << 8);
    a = fgetc(file);
    b = fgetc(file);
    head->img_spec.width = a | (b << 8);
    a = fgetc(file);
    b = fgetc(file);
    head->img_spec.height = a | (b << 8);
    head->img_spec.depth = fgetc(file);
    
    a = fgetc(file);
    head->img_spec.origin = (a >> 4) & 3;
    head->img_spec.alpha_bits = a & 0x0f;
    
    if ((head->img_type != TGA_UNCOMP_TRUE_COL_IMG) && (head->img_type != TGA_RLE_TRUE_COL_IMG)) {
        warnx("tga loader supports only true colour files");
        return -1;
    }
    if (! (( (head->img_spec.alpha_bits == 0) && (head->img_spec.depth == 24) )
        || ( (head->img_spec.alpha_bits == 8) && (head->img_spec.depth == 32) )) ) {
        warnx("tga loader %d bit pixels with %d bit alpha is an unsupported format",
              head->img_spec.depth, head->img_spec.alpha_bits);
        return -1;
    }
    if ((head->col_map_spec.first_entry_index != 0) || (head->col_map_spec.col_map_len != 0)
        || (head->col_map_spec.col_map_entry_size != 0)) {
        warnx("tga file is true colour, but colour map specs are not zero");
        return -1;
    }
    
    return 0;
}


int
tga_read_file(tga_image_t *img, FILE *file)
{
    tga_header_t header;
    
    if (tga_read_header(&header, file)) {
        return -1;
    }

    // jump to start of image data
    fseek(file, header.id_len + TGA_HEADER_SIZE, SEEK_SET);
    
    size_t pixel_size = header.img_spec.depth >> 3; // depth / 8
    size_t n_pixels = header.img_spec.height * header.img_spec.width;
    size_t uncomp_bitmap_bytes = n_pixels * pixel_size;
    void *data = malloc(uncomp_bitmap_bytes);
    if (data == NULL) {
        err(EX_OSERR, "malloc failed to allocate %d bytes", uncomp_bitmap_bytes);
        return -1;
    }
    
    // these are local 
    size_t len;
    int i = 0;
    uint8_t buf[4];
    
    switch (header.img_type) {
    case TGA_UNCOMP_TRUE_COL_IMG:
        len = fread(data, pixel_size, n_pixels, file);
        if (len != n_pixels) {
            warnx("tga loader only read %d pixels, but expected %d pixels", len, n_pixels);
            free(data);
            return -1;
        }
        break;
    case TGA_RLE_TRUE_COL_IMG:
        while (i < n_pixels) {
            int packet_head = fgetc(file);
            int rep_count = (packet_head & 0x7f) + 1;
            if (packet_head & 0x80) {
            // RLE packet
                fread(buf, pixel_size, 1, file);
                for (int j = 0 ; j < rep_count ; j ++) {
                    *(uint8_t*)(data + i*pixel_size + 0) = buf[0];
                    *(uint8_t*)(data + i*pixel_size + 1) = buf[1];
                    *(uint8_t*)(data + i*pixel_size + 2) = buf[2];
                    if (pixel_size == 4) *(uint8_t*)(data + i*pixel_size + 3) = buf[3];
                    i ++;
                }
            } else {
                // raw packet
                size_t len = fread(data+i*pixel_size, pixel_size, rep_count, file);
                i += len;
            }
        } 
        break;
    default:
        assert(0);
    }
    
    img->header = header;
    img->data = data;
    
    return 0;
}
