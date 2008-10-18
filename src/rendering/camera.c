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
#include <stdbool.h>
#include <stdlib.h>

#include "SDL_opengl.h"
#include <vmath/vmath.h>
#include "io-manager.h"
#include "camera.h"

/* Camera actions, registered as action handlers */
void cam_move_forward_button_action(bool up, void *data);
void cam_move_back_button_action(bool up, void *data);
void cam_move_left_button_action(bool up, void *data);
void cam_move_right_button_action(bool up, void *data);
void cam_move_up_button_action(bool up, void *data);
void cam_move_down_button_action(bool up, void *data);
void cam_roll_left_button_action(bool up, void *data);
void cam_roll_right_button_action(bool up, void *data);
void cam_yaw_left_button_action(bool up, void *data);
void cam_yaw_right_button_action(bool up, void *data);
void cam_pitch_down_button_action(bool up, void *data);
void cam_pitch_up_button_action(bool up, void *data);

// TODO: Remove global camera
camera_t *gCam = NULL;
static scalar_t gDist = 0.1f;
static angle_t gRot = 0.1f;


bool
init_cam(void)
{
    gCam = calloc(1, sizeof(camera_t));// TODO: Remove global camera
    gCam->type = CAM_FREE;
	gCam->free_cam.p = v_set(0.0f, 0.0f, 0.0f, 0.0f);
	gCam->free_cam.rq = q_rot(0.0f,0.0f,1.0f,0.0f);
    
    // Register camera actions
#if 0
    io_register_button_handler("cam-fwd", cam_move_forward_button_action);        
    io_register_button_handler("cam-back", cam_move_back_button_action);
    io_register_button_handler("cam-left", cam_move_left_button_action);
    io_register_button_handler("cam-right", cam_move_right_button_action);
    io_register_button_handler("cam-up", cam_move_up_button_action);
    io_register_button_handler("cam-down", cam_move_down_button_action);
    io_register_button_handler("cam-pitch-up", cam_pitch_up_button_action);
    io_register_button_handler("cam-pitch-down", cam_pitch_down_button_action);
    io_register_button_handler("cam-yaw-left", cam_yaw_left_button_action);
    io_register_button_handler("cam-yaw-right", cam_yaw_right_button_action);
    io_register_button_handler("cam-roll-left", cam_roll_left_button_action);
    io_register_button_handler("cam-roll-right", cam_roll_right_button_action);
#endif
    ooIoRegCKeyHandler("cam-fwd", cam_move_forward_button_action);        
    ooIoRegCKeyHandler("cam-back", cam_move_back_button_action);
    ooIoRegCKeyHandler("cam-left", cam_move_left_button_action);
    ooIoRegCKeyHandler("cam-right", cam_move_right_button_action);
    ooIoRegCKeyHandler("cam-up", cam_move_up_button_action);
    ooIoRegCKeyHandler("cam-down", cam_move_down_button_action);
    ooIoRegCKeyHandler("cam-pitch-up", cam_pitch_up_button_action);
    ooIoRegCKeyHandler("cam-pitch-down", cam_pitch_down_button_action);
    ooIoRegCKeyHandler("cam-yaw-left", cam_yaw_left_button_action);
    ooIoRegCKeyHandler("cam-yaw-right", cam_yaw_right_button_action);
    ooIoRegCKeyHandler("cam-roll-left", cam_roll_left_button_action);
    ooIoRegCKeyHandler("cam-roll-right", cam_roll_right_button_action);


    return true;
}

void
cam_rotate(quaternion_t q)
{
    matrix_t m;
    vector_t up, rup, dir, rdir;
    up = v_set(0.0f, 1.0f, 0.0f, 0.0f);
    dir = v_set(0.0f, 0.0f, -1.0f, 0.0f);
    
    q_m_convert(&m, q);
    rup = m_v_mul(&m, up);
    rdir = m_v_mul(&m, dir);
    
    gluLookAt(0.0, 0.0, 0.0, rdir.x, rdir.y, rdir.z,
              rup.x, rup.y, rup.z);
}

void
cam_set_free(vector_t p, quaternion_t q)
{
    matrix_t m;
    vector_t up, rup, dir, rdir;
    up = v_set(0.0f, 1.0f, 0.0f, 0.0f);
    dir = v_set(0.0f, 0.0f, -1.0f, 0.0f);
    
    q_m_convert(&m, q);
    rup = m_v_mul(&m, up);
    rdir = m_v_mul(&m, dir);

    gluLookAt(p.x, p.y, p.z,
              p.x + rdir.x, p.y + rdir.y, p.z + rdir.z,
              rup.x, rup.y, rup.z);
}

