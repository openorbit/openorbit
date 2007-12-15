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

#include "render.h"

#include <stdbool.h>
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "settings.h"
#include "camera.h"
#include "sky.h"
#include "texture.h"
#include "planet.h"

static void draw_gl(void);
static void init_gl(void);
static void print_attributes(void);
static void init_attributes(void);
static void create_surface(void);

static SDL_Surface *REND_screen;


bool
init_renderer(void) {
    // Set GL context attributes
    init_attributes();
    
    // Create GL context
    create_surface();
    
    // Get GL context attributes
    print_attributes();
    
    init_gl();
    tex_init();
    return true;
}

void
render_scene(void) {
    draw_gl();
    SDL_GL_SwapBuffers();
}


static void
init_gl(void)
{
    int width, height;
    
    glMatrixMode(GL_PROJECTION);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glLoadIdentity();
	gluPerspective(/*fovy*/45.0, /*aspect*/1.33 , /*near*/0.001, /*far*/100.0);
    
    if (conf_get_int("video.width", &width)) width = 640;
    if (conf_get_int("video.height", &height)) height = 480;
    
    glViewport(0, 0, width, height);
    
//    ugly_load_and_init_of_test_texture();
}


static void
init_attributes(void)
{
    // Setup attributes we want for the OpenGL context
    
    int value;
    
    // Don't set color bit sizes (SDL_GL_RED_SIZE, etc)
    //    Mac OS X will always use 8-8-8-8 ARGB for 32-bit screens and
    //    5-5-5 RGB for 16-bit screens
    
    // Request a 16-bit depth buffer (without this, there is no depth buffer)
    value = 16;
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, value);
    
    
    // Request double-buffered OpenGL
    //     The fact that windows are double-buffered on Mac OS X has no effect
    //     on OpenGL double buffering.
    value = 1;
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, value);
}

static void
print_attributes (void)
{
    // Print out attributes of the context we created
    int nAttr;
    int i;
    
    int  attr[] = { SDL_GL_RED_SIZE, SDL_GL_BLUE_SIZE, SDL_GL_GREEN_SIZE,
        SDL_GL_ALPHA_SIZE, SDL_GL_BUFFER_SIZE, SDL_GL_DEPTH_SIZE };
    
    char *desc[] = { "Red size: %d bits\n", "Blue size: %d bits\n",
        "Green size: %d bits\n",
        "Alpha size: %d bits\n", "Color buffer size: %d bits\n", 
        "Depth bufer size: %d bits\n" };
    
    nAttr = sizeof(attr) / sizeof(int);
    
    for (i = 0; i < nAttr; i++) {
        
        int value;
        SDL_GL_GetAttribute(attr[i], &value);
        printf(desc[i], value);
    } 
}


static void
create_surface(void)
{
    Uint32 flags = 0;
    
    flags = SDL_OPENGL;
    bool fullscreen;
    int width, height;
    
    if (conf_get_bool("video.fullscreen", &fullscreen)) fullscreen = true;
    if (conf_get_int("video.width", &width)) width = 640;
    if (conf_get_int("video.height", &height)) height = 480;

    if (fullscreen) flags |= SDL_FULLSCREEN;
    // Create window
    REND_screen = SDL_SetVideoMode(width, height, 0, flags);
    if (REND_screen == NULL) {
		
        fprintf(stderr, "Couldn't set %dx%d OpenGL video mode: %s\n",
                width, height, SDL_GetError());
		SDL_Quit();
		exit(2);
	}
}


void exit_fullscreen()
{
    ;
    //SDL_surface *window =  SDL_SetVideoMode(SETTINGS.video.width,
    //                                        SETTINGS.video.height,
    //                                        0, SDL_OPENGL);
}

void enter_fullscreen()
{
    ;
    //SDL_surface *window =  SDL_SetVideoMode(SETTINGS.video.width,
    //                                        SETTINGS.video.height,
    //                                        0, SDL_OPENGL|SDL_FULLSCREEN);
}


static void
draw_gl(void)
{
    extern star_list_t *gSKY_stars;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // first, lets draw the nightsky, z-buffer disabled
    paint_sky(gSKY_stars);
    glLoadIdentity();
    
    // the drawing is not very well structured at the moment
    cam_move_global_camera();

    glPushMatrix();

    glEnable(GL_DEPTH_TEST);
    planet_draw_all();
    glPopMatrix();
    
    // axis, ugly, but this is a temporary hack
    glBegin(GL_LINES);
    {
        // x
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 0.0f, 0.0f);
        
        // y
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        
        // z
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);        
    }
    glEnd();
}
