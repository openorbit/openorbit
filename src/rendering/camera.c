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
#include <tgmath.h>

#include "SDL_opengl.h"

#include "camera.h"
#include "io-manager.h"
#include "math/linalg.h"
#include "math/quaternions.h"
#include "math/constants.h"

// TODO: Remove global camera
static camera_t *gCam = NULL;
static scalar_t gDist = 0.1f;
static angle_t gRot = 0.1f;


bool
init_cam(void)
{
    gCam = calloc(1, sizeof(camera_t));// TODO: Remove global camera
    gCam->type = CAM_FREE;
    V_SET(gCam->u.free_cam.p, 0.0f, 0.0f, 0.0f, 0.0f);
    
    axis_t a;
    V_SET(a, 0.0f, 0.0f, 1.0f, 0.0f);
    Q_ROT(gCam->u.free_cam.rq, a, 0.0);
    
    // Register camera actions
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
    
    return true;
}

void
cam_set_free(vector_t p, quaternion_t q)
{
    matrix_t m;
    vector_t up, rup, dir, rdir;
    V_SET(up, 0.0f, 1.0f, 0.0f, 0.0f);
    V_SET(dir, 0.0f, 0.0f, -1.0f, 0.0f);
    
    Q_M_CONVERT(m, q);
    M_V_MUL(rup, m, up);
    M_V_MUL(rdir, m, dir);

    gluLookAt(p.s.x, p.s.y, p.s.z,
              p.s.x + rdir.s.x, p.s.y + rdir.s.y, p.s.z + rdir.s.z,
              rup.s.x, rup.s.y, rup.s.z);
}

void
cam_set_polar(scalar_t len, scalar_t ra, scalar_t dec)
{
    
}