void
cam_set_polar(vector_t tgt, scalar_t len, scalar_t ra, scalar_t dec)
{
    matrix_t m;
    vector_t cam_pos, rot_cam_pos, up, rot_up, right, rot_right;
    quaternion_t q0, q1, qr;
    
    cam_pos = v_set(0.0f, 0.0f, 1.0f, 0.0f);
    up = v_set(0.0f, 1.0f, 0.0f, 0.0f);
    right = v_set(1.0f, 0.0f, 0.0f, 1.0f);

    q0 = q_rot(0.0f, 1.0f, 0.0f, ra);
    q_m_convert(&m, q0);
    rot_right = m_v_mul(&m, right);
    q1 = q_rotv(rot_right, dec);
    
    qr = q_mul(q0, q1);
    q_m_convert(&m, qr);
    
    rot_cam_pos = m_v_mul(&m, cam_pos);
    rot_up = m_v_mul(&m, up);
    
    rot_cam_pos = v_s_mul(rot_cam_pos, len);
    
    gluLookAt(tgt.x, tgt.y, tgt.z,
              tgt.x + rot_cam_pos.x,
              tgt.y + rot_cam_pos.y,
              tgt.z + rot_cam_pos.z,
              rot_up.x, rot_up.y, rot_up.z);
}


void
cam_set_view(camera_t *cam)
{    
    switch (cam->type) {
        case CAM_FREE:
            cam_set_free(cam->free_cam.p, cam->free_cam.rq);
            break;
        case CAM_CHASE:
            break;
        case CAM_ORBITING:
            break;
        default:
            break;
    }
}
void
cam_move_global_camera(void)
{
    cam_set_view(gCam);
}

/* Camera handling functions,  */
void
cam_move_forward(camera_t *cam, scalar_t distance)
{
    vector_t v, vb;
    matrix_t rm;
    vb = v_set(0.0, 0.0, 1.0, 0.0);
    
    switch (cam->type) {
    case CAM_FREE:
        q_m_convert(&rm, cam->free_cam.rq);
        v = m_v_mul(&rm, vb);
        
        vb = v_s_mul(v, distance);
        cam->free_cam.p = v_sub(cam->free_cam.p, vb);
        break;
    case CAM_CHASE:
        break;
    case CAM_ORBITING:
        break;
    default:
        break;
    }
}

void
cam_move_back(camera_t *cam, scalar_t distance)
{
    vector_t v, vb;
    matrix_t rm;
    vb = v_set(0.0, 0.0, 1.0, 0.0);
    
    switch (cam->type) {
    case CAM_FREE:
        q_m_convert(&rm, cam->free_cam.rq);
        v = m_v_mul(&rm, vb);
        
        vb = v_s_mul(v, distance);
        cam->free_cam.p = v_add(cam->free_cam.p, vb);
        break;
    case CAM_CHASE:
        break;
    case CAM_ORBITING:
        break;
    default:
        break;
    }
}

void
cam_move_left(camera_t *cam, scalar_t distance)
{
    vector_t v, vr;
    matrix_t rm;
    vr = v_set(1.0, 0.0, 0.0, 0.0);

    switch (cam->type) {
    case CAM_FREE:
        q_m_convert(&rm, cam->free_cam.rq);
        v = m_v_mul(&rm, vr);
        
        vr = v_s_mul(v, distance);
        cam->free_cam.p = v_sub(cam->free_cam.p, vr);
        break;
    case CAM_CHASE:
        break;
    case CAM_ORBITING:
        break;
    default:
        break;
    }
}

void
cam_move_right(camera_t *cam, scalar_t distance)
{
    vector_t v, vr;
    matrix_t rm;
    vr = v_set(1.0, 0.0, 0.0, 0.0);
    
    switch (cam->type) {
    case CAM_FREE:
        q_m_convert(&rm, cam->free_cam.rq);
        v = m_v_mul(&rm, vr);
        
        vr = v_s_mul(v, distance);
        cam->free_cam.p = v_add(cam->free_cam.p, vr);
        break;
    case CAM_CHASE:
        break;
    case CAM_ORBITING:
        break;
    default:
        break;
    }
}

void
cam_move_up(camera_t *cam, scalar_t distance)
{
    vector_t v, vup;
    matrix_t rm;
    vup = v_set(0.0, 1.0, 0.0, 0.0);

    switch (cam->type) {
        case CAM_FREE:
            q_m_convert(&rm, cam->free_cam.rq);
            v = m_v_mul(&rm, vup);
            vup = v_s_mul(v, distance);
            cam->free_cam.p = v_add(cam->free_cam.p, vup);
            break;
        case CAM_CHASE:
            break;
        case CAM_ORBITING:
            break;
        default:
            break;
    }
}

