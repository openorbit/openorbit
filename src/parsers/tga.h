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

/*!
    \file tga.h
    \brief Functions, enums and structs used when dealing with TGA image files.
    
    The TGA functions allows for the loading of uncompressed and RLE compressed
    TGA files. The implementation allow for a subset of the TGA standard to be
    handeled. The requrements for handeling a TGA file
    are the following:
    <ul>
        <li />It must be a truecolour TGA file.
        <li />The depth of the image data must be either of:
        <ul>
            <li />24 bits when NO alpha channel is present
            <li />32 bits when an 8 bit alpha channel is present
        </ul>
    </ul>
    All other combinations are illegal.
*/

#ifndef TGA_H__
#define TGA_H__
#ifdef __cplusplus
extern "C" {
#endif 

#include <stdint.h>
#include <stdio.h>

/*!
    \brief   The constants representing the type of the TGA file's image data.
    
    The tga_img_type_t enum supplies constants that can be used to check what
    kind of TGA image data that has been loaded. Please note that the
    @link tga_read_file tga_read_file @/link will decompress any RLE compressed
    data. At the moment only TGA_UNCOMP_TRUE_COL_IMG and TGA_RLE_TRUE_COL_IMG
    has any meaning since these represent the only TGA files that the
    tga_read_file function can load.
*/
typedef enum {
    TGA_NO_IMG_DATA = 0,
    TGA_UNCOMP_COM_MAPPED_IMG = 1,
    TGA_UNCOMP_TRUE_COL_IMG = 2, //!< The tga is an uncompressed truecolour TGA.
    TGA_UNCOMP_BW_IMG = 3,
    TGA_RLE_COL_MAPPED_IMG = 9,
    TGA_RLE_TRUE_COL_IMG = 10, //!< The tga is an RLE compressed truecolour TGA.
    TGA_RLE_BW_IMG = 11
} tga_img_type_t;

/*!
    \brief   Describes the CLUT tables.
    
    The structure describes the TGA colour maps. Since the current TGA loader
    only supports truecolour all fields shall be zero.
*/
typedef struct {
    uint16_t first_entry_index;
    uint16_t col_map_len;
    uint8_t col_map_entry_size;
} tga_col_map_spec_t;

/*!
    \brief  Describes the origin of the file.
    
    The tga_origin_t enum is used to describe the origin of the TGA image data.
    This should be used in order map in the image to the correct u-v coordinates.
*/
typedef enum {
    TGA_BOTTOM_LEFT = 0, //!< The TGA starts at the bottom left corner.
    TGA_BOTTOM_RIGHT = 1, //!< The TGA starts at the bottom right corner.
    TGA_TOP_LEFT = 2, //!< The TGA starts at the top left corner.
    TGA_TOP_RIGHT = 3 //!< The TGA starts at the top right corner.
} tga_origin_t;

/*!
    \brief   Metadata describing the image data. 
    
    The tga_img_spec_t structure stores most of the interesting image
    information such as width, height and depth.            
*/
typedef struct {
    uint16_t x_orig; //!< The screen x-coord where the image is to be drawn,
                     //!< this is typically unused and set to zero
    uint16_t y_orig; //!< The screen y-coord where the image is to be drawn,
                     //!< this is typically unused and set to zero.
    uint16_t width;  //!< The width of the image in pixels.
    uint16_t height; //!< The height of the image in pixels.
    uint8_t depth;   //!< Bits per pixel, only 24 and 32 are allowed values.
    tga_origin_t origin; //!< The corner that the image data starts from.
                         //!< This should be queried in order to find out
                         //!< wether one needs to flip the image's u-v coords
                         //!< when used as a texture.
    uint8_t alpha_bits; //!< Bits used by the alpha channel. The only allowed
                        //!< values at the moment are 0 and 8.
} tga_img_spec_t;

/*!
    @typedef 
    @abstract   (description)
    @discussion Discussion forthcoming
    @field      id_len 
    @field      col_map_type
    @field      img_type
    @field      col_map_spec
    @field      img_spec
*/
typedef struct {
    uint8_t id_len;             // n bytes in ID-field
    uint8_t col_map_type;       // 0 = no col mal, 1 col map embedded in file
    tga_img_type_t img_type;
    tga_col_map_spec_t col_map_spec;
    tga_img_spec_t img_spec;
} tga_header_t;

/*!
    \brief   A loaded TGA image.
    
    The @link tga_image_t tga_image_t @/link struct stores all interesting
    information on a loaded TGA file. See @link tga_header_t tga_header_t @/link
    for more information.
*/
typedef struct {
    tga_header_t header; //!< The header field describes the TGA file. From this
                         //!< a number of properties on both the file and
                         //!< the loaded data can be deduced.
    void *data; //!< The decompressed bitmap image data. The data is loaded
                //!< into ABGR or BGR format depending on the availability of
                //!< an alpha channel.
} tga_image_t;

/*!
    \brief   Reads the header of a TGA file.
    
    The tga_read_header reads the header of a TGA file. The function is usually
    not necisary to use by one self, since it is called by tga_read_file
    implicitly, but can be useful if you want to take decisions depending on the
    file size and other properties but do not wish to load the file before these
    decisions has been made.
    \param  head A pointer to a tga_header_t struct where the header data is to
                 be stored. 
    \param  file A FILE pointer to the TGA file. 
    \return The function returns 0 on success, all other values should be
            treated as if the function failed.
*/
int tga_read_header(tga_header_t *head, FILE *file);

/*!
    \brief   Reads a TGA formatted graphics file.
    
    The tga_read_file function reads a TGA formatted image from a specified
    file. The function allows you to open a TGA file (with or without RLE
    compression) in 24 bit true colour format (with an optional alpha channel),
    thus the loaded data will have either 24 or 32 bit pixels. This pixels are
    stored in either ABGR or BGR format depending on the inclusion of an alpha
    channel or not. Please be advised that the function will malloc space where
    the bitmap data is to be stored that YOU are responsible for cleaning up.
    This is done by calling free(tga_image_t.data).
    \param img A pointer to a tga_image_t struct where the decoded data is to
               be stored. 
    \param file A FILE pointer to the tga file.
    \return The function returns 0 on success, all other values should be
            treated as if the function failed.
*/
int tga_read_file(tga_image_t *img, FILE *file);

#ifdef __cplusplus
}
#endif 

#endif /* ! __TGA_H__ */
