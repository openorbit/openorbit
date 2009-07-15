/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
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
    ooTexInit();
    return true;
}

static void
init_gl(void)
{
    int width, height;
    float fovy, aspect;
    
    glMatrixMode(GL_PROJECTION);
	  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glLoadIdentity();
    
    ooConfGetFloatDef("openorbit/video/gl/fovy", &fovy, 45.0f);
    ooConfGetFloatDef("openorbit/video/gl/aspect", &aspect, 1.33f);

    // Near clipping 1 cm away, far clipping 20 au away
    gluPerspective(fovy, aspect, /*near*/0.1, /*far*/149598000000.0*0.1);
    
    ooConfGetIntDef("openorbit/video/width", &width, 640);
    ooConfGetIntDef("openorbit/video/height", &height, 480);

    glViewport(0, 0, width, height);
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
    
    ooConfGetBoolDef("openorbit/video/fullscreen", &fullscreen, false);
    ooConfGetIntDef("openorbit/video/width", &width, 640);
    ooConfGetIntDef("openorbit/video/height", &height, 480);

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

