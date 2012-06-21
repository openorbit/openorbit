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
#include <openorbit/log.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
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
  // Set the output fragment name
  glBindFragDataLocation(shaderProgram, 0, "oo_FragColor");

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
  [SG_VERTEX] = "Position",
  [SG_NORMAL] = "Normal",
  [SG_TEX_COORD] = "TexCoord[%u]",
  [SG_COLOR] = "Color",
  [SG_TEX] = "Tex[%u]",

  [SG_LIGHT] = "Light[%u]",

  [SG_LIGHT_AMB] = "Light[%u].ambient",
  [SG_LIGHT_POS] = "Light[%u].pos",
  [SG_LIGHT_SPEC] = "Light[%u].specular",
  [SG_LIGHT_DIFF] = "Light[%u].diffuse",
  [SG_LIGHT_DIR] = "Light[%u].dir",
  [SG_LIGHT_CONST_ATTEN] = "Light[%u].constantAttenuation",
  [SG_LIGHT_LINEAR_ATTEN] = "Light[%u].linearAttenuation",
  [SG_LIGHT_QUAD_ATTEN] = "Light[%u].quadraticAttenuation",
  [SG_LIGHT_MOD_GLOB_AMB] = "Light[%u].globAmbient",
  
  [SG_MATERIAL_EMIT] = "Material.emission",
  [SG_MATERIAL_AMB] = "Material.ambient",
  [SG_MATERIAL_DIFF] = "Material.diffuse",
  [SG_MATERIAL_SPEC] = "Material.specular",
  [SG_MATERIAL_SHINE] = "Material.shininess",
  
  [SG_MODELVIEW] = "ModelViewMatrix",
  [SG_PROJECTION] = "ProjectionMatrix",
};


GLint
sgGetLocationForParam(GLuint program, sg_param_id_t param)
{
  GLint loc = glGetUniformLocation(program, param_names[param]);
  return loc;
}

GLint
sgGetLocationForParamAndIndex(GLuint program, sg_param_id_t param,
                              unsigned index)
{
  char locName[strlen(param_names[param])];
  snprintf(locName, sizeof(locName), param_names[param], index);
  GLint loc = glGetUniformLocation(program, locName);
  return loc;
}


void
sgSetShaderTex(GLuint program, unsigned index, GLuint tex)
{
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint loc = sgGetLocationForParamAndIndex(program, SG_TEX, index);
  glUniform1i(loc, index); // Uniform value is texture unit id
}
