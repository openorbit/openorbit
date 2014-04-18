/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include <check.h>

#include "../../lib/imgload/tga.h"

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
