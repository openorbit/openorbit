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
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

static hashtable_t *shaderKeyMap = NULL;


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

SGshader*
sgLoadProgram(const char *key,
              const char *vspath,
              const char *fspath,
              const char *gspath)
{
  assert(key != NULL);
  assert(vspath != NULL);
  assert(fspath != NULL);


  SGshader *tmp = hashtable_lookup(shaderKeyMap, key);
  if (tmp) return tmp;


  SGshader *si = malloc(sizeof(SGshader));

  GLuint shaderProgram = glCreateProgram();
  si->shaderId = shaderProgram;
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
  glBindAttribLocation(shaderProgram, SG_VERTEX_LOC, SG_VERTEX_NAME);
  glBindAttribLocation(shaderProgram, SG_NORMAL_LOC, SG_NORMAL_NAME);
  glBindAttribLocation(shaderProgram, SG_COLOUR_LOC, SG_COLOUR_NAME);
  glBindAttribLocation(shaderProgram, SG_TEX0_COORD_LOC, SG_TEX0_COORD_NAME);
  glBindAttribLocation(shaderProgram, SG_TEX1_COORD_LOC, SG_TEX1_COORD_NAME);
  si->attribs.vertexId = SG_VERTEX_LOC;
  si->attribs.normalId = SG_NORMAL_LOC;
  si->attribs.colourId = SG_COLOUR_LOC;
  si->attribs.texCoord0Id = SG_TEX0_COORD_LOC;
  si->attribs.texCoord1Id = SG_TEX1_COORD_LOC;

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

  // After linking, we have valid uniform locations, we build up the location
  // table here.
  si->uniforms.modelViewId = sgGetLocationForParam(shaderProgram, SG_MODELVIEW);
  si->uniforms.projectionId = sgGetLocationForParam(shaderProgram,
                                                    SG_PROJECTION);
  si->uniforms.normalMatrixId = sgGetLocationForParam(shaderProgram,
                                                      SG_NORMAL_MATRIX);
  si->uniforms.materialId.ambient = sgGetLocationForParam(shaderProgram,
                                                          SG_MATERIAL_AMB);
  si->uniforms.materialId.emission = sgGetLocationForParam(shaderProgram,
                                                          SG_MATERIAL_EMIT);
  si->uniforms.materialId.diffuse = sgGetLocationForParam(shaderProgram,
                                                          SG_MATERIAL_DIFF);
  si->uniforms.materialId.specular = sgGetLocationForParam(shaderProgram,
                                                          SG_MATERIAL_SPEC);
  si->uniforms.materialId.shininess = sgGetLocationForParam(shaderProgram,
                                                            SG_MATERIAL_SHINE);
  for (int i = 0 ; i < SG_OBJ_MAX_LIGHTS ; i++) {
    si->uniforms.lightIds[i].pos =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_POS, i);
    si->uniforms.lightIds[i].ambient =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_AMB, i);
    si->uniforms.lightIds[i].diffuse =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_DIFF, i);
    si->uniforms.lightIds[i].dir =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_DIR, i);
    si->uniforms.lightIds[i].specular =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_SPEC, i);
    si->uniforms.lightIds[i].constantAttenuation =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_CONST_ATTEN, i);
    si->uniforms.lightIds[i].linearAttenuation =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_LINEAR_ATTEN, i);
    si->uniforms.lightIds[i].quadraticAttenuation =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_QUAD_ATTEN, i);
    si->uniforms.lightIds[i].globAmbient =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_MOD_GLOB_AMB, i);
  }

  for (int i = 0 ; i < SG_OBJ_MAX_TEXTURES ; i++) {
    si->uniforms.texIds[i] = sgGetLocationForParamAndIndex(shaderProgram,
                                                           SG_TEX, i);
  }


  return si;
}

SGshader*
sgGetProgram(const char *key)
{
  SGshader *tmp = hashtable_lookup(shaderKeyMap, key);
  if (tmp) return tmp;
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

SGshader*
sgShaderFromKey(const char *key)
{
  SGshader *si = hashtable_lookup(shaderKeyMap, key);
  return si;
}

static const char* param_names[SG_PARAM_COUNT] = {
  [SG_VERTEX] = "oo_Vertex",
  [SG_NORMAL] = "oo_Normal",
  [SG_TEX_COORD] = "oo_TexCoord[%u]",
  [SG_COLOR] = "oo_Color",
  [SG_TEX] = "oo_Texture[%u]",

  [SG_LIGHT] = "oo_Light[%u]",

  [SG_LIGHT_AMB] = "oo_Light[%u].ambient",
  [SG_LIGHT_POS] = "oo_Light[%u].pos",
  [SG_LIGHT_SPEC] = "oo_Light[%u].specular",
  [SG_LIGHT_DIFF] = "oo_Light[%u].diffuse",
  [SG_LIGHT_DIR] = "oo_Light[%u].dir",
  [SG_LIGHT_CONST_ATTEN] = "oo_Light[%u].constantAttenuation",
  [SG_LIGHT_LINEAR_ATTEN] = "oo_Light[%u].linearAttenuation",
  [SG_LIGHT_QUAD_ATTEN] = "oo_Light[%u].quadraticAttenuation",
  [SG_LIGHT_MOD_GLOB_AMB] = "oo_Light[%u].globAmbient",

  [SG_MATERIAL_EMIT] = "oo_Material.emission",
  [SG_MATERIAL_AMB] = "oo_Material.ambient",
  [SG_MATERIAL_DIFF] = "oo_Material.diffuse",
  [SG_MATERIAL_SPEC] = "oo_Material.specular",
  [SG_MATERIAL_SHINE] = "oo_Material.shininess",

  [SG_MODELVIEW] = "oo_ModelViewMatrix",
  [SG_PROJECTION] = "oo_ProjectionMatrix",
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