void
cam_set_view(camera_t *cam)
{
    matrix_t m;
    vector_t up, rup, dir, rdir;
    V_SET(up, 0.0f, 1.0f, 0.0f, 0.0f);
    V_SET(dir, 0.0f, 0.0f, -1.0f, 0.0f);
    
    switch (cam->type) {
        case CAM_FREE:
            Q_M_CONVERT(m, cam->u.free_cam.rq);
            M_V_MUL(rup, m, up);
            M_V_MUL(rdir, m, dir);

            gluLookAt(cam->u.free_cam.p.a[0], cam->u.free_cam.p.a[1], cam->u.free_cam.p.a[2],
                      cam->u.free_cam.p.a[0] + rdir.a[0],
                      cam->u.free_cam.p.a[1] + rdir.a[1],
                      cam->u.free_cam.p.a[2] + rdir.a[2],
                      rup.a[0], rup.a[1], rup.a[2]);            
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
cam_hack(void)
{
    cam_set_view(gCam);
}

/* Camera handling functions,  */
void
cam_move_forward(camera_t *cam, scalar_t distance)
{
    vector_t v, vb;
    matrix_t rm;
    V_SET(vb, 0.0, 0.0, 1.0, 0.0);
    
    switch (cam->type) {
    case CAM_FREE:
        Q_M_CONVERT(rm, cam->u.free_cam.rq);
        M_V_MUL(v, rm, vb);
        
        V_S_MUL(vb, v, distance);
        V_SUB(cam->u.free_cam.p, cam->u.free_cam.p, vb);
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
    V_SET(vb, 0.0, 0.0, 1.0, 0.0);
    
    switch (cam->type) {
    case CAM_FREE:
        Q_M_CONVERT(rm, cam->u.free_cam.rq);
        M_V_MUL(v, rm, vb);
        
        V_S_MUL(vb, v, distance);
        V_ADD(cam->u.free_cam.p, cam->u.free_cam.p, vb);
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
    V_SET(vr, 1.0, 0.0, 0.0, 0.0);

    switch (cam->type) {
    case CAM_FREE:
        Q_M_CONVERT(rm, cam->u.free_cam.rq);
        M_V_MUL(v, rm, vr);
        
        V_S_MUL(vr, v, distance);
        V_SUB(cam->u.free_cam.p, cam->u.free_cam.p, vr);
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
    V_SET(vr, 1.0, 0.0, 0.0, 0.0);
    
    switch (cam->type) {
    case CAM_FREE:
        Q_M_CONVERT(rm, cam->u.free_cam.rq);
        M_V_MUL(v, rm, vr);
        
        V_S_MUL(vr, v, distance);
        V_ADD(cam->u.free_cam.p, cam->u.free_cam.p, vr);
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
    V_SET(vup, 0.0, 1.0, 0.0, 0.0);

    switch (cam->type) {
        case CAM_FREE:
            Q_M_CONVERT(rm, cam->u.free_cam.rq);
            M_V_MUL(v, rm, vup);
            
            V_S_MUL(vup, v, distance);
            V_ADD(cam->u.free_cam.p, cam->u.free_cam.p, vup);
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
    V_SET(vup, 0.0, 1.0, 0.0, 0.0);
    switch (cam->type) {
        case CAM_FREE:
            Q_M_CONVERT(rm, cam->u.free_cam.rq);
            M_V_MUL(v, rm, vup);
            
            V_S_MUL(vup, v, distance);
            V_SUB(cam->u.free_cam.p, cam->u.free_cam.p, vup);
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
    
    V_SET(side, S_CONST(1.0), S_CONST(0.0), S_CONST(0.0), S_CONST(0.0));
    
    
    switch (cam->type) {
        case CAM_FREE:
            Q_M_CONVERT(mrot, cam->u.free_cam.rq);
            M_V_MUL(rotside, mrot, side);

            Q_ROT(rot, side, ang);
            V_CPY(rq, cam->u.free_cam.rq);
            
            Q_MUL(cam->u.free_cam.rq, rq, rot);
            Q_NORMALISE(cam->u.free_cam.rq);
            
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
    
    V_SET(up, S_CONST(0.0), S_CONST(1.0), S_CONST(0.0), S_CONST(0.0));
    
    switch (cam->type) {
        case CAM_FREE:            
            Q_M_CONVERT(mrot, cam->u.free_cam.rq);
            M_V_MUL(rotup, mrot, up);

            Q_ROT(rot, up, ang);
            V_CPY(rq, cam->u.free_cam.rq);
        
            Q_MUL(cam->u.free_cam.rq, rq, rot);
            Q_NORMALISE(cam->u.free_cam.rq);
            
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
    
    V_SET(fwd, S_CONST(0.0), S_CONST(0.0), S_CONST(-1.0), S_CONST(0.0));

    switch (cam->type) {
        case CAM_FREE:
            Q_M_CONVERT(mrot, cam->u.free_cam.rq);
            M_V_MUL(rotfwd, mrot, fwd);

            Q_ROT(rot, fwd, ang);
            V_CPY(rq, cam->u.free_cam.rq);
        
            Q_MUL(cam->u.free_cam.rq, rq, rot);
            Q_NORMALISE(cam->u.free_cam.rq);
            
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
cam_move_forward_button_action(void)
{
    cam_move_forward(gCam, gDist);
}

void
cam_move_back_button_action(void)
{
    cam_move_back(gCam, gDist);    
}
void
cam_move_left_button_action(void)
{
    cam_move_left(gCam, gDist);
}
void
cam_move_right_button_action(void)
{
    cam_move_right(gCam, gDist);
}
void
cam_move_up_button_action(void)
{
    cam_move_up(gCam, gDist);
}
void
cam_move_down_button_action(void)
{
    cam_move_down(gCam, gDist);
}
void
cam_roll_left_button_action(void)
{
    cam_rotate_gamma(gCam, -gRot);
}
void
cam_roll_right_button_action(void)
{
    cam_rotate_gamma(gCam, gRot);    
}

void
cam_yaw_left_button_action(void)
{
    cam_rotate_beta(gCam, gRot);
}

void
cam_yaw_right_button_action(void)
{
    cam_rotate_beta(gCam, -gRot);
}

void
cam_pitch_down_button_action(void)
{
    cam_rotate_alpha(gCam, -gRot);
}


void
cam_pitch_up_button_action(void)
{
    cam_rotate_alpha(gCam, gRot);
}
