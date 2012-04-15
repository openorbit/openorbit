/*
 Copyright 2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "shader-manager.h"
#include "common/mapped-file.h"
#include "common/moduleinit.h"
#include <gencds/hashtable.h>
#include "res-manager.h"
#include "log.h"
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

static hashtable_t *shaderKeyMap = NULL;

typedef struct SGshaderinfo {
  GLuint ident;
} SGshaderinfo;

MODULE_INIT(shadermanager, NULL)
{
  ooLogTrace("initialising 'shadermanager' module");
  shaderKeyMap = hashtable_new_with_str_keys(64);
}

void
sgLoadAllShaders(void)
{
  const char *path = ooResGetPath("shaders");

  DIR *dir = opendir(path);
  if (dir) {
    char fullpath[PATH_MAX];
    struct dirent *entry = NULL;
    while ((entry = readdir(dir))) {
      if (entry->d_name[0] == '.') continue; // ignore parent and hidden dirs

      strcpy(fullpath, "shaders/");
      strcat(fullpath, entry->d_name);
      sgLoadProgram(entry->d_name, fullpath, fullpath, fullpath);
    }

    closedir(dir);
  }
}

GLuint
sgLoadProgram(const char *key,
              const char *vspath,
              const char *fspath,
              const char *gspath)
{
  assert(key != NULL);
  assert(vspath != NULL);
  assert(fspath != NULL);

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
        GLint logLen = 0;
        GLint retrievedLen = 0;

        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLen);

        char *tmp = malloc(logLen);

        glGetShaderInfoLog(shaderId, logLen, &retrievedLen, tmp);
        fputs(tmp, stderr);

        free(tmp);

        // No globfree as this is a fatal error
        ooLogFatal("vertex shader '%s' did not compile", shaders.gl_pathv[i]);
      }
      glAttachShader(shaderProgram, shaderId);
      ooLogTrace("loaded vertex shader '%s'", shaders.gl_pathv[i]);
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
        GLint logLen = 0;
        GLint retrievedLen = 0;

        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLen);

        char *tmp = malloc(logLen);

        glGetShaderInfoLog(shaderId, logLen, &retrievedLen, tmp);
        fputs(tmp, stderr);

        free(tmp);

        // No globfree as this is a fatal error
        ooLogFatal("fragment shader '%s' did not compile", shaders.gl_pathv[i]);
      }
      glAttachShader(shaderProgram, shaderId);
      ooLogTrace("loaded fragment shader '%s'", shaders.gl_pathv[i]);
    }
    globfree(&shaders);
  }

  // Ignore geometry shaders for now...

  glLinkProgram(shaderProgram);
  GLint linkStatus = 0;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    GLint logLen = 0;
    GLint retrievedLen = 0;

    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLen);

    char *tmp = malloc(logLen);

    glGetProgramInfoLog(shaderProgram, logLen, &retrievedLen, tmp);
    fputs(tmp, stderr);

    free(tmp);

    ooLogFatal("shader linking did not succeed");
  }
  ooLogInfo("shader program '%s' succesfully linked", key);

  if (!didLoadVertexShader && !didLoadFragShader && !didLoadGeoShader) {
    ooLogInfo("no shaders found for '%s'", key);
    glDeleteProgram(shaderProgram);
    hashtable_remove(shaderKeyMap, key); // Memoize if loaded again

    return 0;
  }
  return shaderProgram;
}

GLuint
sgGetProgram(const char *key)
{
  SGshaderinfo *tmp = hashtable_lookup(shaderKeyMap, key);
  if (tmp) return tmp->ident;
  else ooLogWarn("no such shader '%s'", key);
  return 0;
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

static const char* param_names[SG_PARAM_COUNT] = {
  [SG_VERTEX0] = "Position",
  [SG_TEX_COORD0] = "TexCoord[0]",
  [SG_COLOR0] = "Color",
  [SG_TEX0] = "Tex[0]",
  [SG_TEX1] = "Tex[1]",
  [SG_TEX2] = "Tex[2]",
  [SG_TEX3] = "Tex[3]",
  [SG_LIGHT0] = "Light[0]",
  [SG_LIGHT1] = "Light[1]",
  [SG_LIGHT2] = "Light[2]",
  [SG_LIGHT3] = "Light[3]",
  [SG_MODELVIEW] = "ModelView",
  [SG_PROJECTION] = "Projection",
};

GLint
sgGetLocationForParam(GLuint program, sg_param_id_t param)
{
  GLint loc = glGetUniformLocation(program, param_names[param]);
  return loc;
}

void
sgSetShaderTex(GLuint program, sg_param_id_t param, GLuint tex)
{
  assert(SG_TEX0 <= param && param <= SG_TEX3);
  glActiveTexture(GL_TEXTURE0 + param - SG_TEX0);
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint loc = sgGetLocationForParam(program, param);
  glUniform1i(loc, param - SG_TEX0);
}