void
cam_move_down(camera_t *cam, scalar_t distance)
{
    vector_t v, vup;
    matrix_t rm;
    vup = v_set(0.0, 1.0, 0.0, 0.0);
    switch (cam->type) {
        case CAM_FREE:
            q_m_convert(&rm, cam->free_cam.rq);
            v = m_v_mul(&rm, vup);
            
            vup = v_s_mul(v, distance);
            cam->free_cam.p = v_sub(cam->free_cam.p, vup);
            break;
        case CAM_CHASE:
            break;
        case CAM_ORBITING:
            break;
        default:
            break;
    }    
}

// pitch
void
cam_rotate_alpha(camera_t *cam, angle_t ang)
{
    quaternion_t rot, rq;
    matrix_t mrot;
    vector_t side, rotside;
    
    side = v_set(1.0f, 0.0f, 0.0f, 1.0f);
    
    
    switch (cam->type) {
        case CAM_FREE:
            q_m_convert(&mrot, cam->free_cam.rq);
            rotside = m_v_mul(&mrot, side);

            rot = q_rotv(rotside, ang);
            rq = cam->free_cam.rq;
            
            cam->free_cam.rq = q_mul(rq, rot);
            cam->free_cam.rq = q_normalise(cam->free_cam.rq);
            
            break;
        case CAM_CHASE:
            break;
        case CAM_ORBITING:
            break;
        default:
            break;
    }    
}

// yaw
void
cam_rotate_beta(camera_t *cam, angle_t ang)
{
    quaternion_t rot, rq;
    matrix_t mrot;
    vector_t up, rotup;
    
    up = v_set(0.0f, 1.0f, 0.0f, 1.0f);
    
    switch (cam->type) {
        case CAM_FREE:            
            q_m_convert(&mrot, cam->free_cam.rq);
            rotup = m_v_mul(&mrot, up);

            rot = q_rotv(rotup, ang);
            rq  = cam->free_cam.rq;
        
            cam->free_cam.rq = q_mul(rq, rot);
            cam->free_cam.rq = q_normalise(cam->free_cam.rq);
            
            break;
        case CAM_CHASE:
            break;
        case CAM_ORBITING:
            break;
        default:
            break;
    }
    
}

// roll
void
cam_rotate_gamma(camera_t *cam, angle_t ang)
{
    quaternion_t rot, rq;
    vector_t fwd, rotfwd;
    matrix_t mrot;
    
    fwd = v_set(0.0f, 0.0f, -1.0f, 1.0f);

    switch (cam->type) {
        case CAM_FREE:
            q_m_convert(&mrot, cam->free_cam.rq);
            rotfwd = m_v_mul(&mrot, fwd);

            rot = q_rotv(rotfwd, ang);
            rq = cam->free_cam.rq;
        
            cam->free_cam.rq = q_mul(rq, rot);
            cam->free_cam.rq = q_normalise(cam->free_cam.rq);
            
            break;
        case CAM_CHASE:
            break;
        case CAM_ORBITING:
            break;
        default:
            break;
    }        
}

/* Camera actions, registered as action handlers */
void
cam_move_forward_button_action(bool up, void *data)
{
    cam_move_forward(gCam, gDist);
}

void
cam_move_back_button_action(bool up, void *data)
{
    cam_move_back(gCam, gDist);    
}
void
cam_move_left_button_action(bool up, void *data)
{
    cam_move_left(gCam, gDist);
}
void
cam_move_right_button_action(bool up, void *data)
{
    cam_move_right(gCam, gDist);
}
void
cam_move_up_button_action(bool up, void *data)
{
    cam_move_up(gCam, gDist);
}
void
cam_move_down_button_action(bool up, void *data)
{
    cam_move_down(gCam, gDist);
}
void
cam_roll_left_button_action(bool up, void *data)
{
    cam_rotate_gamma(gCam, -gRot);
}
void
cam_roll_right_button_action(bool up, void *data)
{
    cam_rotate_gamma(gCam, gRot);    
}

void
cam_yaw_left_button_action(bool up, void *data)
{
    cam_rotate_beta(gCam, gRot);
}

void
cam_yaw_right_button_action(bool up, void *data)
{
    cam_rotate_beta(gCam, -gRot);
}

void
cam_pitch_down_button_action(bool up, void *data)
{
    cam_rotate_alpha(gCam, -gRot);
}


void
cam_pitch_up_button_action(bool up, void *data)
{
    cam_rotate_alpha(gCam, gRot);
}
