/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "shader-manager.h"
#include "common/mapped-file.h"
#include "common/moduleinit.h"
#include <gencds/hashtable.h>
#include "res-manager.h"
#include "log.h"
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <gl/gl.h>
#endif

static hashtable_t *shaderKeyMap = NULL;

typedef struct SGshaderinfo {
  GLuint ident;
} SGshaderinfo;

INIT_PRIMARY_MODULE
{
  shaderKeyMap = hashtable_new_with_str_keys(64);
}

GLuint
sgLoadProgram(const char *key,
              const char *vspath,
              const char *fspath,
              const char *gspath)
{
  SGshaderinfo *tmp = hashtable_lookup(shaderKeyMap, key);
  if (tmp) return tmp->ident;

  SGshaderinfo *si = malloc(sizeof(SGshaderinfo));

  GLuint shaderProgram = glCreateProgram();
  si->ident = shaderProgram;
  hashtable_insert(shaderKeyMap, key, si); // Memoize if loaded again

  bool didLoadVertexShader = false;
  bool didLoadFragShader = false;
  bool didLoadGeoShader = false;
  if (vspath) {
    mapped_file_t mf;
    GLuint shaderId;
    char pattern[strlen(vspath)+1+9];
    strcpy(pattern, vspath);
    strcat(pattern, "/*.vertex");
    glob_t shaders = ooResGetFilePaths(pattern);

    if (shaders.gl_matchc > 0) didLoadVertexShader = true;

    for (int i = 0 ; i < shaders.gl_matchc ; ++ i) {
      mf = map_file(shaders.gl_pathv[i]);
      if (mf.fd == -1) return 0;
      shaderId = glCreateShader(GL_VERTEX_SHADER);
      GLint len = mf.fileLenght;
      glShaderSource(shaderId, 1, (const GLchar**)&mf.data, &len);
      unmap_file(&mf);
      glCompileShader(shaderId);

      GLint compileStatus = 0;
      glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
      if (compileStatus == GL_FALSE) {
        // No globfree as this is a fatal error
        ooLogFatal("vertex shader '%s' did not compile", shaders.gl_pathv[i]);
      }
      glAttachShader(shaderProgram, shaderId);
      ooLogInfo("loaded vertex shader '%s'", shaders.gl_pathv[i]);
    }
    globfree(&shaders);
  }


  if (fspath) {
    mapped_file_t mf;
    GLuint shaderId;
    char pattern[strlen(vspath)+1+11];
    strcpy(pattern, vspath);
    strcat(pattern, "/*.fragment");
    glob_t shaders = ooResGetFilePaths(pattern);

    if (shaders.gl_matchc > 0) didLoadFragShader = true;

    for (int i = 0 ; i < shaders.gl_matchc ; ++ i) {
      mf = map_file(shaders.gl_pathv[i]);
      if (mf.fd == -1) return 0;
      shaderId = glCreateShader(GL_FRAGMENT_SHADER);
      GLint len = mf.fileLenght;
      glShaderSource(shaderId, 1, (const GLchar**)&mf.data, &len);
      unmap_file(&mf);
      glCompileShader(shaderId);

      GLint compileStatus = 0;
      glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
      if (compileStatus == GL_FALSE) {
        // No globfree as this is a fatal error
        ooLogFatal("fragment shader '%s' did not compile", shaders.gl_pathv[i]);
      }
      glAttachShader(shaderProgram, shaderId);
      ooLogInfo("loaded fragment shader '%s'", shaders.gl_pathv[i]);
    }
    globfree(&shaders);
  }

  // Ignore geometry shaders for now...

  glLinkProgram(shaderProgram);
  GLint linkStatus = 0;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    ooLogFatal("shader linking did not succeed");
  }
  ooLogInfo("shader program succesfully linked");

  if (!didLoadVertexShader && !didLoadFragShader && !didLoadGeoShader) {
    ooLogInfo("no shaders found for '%s'", key);
    glDeleteProgram(shaderProgram);
    hashtable_remove(shaderKeyMap, key); // Memoize if loaded again

    return 0;
  }
  return shaderProgram;
}

void
sgEnableProgram(GLuint programId)
{
  glUseProgram(programId);
}

void
sgDisableProgram(void)
{
  glUseProgram(0); // Return to fixed functionality
}

GLuint
sgShaderFromKey(const char *key)
{
  SGshaderinfo *si = hashtable_lookup(shaderKeyMap, key);
  return si->ident;
}
