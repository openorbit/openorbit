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

#ifndef __CAMERA_H__
#define __CAMERA_H__
#ifdef __cplusplus
extern "C" {
#endif 


#include <math/types.h>

#include <stdbool.h>

typedef enum {
    CAM_FREE,
    CAM_ORBITING,
    CAM_CHASE
} camera_type_t;

typedef struct {
    camera_type_t type;
    union {
        struct {
            vector_t p;   // has its own position
            quaternion_t rq;
        } free_cam;
        
        struct {
            vector_t *p;   // position of center object
            scalar_t d;      // distance between object and camera
            angle_t ra;
            angle_t dec;
        } orbiting_cam;
        
        struct {
            vector_t *p;  // pos of chased object
            vector_t d;   // relative position to object
        } chase_cam;
    } u;
} camera_t;

bool init_cam(void);

/*! cam_set_free
    \param p
    \param q
 */
void cam_set_free(vector_t p, quaternion_t q);

/*! cam_set_polar
    \param tgt
    \param len
    \param ra
    \param dec
 */
void cam_set_polar(vector_t tgt, scalar_t len, scalar_t ra, scalar_t dec);

/* Camera handling functions,  */
void cam_move_forward(camera_t *cam, scalar_t distance);
void cam_move_back(camera_t *cam, scalar_t distance);
void cam_move_left(camera_t *cam, scalar_t distance);
void cam_move_right(camera_t *cam, scalar_t distance);
void cam_move_up(camera_t *cam, scalar_t distance);
void cam_move_down(camera_t *cam, scalar_t distance);
void cam_rotate_alpha(camera_t *cam, angle_t ang);
void cam_rotate_beta(camera_t *cam, angle_t ang);
void cam_rotate_gamma(camera_t *cam, angle_t ang);

/* Camera actions, registered as action handlers */
void cam_move_forward_button_action(void);
void cam_move_back_button_action(void);
void cam_move_left_button_action(void);
void cam_move_right_button_action(void);
void cam_move_up_button_action(void);
void cam_move_down_button_action(void);
void cam_roll_left_button_action(void);
void cam_roll_right_button_action(void);
void cam_yaw_left_button_action(void);
void cam_yaw_right_button_action(void);
void cam_pitch_down_button_action(void);
void cam_pitch_up_button_action(void);

#ifdef __cplusplus
}
#endif 

#endif
