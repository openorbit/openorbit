/*
 Copyright 2010,2011,2012 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include "location.h"
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

#include "rendering/scenegraph.h"
#include "palloc.h"

struct sg_light_ids_t {
  GLint pos;
  GLint ambient;
  GLint specular;
  GLint diffuse;
  GLint dir;

  GLint constantAttenuation;
  GLint linearAttenuation;
  GLint quadraticAttenuation;
};

struct sg_material_ids_t {
  GLint emission;
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint shininess;
};

struct sg_shader_t {
  const char *name;
  GLuint shaderId;

  // Uniform IDs
  struct {
    GLint modelViewId;
    GLint projectionId;
    GLint normalMatrixId;
    GLint texIds[SG_OBJ_MAX_TEXTURES];
    GLint tex_validity[SG_OBJ_MAX_TEXTURES];
    sg_light_ids_t lightIds[SG_OBJ_MAX_LIGHTS];
    GLint globAmbient;
    sg_material_ids_t materialId;
  } uniforms;
};


static const char* param_names[SG_PARAM_COUNT] = {
  [SG_VERTEX] = "oo_Vertex",
  [SG_NORMAL] = "oo_Normal",
  [SG_TEX_COORD_0] = "oo_TexCoord0",
  [SG_TEX_COORD_1] = "oo_TexCoord0",
  [SG_TEX_COORD_2] = "oo_TexCoord0",
  [SG_TEX_COORD_3] = "oo_TexCoord0",
  [SG_COLOR] = "oo_Color",
  [SG_TEX_0] = "oo_Texture0",
  [SG_TEX_1] = "oo_Texture1",
  [SG_TEX_2] = "oo_Texture2",
  [SG_TEX_3] = "oo_Texture3",

  [SG_TEX_VALIDITY_0] = "oo_Texture0_valid",
  [SG_TEX_VALIDITY_1] = "oo_Texture1_valid",
  [SG_TEX_VALIDITY_2] = "oo_Texture2_valid",
  [SG_TEX_VALIDITY_3] = "oo_Texture3_valid",

  [SG_LIGHT] = "oo_Light[%u]",

  [SG_LIGHT_AMB] = "oo_Light[%u].ambient",
  [SG_LIGHT_POS] = "oo_Light[%u].pos",
  [SG_LIGHT_SPEC] = "oo_Light[%u].specular",
  [SG_LIGHT_DIFF] = "oo_Light[%u].diffuse",
  [SG_LIGHT_DIR] = "oo_Light[%u].dir",
  [SG_LIGHT_CONST_ATTEN] = "oo_Light[%u].constantAttenuation",
  [SG_LIGHT_LINEAR_ATTEN] = "oo_Light[%u].linearAttenuation",
  [SG_LIGHT_QUAD_ATTEN] = "oo_Light[%u].quadraticAttenuation",

  [SG_LIGHT_MOD_GLOB_AMB] = "oo_LightGlobalAmbient",

  [SG_MATERIAL_EMIT] = "oo_Material.emission",
  [SG_MATERIAL_AMB] = "oo_Material.ambient",
  [SG_MATERIAL_DIFF] = "oo_Material.diffuse",
  [SG_MATERIAL_SPEC] = "oo_Material.specular",
  [SG_MATERIAL_SHINE] = "oo_Material.shininess",

  [SG_MODELVIEW] = "oo_ModelViewMatrix",
  [SG_PROJECTION] = "oo_ProjectionMatrix",
  [SG_NORMAL_MATRIX] = "oo_NormalMatrix",
};

static hashtable_t *shaderKeyMap = NULL;


MODULE_INIT(shadermanager, NULL)
{
  ooLogTrace("initialising 'shadermanager' module");
  shaderKeyMap = hashtable_new_with_str_keys(64);
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

char_array_t
sgShaderPreprocess(mapped_file_t mf)
{
  char_array_t shader;
  char_array_init(&shader);

  for (int i = 0 ; i < mf.fileLenght ; i ++) {
    if (((char*)mf.data)[i] == '#') {
      ooLogTrace("found preprocessor directive");
      int j = i + 1;
      // Skip ws after '#'
      while (j < mf.fileLenght && (((char*)mf.data)[j] == ' ' || ((char*)mf.data)[j] == '\t')) {
        j ++;
      }
      if (strncmp(&(((char*)mf.data)[j]), "include", MIN(sizeof("include")-1, mf.fileLenght-j)) == 0) {
        ooLogTrace("found include directive");

        j += sizeof("include") - 1;

        // Skip ws after include dir
        while (j < mf.fileLenght && (((char*)mf.data)[j] == ' ' || ((char*)mf.data)[j] == '\t')) {
          j ++;
        }

        if (((char*)mf.data)[j] == '"') {
          j ++; // Skip quote
          int incfile_start = j;
          while (j < mf.fileLenght && (((char*)mf.data)[j] != '"' && ((char*)mf.data)[j] != '\n')) {
            j ++;
          }
          if (((char*)mf.data)[j] != '"') {
            ooLogError("syntax error for include directive\n");
            // TODO: Return error code
          }

          int incfile_end = j-1;

          // Valid include file path (probably)
          if (incfile_start < incfile_end) {
            char incfile[incfile_end - incfile_start + 2];
            memset(incfile, 0, incfile_end-incfile_start + 2);
            strncpy(incfile, &(((char*)mf.data)[incfile_start]), incfile_end - incfile_start + 1);
            FILE *file = ooResGetFile(incfile);
            if (file) {
              size_t bytes = 0;
              ooLogTrace("found include file");
              char byte;
              while (fread(&byte, sizeof(char), 1, file)) {
                char_array_push(&shader, byte);
                bytes ++;
              }
            } else {
              ooLogError("include file '%s' not found\n", incfile);
            }
          }
        }
        i = j;
        continue;
      }
    }
    char_array_push(&shader, ((char*)mf.data)[i]);
  }
  char_array_push(&shader, '\0');

  //printf("shader: %s", shader.elems);
  return shader;
}


void
sg_load_all_shaders(void)
{
  SG_CHECK_ERROR;
  const char *path = ooResGetPath("shaders");

  DIR *dir = opendir(path);
  if (dir) {
    char fullpath[PATH_MAX];
    struct dirent *entry = NULL;
    while ((entry = readdir(dir))) {
      if (entry->d_name[0] == '.') continue; // ignore parent and hidden dirs

      strcpy(fullpath, "shaders/");

      // Hardcoded ignore of shared directory
      if (strcmp(entry->d_name, "shared")) {
        strcat(fullpath, entry->d_name);
        sg_load_shader(entry->d_name, fullpath, fullpath, fullpath);
      }
    }

    closedir(dir);
  }
  SG_CHECK_ERROR;
}

sg_shader_t*
sg_load_shader(const char *key,
               const char *vspath,
               const char *fspath,
               const char *gspath)
{
  assert(key != NULL);
  assert(vspath != NULL);
  assert(fspath != NULL);

  SG_CHECK_ERROR;
  ooLogInfo("compiling '%s'", key);

  sg_shader_t *tmp = hashtable_lookup(shaderKeyMap, key);
  if (tmp) return tmp;


  sg_shader_t *si = smalloc(sizeof(sg_shader_t));
  memset(si, 0, sizeof(sg_shader_t));
  si->name = strdup(key);

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
      char_array_t vshader = sgShaderPreprocess(mf);

      shaderId = glCreateShader(GL_VERTEX_SHADER);
      GLint len = vshader.length;
      glShaderSource(shaderId, 1, (const GLchar**)&vshader.elems, &len);
      unmap_file(&mf);
      glCompileShader(shaderId);

      GLint compileStatus = 0;
      glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
      if (compileStatus == GL_FALSE) {
        GLint logLen = 0;
        GLint retrievedLen = 0;

        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLen);

        char *tmp = smalloc(logLen);

        glGetShaderInfoLog(shaderId, logLen, &retrievedLen, tmp);
        fputs(tmp, stderr);

        free(tmp);

        // No globfree as this is a fatal error
        ooLogFatal("vertex shader '%s' did not compile", shaders.gl_pathv[i]);
      }

      char_array_dispose(&vshader);
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
      char_array_t fshader = sgShaderPreprocess(mf);

      shaderId = glCreateShader(GL_FRAGMENT_SHADER);

      GLint len = fshader.length;
      glShaderSource(shaderId, 1, (const GLchar**)&fshader.elems, &len);
      unmap_file(&mf);
      glCompileShader(shaderId);

      GLint compileStatus = 0;
      glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
      if (compileStatus == GL_FALSE) {
        GLint logLen = 0;
        GLint retrievedLen = 0;

        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLen);

        char *tmp = smalloc(logLen);

        glGetShaderInfoLog(shaderId, logLen, &retrievedLen, tmp);
        fputs(tmp, stderr);

        free(tmp);

        // No globfree as this is a fatal error
        ooLogFatal("fragment shader '%s' did not compile", shaders.gl_pathv[i]);
      }

      char_array_dispose(&fshader);
      glAttachShader(shaderProgram, shaderId);
      ooLogTrace("loaded fragment shader '%s'", shaders.gl_pathv[i]);
    }
    globfree(&shaders);
  }

  // Ignore geometry shaders for now...
  //TODO: Fix attrib loc caching
  //si->attribs.texCoord0Id = SG_TEX0_COORD_LOC;
  //si->attribs.texCoord1Id = SG_TEX1_COORD_LOC;


  glBindAttribLocation(si->shaderId, SG_VERTEX, "oo_Vertex");
  glBindAttribLocation(si->shaderId, SG_NORMAL, "oo_Normal");
  glBindAttribLocation(si->shaderId, SG_COLOR, "oo_Color");
  glBindAttribLocation(si->shaderId, SG_TEX_COORD_0, "oo_TexCoord0");
  glBindAttribLocation(si->shaderId, SG_TEX_COORD_1, "oo_TexCoord1");
  glBindAttribLocation(si->shaderId, SG_TEX_COORD_2, "oo_TexCoord2");
  glBindAttribLocation(si->shaderId, SG_TEX_COORD_3, "oo_TexCoord3");

  // Set the output fragment name
  glBindFragDataLocation(shaderProgram, 0, "oo_FragColor");

  glLinkProgram(shaderProgram);
  GLint linkStatus = 0;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    GLint logLen = 0;
    GLint retrievedLen = 0;

    glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLen);

    char *tmp = smalloc(logLen);

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

  // After linking, we have valid uniform and attribute locations, we build up the location
  // table here.


  si->uniforms.modelViewId = sg_shader_get_location(si, SG_MODELVIEW, false);
  si->uniforms.projectionId = sg_shader_get_location(si, SG_PROJECTION, false);
  si->uniforms.normalMatrixId = sg_shader_get_location(si, SG_NORMAL_MATRIX, false);
  si->uniforms.materialId.ambient = sg_shader_get_location(si, SG_MATERIAL_AMB, false);
  si->uniforms.materialId.emission
    = sg_shader_get_location(si, SG_MATERIAL_EMIT, false);
  si->uniforms.materialId.diffuse
    = sg_shader_get_location(si, SG_MATERIAL_DIFF, false);
  si->uniforms.materialId.specular
    = sg_shader_get_location(si, SG_MATERIAL_SPEC, false);
  si->uniforms.materialId.shininess
    = sg_shader_get_location(si, SG_MATERIAL_SHINE, false);

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

  }
  // Glob amb should be outside of light struct...
  si->uniforms.globAmbient = glGetUniformLocation(shaderProgram,
                                                  "oo_LightGlobalAmbient");

  si->uniforms.texIds[0] = glGetUniformLocation(shaderProgram, "oo_Texture0");
  si->uniforms.texIds[1] = glGetUniformLocation(shaderProgram, "oo_Texture1");
  si->uniforms.texIds[2] = glGetUniformLocation(shaderProgram, "oo_Texture2");
  si->uniforms.texIds[3] = glGetUniformLocation(shaderProgram, "oo_Texture3");

  si->uniforms.tex_validity[0] = glGetUniformLocation(shaderProgram,
                                                      "oo_Texture0_valid");
  si->uniforms.tex_validity[1] = glGetUniformLocation(shaderProgram,
                                                      "oo_Texture1_valid");
  si->uniforms.tex_validity[2] = glGetUniformLocation(shaderProgram,
                                                      "oo_Texture2_valid");
  si->uniforms.tex_validity[3] = glGetUniformLocation(shaderProgram,
                                                      "oo_Texture3_valid");


  return si;
}

sg_shader_t*
sg_get_shader(const char *key)
{
  sg_shader_t *tmp = hashtable_lookup(shaderKeyMap, key);
  if (tmp) return tmp;
  else ooLogWarn("no such shader '%s'", key);
  return 0;
}

sg_shader_t*
sg_get_shader_without_warnings(const char *key)
{
  sg_shader_t *si = hashtable_lookup(shaderKeyMap, key);
  return si;
}

GLint
sg_shader_get_location(sg_shader_t *program, sg_param_id_t param, bool required)
{
  assert(program != NULL);
  SG_CHECK_ERROR;
  // Handle both attributes and uniforms
  if (param < SG_ATTRIBUTE_END) {
    GLint attrib_loc = glGetAttribLocation(program->shaderId, param_names[param]);
    if (required && (attrib_loc == -1)) {
      ooLogError("shader '%s' missing attribute: '%s'",
                 program->name, param_names[param]);
    }
    return attrib_loc;
  }

  GLint uniform_loc = glGetUniformLocation(program->shaderId, param_names[param]);
  if (required && (uniform_loc == -1)) {
    ooLogError("shader '%s' missing uniform: '%s'",
               program->name, param_names[param]);
  }
  return uniform_loc;
}

void
sg_shader_bind(sg_shader_t *program)
{
  SG_CHECK_ERROR;
  if (program) {
    //ooLogInfo("bind shader '%s'", program->name);
    glUseProgram(program->shaderId);
  } else {
    glUseProgram(0);
  }
  SG_CHECK_ERROR;
}

void
sg_shader_set_projection(sg_shader_t *shader, const float4x4 proj)
{
  assert(shader != NULL);
  SG_CHECK_ERROR;
  sg_shader_bind(shader);
  glUniformMatrix4fv(shader->uniforms.projectionId, 1, GL_TRUE,
                     (GLfloat*)proj);
  SG_CHECK_ERROR;

  //for (int i = 0; i < 4; i++) {
  //  ooLogInfo("project[%d]: [%f %f %f %f]",
  //            i, proj[i].x, proj[i].y, proj[i].z, proj[i].w);
  //}
}

void
sg_shader_set_model_view(sg_shader_t *shader, const float4x4 modelview)
{
  assert(shader != NULL);
  SG_CHECK_ERROR;

  sg_shader_bind(shader);
  glUniformMatrix4fv(shader->uniforms.modelViewId, 1, GL_TRUE,
                     (GLfloat*)modelview);
  SG_CHECK_ERROR;

  //for (int i = 0; i < 4; i++) {
  //  ooLogInfo("model[%d]: [%f %f %f %f]",
  //           i, modelview[i].x, modelview[i].y, modelview[i].z, modelview[i].w);
  //}
}

void
sg_shader_set_normal_matrix(sg_shader_t *shader, const float4x4 norm)
{
  assert(shader != NULL);
  SG_CHECK_ERROR;
  sg_shader_bind(shader);
  if (shader->uniforms.normalMatrixId >= 0) {
    float nmat[9];
    for (int i = 0 ; i < 3 ; i ++) {
      for (int j = 0 ; j < 3 ; j ++) {
        nmat[i*3+j] = norm[i][j];
      }
    }
    glUniformMatrix3fv(shader->uniforms.normalMatrixId, 1, GL_TRUE,
                       nmat);
  }
  SG_CHECK_ERROR;
}


void
sg_shader_bind_param_3fv(sg_shader_t *shader, sg_param_id_t param, float3 p)
{
  SG_CHECK_ERROR;
  sg_shader_bind(shader);
  glUniform3fv(sg_shader_get_location(shader, param, true), 1, (float*)&p);
  SG_CHECK_ERROR;
}

void
sg_shader_bind_param_4fv(sg_shader_t *shader, sg_param_id_t param, float4 p)
{
  SG_CHECK_ERROR;
  sg_shader_bind(shader);
  glUniform4fv(sg_shader_get_location(shader, param, true), 1, (float*)&p);
  SG_CHECK_ERROR;
}

void
sg_shader_bind_texture(sg_shader_t *shader, sg_texture_t *tex, unsigned tex_unit)
{
  SG_CHECK_ERROR;
  sg_shader_bind(shader);
  glUniform1i(shader->uniforms.texIds[tex_unit], tex_unit);
  glActiveTexture(GL_TEXTURE0 + tex_unit);
  glBindTexture(GL_TEXTURE_2D, sg_texture_get_id(tex));
  //glBindSampler(tex_unit, linearFiltering);
  SG_CHECK_ERROR;

  if (shader->uniforms.tex_validity[tex_unit] >= 0) {
    glUniform1i(shader->uniforms.tex_validity[tex_unit], 1);
    SG_CHECK_ERROR;
  }
  SG_CHECK_ERROR;
}

void
sg_shader_invalidate_textures(sg_shader_t *shader)
{
  SG_CHECK_ERROR;
  sg_shader_bind(shader);
  if (shader->uniforms.tex_validity[0] >= 0) {
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(shader->uniforms.tex_validity[0], 0);
    SG_CHECK_ERROR;
  }
  if (shader->uniforms.tex_validity[1] >= 0) {
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(shader->uniforms.tex_validity[1], 0);
    SG_CHECK_ERROR;
  }
  if (shader->uniforms.tex_validity[2] >= 0) {
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(shader->uniforms.tex_validity[2], 0);
    SG_CHECK_ERROR;
  }
  if (shader->uniforms.tex_validity[3] >= 0) {
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUniform1i(shader->uniforms.tex_validity[3], 0);
    SG_CHECK_ERROR;
  }
}

void
sg_shader_bind_light(sg_shader_t *shader, unsigned light_num,
                     sg_light_t *light)
{
  SG_CHECK_ERROR;
  sg_shader_bind(shader);

  float3 pos = sg_light_get_pos(light);

  //ooLogInfo("light pos %f %f %f", pos.x, pos.y, pos.z);

  float4 amb = sg_light_get_ambient(light);
  float4 diffuse = sg_light_get_diffuse(light);
  float4 specular = sg_light_get_specular(light);

  glUniform3fv(shader->uniforms.lightIds[light_num].pos, 1,
               (float*)&pos);
  glUniform4fv(shader->uniforms.lightIds[light_num].ambient, 1,
               (float*)&amb);
  glUniform4fv(shader->uniforms.lightIds[light_num].diffuse, 1,
               (float*)&diffuse);
  glUniform4fv(shader->uniforms.lightIds[light_num].specular, 1,
               (float*)&specular);
  glUniform1f(shader->uniforms.lightIds[light_num].constantAttenuation,
              sg_light_get_const_attenuation(light));
  glUniform1f(shader->uniforms.lightIds[light_num].linearAttenuation,
              sg_light_get_linear_attenuation(light));
  glUniform1f(shader->uniforms.lightIds[light_num].quadraticAttenuation,
              sg_light_get_quadratic_attenuation(light));
  SG_CHECK_ERROR;
  // shader->uniforms.lightIds[light_num].globAmbient =
  // sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_MOD_GLOB_AMB, i);

}

void
sg_shader_bind_amb(sg_shader_t *shader, float4 light)
{
  glUniform4fv(shader->uniforms.globAmbient, 1,
               (float*)&light);

}

GLint
sgGetLocationForParamAndIndex(GLuint program, sg_param_id_t param,
                              unsigned index)
{
  SG_CHECK_ERROR;
  assert(index < 100);

  char locName[strlen(param_names[param])];
  snprintf(locName, sizeof(locName), param_names[param], index);

  if (param < SG_ATTRIBUTE_END) {
    return glGetAttribLocation (program, locName);
  }
  return glGetUniformLocation(program, locName);
}

void
sg_shader_bind_material(sg_shader_t *shader, sg_material_t *mat)
{
  if (mat == NULL) return;
  float4 amb = sg_material_get_amb(mat);
  float4 diff = sg_material_get_diff(mat);
  float4 spec = sg_material_get_spec(mat);
  float4 emiss = sg_material_get_emiss(mat);
  float shine = sg_material_get_shininess(mat);

  glUniform4fv(shader->uniforms.materialId.ambient, 1, (float*)&amb);
  glUniform4fv(shader->uniforms.materialId.diffuse, 1, (float*)&diff);
  glUniform4fv(shader->uniforms.materialId.specular, 1, (float*)&spec);
  glUniform4fv(shader->uniforms.materialId.emission, 1, (float*)&emiss);
  glUniform1f(shader->uniforms.materialId.shininess, shine);
}