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

static void draw_gl(void);
static void init_gl(void);
static void print_attributes(void);
static void init_attributes(void);
static void create_surface(void);

static SDL_Surface *REND_screen;
extern settings_t SETTINGS;

float *stars; // star coordinates
float *starColours;


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


void ugly_load_and_init_of_test_texture(void);

static void
init_gl(void)
{
    glMatrixMode(GL_PROJECTION);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glLoadIdentity();
	gluPerspective(/*fovy*/45.0, /*aspect*/1.33 , /*near*/0.00001, /*far*/100.0);
    
    glViewport(0, 0, SETTINGS.video.width, SETTINGS.video.height);
    
   // ugly_load_and_init_of_test_texture();
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
    if (SETTINGS.video.fullscreen)
        flags |= SDL_FULLSCREEN;
    
    // Create window
    REND_screen = SDL_SetVideoMode(SETTINGS.video.width, SETTINGS.video.height,
                                   0, flags);
    if (REND_screen == NULL) {
		
        fprintf(stderr, "Couldn't set %dx%d OpenGL video mode: %s\n",
                SETTINGS.video.width, SETTINGS.video.height, SDL_GetError());
		SDL_Quit();
		exit(2);
	}
}

void cam_hack(void);

#include "parsers/tga.h"
#include <assert.h>
#define MY_TGA_FILE "/Users/holm/Desktop/earth.tga"
GLUquadricObj *quadric;


void
ugly_load_and_init_of_test_texture(void)
{
    tga_image_t tex;
    FILE *file;
    file = fopen(MY_TGA_FILE, "r");
    assert(file);
    int res = tga_read_file(&tex, file);
    assert(!res);
    
    glBindTexture(GL_TEXTURE_2D, 0); 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, tex.header.img_spec.width, 
                 tex.header.img_spec.height, 0, GL_BGR, GL_UNSIGNED_BYTE, 
                 tex.data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                    GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    
    glPolygonMode(GL_FRONT,GL_FILL);
    glPolygonMode(GL_BACK,GL_LINE);

    quadric=gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);
    
}


void exit_fullscreen()
{
    SDL_surface *window =  SDL_SetVideoMode(SETTINGS.video.width,
                                            SETTINGS.video.height,
                                            0, SDL_OPENGL);
}

void enter_fullscreen()
{
    SDL_surface *window =  SDL_SetVideoMode(SETTINGS.video.width,
                                            SETTINGS.video.height,
                                            0, SDL_OPENGL|SDL_FULLSCREEN);
}


static void
draw_gl(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // first, lets draw the nightsky, z-buffer disabled
    //glDisable(GL_DEPTH_TEST);
    
    //glEnable(GL_DEPTH_TEST);
    
    // the drawing is not very well structured at the moment
    // we should do the following: 1. traverse the scene graph and move the
    // camera, 2. paint visible objects in the scene graph node, 3. there is no
    // step 3...
    cam_hack();
    
    glEnable(GL_DEPTH_TEST);

    GLfloat lightCol[] = {1.0,1.0,1.0,1.0};
    GLfloat pos[] = {100.0, 0.0, 0.0, 0.0};
    
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    
    glPushMatrix();
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
        
    static float sphere_rot = 0.0f;
    GLfloat col[] = {1.0f, 1.0f, 1.0f, 1.0f};
        
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    glColor3f(1.0f, 1.0f, 1.0f);

    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(sphere_rot, 0.0f, 0.0f, 1.0f);
    gluSphere(quadric,1,64,64);
//        sphere_rot = fmodf(sphere_rot - 0.1f, 360.0);
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
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
