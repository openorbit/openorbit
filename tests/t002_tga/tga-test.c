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
#include <check.h>

#include "tga.h"

static char *rle_file_name = NULL;
static char *uncomp_file_name = NULL;

START_TEST(test_tga_uncomp)
{
    FILE *uncompFile = fopen(uncomp_file_name, "r"); 
    
    assert(uncompFile);
    
    tga_image_t uncomp;
    int resUncomp = tga_read_file(&uncomp, uncompFile);
    
    assert(resUncomp == 0);
    
    fail_unless(uncomp.header.img_type == TGA_UNCOMP_TRUE_COL_IMG,
                "Image does not appear to be a true colour image");
    fail_unless(uncomp.header.img_spec.width == (uint16_t)512,
                "Image does not have the correct width");
    fail_unless(uncomp.header.img_spec.height == (uint16_t)512,
                "Image does not have the correct height");
    fail_unless(uncomp.header.img_spec.depth == (uint8_t)24,
                "Image does not have the correct depth");
    
    uint8_t *dat = uncomp.data;
    uint16_t width = uncomp.header.img_spec.width;
    uint16_t height = uncomp.header.img_spec.height;

   
    for (int i = 0 ; i < height ; i ++) {
        int j = 0;

        /* First pixel of line */
        fail_unless(dat[i*width*3 + 0] == (uint8_t)0xff,
                    "blue stuff(0): (%x!=0xff) i = %d, j = %d",
                    dat[i*width*3 + 0] , i, 0);
            
        fail_unless(dat[i*width*3 + 1] == (uint8_t)0x00,
                    "blue stuff(1): (%x!=0x00) i = %d, j = %d",
                    dat[i*width*3 + 1] , i, 0);
                        
        fail_unless(dat[i*width*3 + 2] == (uint8_t)0x00,
                    "blue stuff(2): (%x!=0x00) i = %d, j = %d",
                    dat[i*width*3 + 2] , i, 0);

        for (j = 480 ; j < width - 21 ; j ++) {
            fail_unless(dat[i*width*3 + j*3 + 0] == (uint8_t)0xff,
                        "blue stuff(0): (%x!=0xff) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 0] , i, j);
            fail_unless(dat[i*width*3 + j*3 + 1] == (uint8_t)0x00,
                        "blue stuff(1): (%x!=0x00) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 1] , i, j);
            fail_unless(dat[i*width*3 + j*3 + 2] == (uint8_t)0x00,
                        "blue stuff(2): (%x!=0x00) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 2] , i, j);
        }
        for ( ; j < width -15 ; j ++) {
            fail_unless(dat[i*width*3 + j*3 + 0] == (uint8_t)0x33,
                        "brown stuff(0): (%x!=0x33) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 0] , i, j);
            fail_unless(dat[i*width*3 + j*3 + 1] == (uint8_t)0x66,
                        "brown stuff(1): (%x!=0x66) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 1] , i, j);
            fail_unless(dat[i*width*3 + j*3 + 2] == (uint8_t)0x99,
                        "brown stuff(2): (%x!=0x99) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 2] , i, j);            
        }
        
        /* Last pixel of line */
        fail_unless(dat[i*width*3 + 511*3 + 0] == (uint8_t)0x33,
                    "brown stuff(0): (%x!=0x33) i = %d, j = %d", 
                    dat[i*width*3 + 511*3 + 0] , i, 511);
        
        fail_unless(dat[i*width*3 + 511*3 + 1] == (uint8_t)0x66,
                    "brown stuff(1): (%x!=0x66) i = %d, j = %d",
                    dat[i*width*3 + 511*3 + 1] , i, 511);
                    
        fail_unless(dat[i*width*3 + 511*3 + 2] == (uint8_t)0x99,
                    "brown stuff(2): (%x!=0x99) i = %d, j = %d",
                    dat[i*width*3 + 511*3 + 2] , i, 511);            
        
    } 
    
    fclose(uncompFile);
    free(uncomp.data);
}
END_TEST

START_TEST(test_tga_rle)
{
    FILE *rleFile = fopen(rle_file_name, "r");
    assert(rleFile);
    
    tga_image_t rle;
    int resRle = tga_read_file(&rle, rleFile);
    
    assert(resRle == 0);
    
    fail_unless(rle.header.img_type == TGA_RLE_TRUE_COL_IMG,
                "RLE image does not appear to be a rle true colour image");
    fail_unless(rle.header.img_spec.width == (uint16_t)512,
                "Image does not have the correct width");
    fail_unless(rle.header.img_spec.height == (uint16_t)512,
                "Image does not have the correct height");
    fail_unless(rle.header.img_spec.depth == (uint8_t)24,
                "Image does not have the correct depth");
    
    uint8_t *dat = rle.data;
    
    uint16_t width = rle.header.img_spec.width;
    uint16_t height = rle.header.img_spec.height;
    
    for (int i = 0 ; i < height ; i ++) {
        int j = 0;

        /* First pixel of line */
        fail_unless(dat[i*width*3 + 0] == (uint8_t)0xff,
                    "blue stuff(0): (%x!=0xff) i = %d, j = %d",
                    dat[i*width*3 + 0] , i, 0);
            
        fail_unless(dat[i*width*3 + 1] == (uint8_t)0x00,
                    "blue stuff(1): (%x!=0x00) i = %d, j = %d",
                    dat[i*width*3 + 1] , i, 0);
                        
        fail_unless(dat[i*width*3 + 2] == (uint8_t)0x00,
                    "blue stuff(2): (%x!=0x00) i = %d, j = %d",
                    dat[i*width*3 + 2] , i, 0);


        for (j = 480 ; j < width - 21 ; j ++) {
            fail_unless(dat[i*width*3 + j*3 + 0] == (uint8_t)0xff,
                        "blue stuff(0): (%x!=0xff) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 0] , i, j);
            fail_unless(dat[i*width*3 + j*3 + 1] == (uint8_t)0x00,
                        "blue stuff(1): (%x!=0x00) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 1] , i, j);
            fail_unless(dat[i*width*3 + j*3 + 2] == (uint8_t)0x00,
                        "blue stuff(2): (%x!=0x00) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 2] , i, j);
        }
        for ( ; j < width-15 ; j ++) {
            fail_unless(dat[i*width*3 + j*3 + 0] == (uint8_t)0x33,
                        "brown stuff(0): (%x!=0x33) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 0] , i, j);
            fail_unless(dat[i*width*3 + j*3 + 1] == (uint8_t)0x66,
                        "brown stuff(1): (%x!=0x66) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 1] , i, j);
            fail_unless(dat[i*width*3 + j*3 + 2] == (uint8_t)0x99,
                        "brown stuff(2): (%x!=0x99) i = %d, j = %d",
                        dat[i*width*3 + j*3 + 2] , i, j);            
        }
        
        /* End of line pixel */
        fail_unless(dat[i*width*3 + 511*3 + 0] == (uint8_t)0x33,
                    "brown stuff(0): (%x!=0x33) i = %d, j = %d", 
                    dat[i*width*3 + 511*3 + 0] , i, 511);
        
        fail_unless(dat[i*width*3 + 511*3 + 1] == (uint8_t)0x66,
                    "brown stuff(1): (%x!=0x66) i = %d, j = %d",
                    dat[i*width*3 + 511*3 + 1] , i, 511);
                    
        fail_unless(dat[i*width*3 + 511*3 + 2] == (uint8_t)0x99,
                    "brown stuff(2): (%x!=0x99) i = %d, j = %d",
                    dat[i*width*3 + 511*3 + 2] , i, 511);            
    } 

    fclose(rleFile);
    free(rle.data);
    
}
END_TEST
Suite
*test_suite (int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Invalid number of arguments start by supplying the rle"
                " file and after that the uncompressed file");
        exit(EXIT_FAILURE);
    }
    
    rle_file_name = argv[1];
    uncomp_file_name = argv[2];
    
    Suite *s = suite_create ("TGA Test");
    
    /* Core test case */
    TCase *tc_core = tcase_create ("Core");

    tcase_add_test(tc_core, test_tga_uncomp);
    tcase_add_test(tc_core, test_tga_rle);
    tcase_set_timeout(tc_core, 10); // these tests will take some time
        
    suite_add_tcase(s, tc_core);
    
    return s;
}

