/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

/*! Computes area tables for a given model and varying angles of attack and
    slew.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#include <GLUT/GLUT.h>
#else
#include <GL3/gl3.h>
#include <GL3/glut.h>
#endif


static struct {
  int width;
  int height;
} view = {
  .width = 640,
  .height = 480,
};

static struct {
  char *model_file;
  char *data_file;
} opts = {
  NULL, NULL,
};


void
write_results(void)
{

}

void
print_results(void)
{

}

void
start_calculations(void)
{

}


void
display(void)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glutSwapBuffers();
}
void
reshape(GLint width, GLint height)
{
  view.width = width;
  view.height = height;
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // TODO: Fix viewport size
  gluOrtho2D(-10.0, 10.0, -10.0, 10.0);
  glMatrixMode(GL_MODELVIEW);
}

void
keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q':
    exit(0);
  case 'w':
    write_results();
    break;
  case 'p':
    print_results();
    break;
  case 'c':
    start_calculations();
    break;
  default:
    // Ignore
    break;
  }
}

void
mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
    }
  }
}

void
mouse_move(int x, int y)
{
}

void
usage(void)
{
  printf("dragarea: computes the area for different angles of attack/slew\n"
         "usage:\n"
         "  dragarea -m model.ac -f model.dat\n");
}

void
parse_args(int argc, char **argv)
{
  int ch;
  while ((ch = getopt(argc, argv, "m:f:")) != -1) {
    switch (ch) {
    case 'm':
      opts.model_file = strdup(optarg);
      break;
    case 'f':
      opts.data_file = strdup(optarg);
      break;
    case '?':
    default:
      usage();
      exit(0);
    }
  }

  if (opts.model_file == NULL) {
    usage();
    exit(0);
  }
}

int
main(int argc, char **argv)
{
  parse_args(argc, argv);

  glutInit(&argc, argv);
  glutInitWindowSize(view.width, view.height);
  glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow ("Drag Area Calculator");


  // Callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(mouse_move);

  glutMainLoop();

  return 0;
}
