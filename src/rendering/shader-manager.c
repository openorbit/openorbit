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

struct sg_light_ids_t {
  GLint pos;
  GLint ambient;
  GLint specular;
  GLint diffuse;
  GLint dir;

  GLint constantAttenuation;
  GLint linearAttenuation;
  GLint quadraticAttenuation;
  GLint globAmbient;
} ;

struct sg_material_ids_t {
  GLint emission;
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint shininess;
};

struct sg_shader_t {
  GLuint shaderId;

  // Uniform IDs
  struct {
    GLuint modelViewId;
    GLuint projectionId;
    GLuint normalMatrixId;
    GLuint texIds[SG_OBJ_MAX_TEXTURES];
    sg_light_ids_t lightIds[SG_OBJ_MAX_LIGHTS];
    sg_material_ids_t materialId;
  } uniforms;
  // Attribute IDs
  struct {
    GLuint vertexId;
    GLuint normalId;
    GLuint colourId;
    GLuint texCoordId[SG_OBJ_MAX_TEXTURES];
  } attribs;
};


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
      ooLogInfo("found preprocessor directive");
      int j = i + 1;
      // Skip ws after '#'
      while (j < mf.fileLenght && (((char*)mf.data)[j] == ' ' || ((char*)mf.data)[j] == '\t')) {
        j ++;
      }
      if (strncmp(&(((char*)mf.data)[j]), "include", MIN(sizeof("include")-1, mf.fileLenght-j)) == 0) {
        ooLogInfo("found include directive");

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
            fprintf(stderr, "syntax error for include directive\n");
            // TODO: Return error code
          }

          int incfile_end = j-1;

          // Valid include file path (probably)
          if (incfile_start < incfile_end) {
            char incfile[incfile_end - incfile_start + 1];
            memset(incfile, incfile_end-incfile_start + 1, 0);
            strncpy(incfile, &(((char*)mf.data)[incfile_start]), incfile_end - incfile_start + 1);
            FILE *file = ooResGetFile(incfile);
            if (file) {
              size_t bytes = 0;
              printf("found include file '%s'\n", incfile);
              char byte;
              while (fread(&byte, sizeof(char), 1, file)) {
                char_array_push(&shader, byte);
                bytes ++;
              }
            } else {
              fprintf(stderr, "include file '%s' not found\n", incfile);
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

  printf("shader: %s", shader.elems);
  return shader;
}


void
sg_load_all_shaders(void)
{
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

  ooLogInfo("compiling '%s'", key);

  sg_shader_t *tmp = hashtable_lookup(shaderKeyMap, key);
  if (tmp) return tmp;


  sg_shader_t *si = malloc(sizeof(sg_shader_t));

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

        char *tmp = malloc(logLen);

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

        char *tmp = malloc(logLen);

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

  // Set the output fragment name
  glBindFragDataLocation(shaderProgram, 0, SG_OUT_FRAGMENT);

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

  // After linking, we have valid uniform and attribute locations, we build up the location
  // table here.

  si->attribs.vertexId = sg_shader_get_location(si, SG_VERTEX);
  si->attribs.normalId = sg_shader_get_location(si, SG_NORMAL);
  si->attribs.colourId = sg_shader_get_location(si, SG_COLOR);
  for (int i = 0 ; i < SG_OBJ_MAX_TEXTURES ; i++) {
    si->attribs.texCoordId[i] = sgGetLocationForParamAndIndex(shaderProgram, SG_TEX_COORD, i);
  }

  si->uniforms.modelViewId = sg_shader_get_location(si, SG_MODELVIEW);
  si->uniforms.projectionId = sg_shader_get_location(si, SG_PROJECTION);
  si->uniforms.normalMatrixId = sg_shader_get_location(si, SG_NORMAL_MATRIX);
  si->uniforms.materialId.ambient = sg_shader_get_location(si, SG_MATERIAL_AMB);
  si->uniforms.materialId.emission
    = sg_shader_get_location(si, SG_MATERIAL_EMIT);
  si->uniforms.materialId.diffuse
    = sg_shader_get_location(si, SG_MATERIAL_DIFF);
  si->uniforms.materialId.specular
    = sg_shader_get_location(si, SG_MATERIAL_SPEC);
  si->uniforms.materialId.shininess
    = sg_shader_get_location(si, SG_MATERIAL_SHINE);

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

    // Glob amb should be outside of light struct...
    si->uniforms.lightIds[i].globAmbient =
      sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_MOD_GLOB_AMB, i);
  }

  for (int i = 0 ; i < SG_OBJ_MAX_TEXTURES ; i++) {
    si->uniforms.texIds[i] = sgGetLocationForParamAndIndex(shaderProgram,
                                                           SG_TEX, i);
  }

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
sg_shader_get_location(sg_shader_t *program, sg_param_id_t param)
{
  // Handle both attributes and uniforms
  if (param < SG_ATTRIBUTE_END) {
    return glGetAttribLocation (program->shaderId, param_names[param]);
  }
  return glGetUniformLocation(program->shaderId, param_names[param]);
}

void
sg_shader_bind(sg_shader_t *program)
{
  if (program) {
    glUseProgram(program->shaderId);
  } else {
    glUseProgram(0);
  }
}

void
sg_shader_set_projection(sg_shader_t *shader, const float4x4 proj)
{
  sg_shader_bind(shader);
  glUniformMatrix4fv(shader->uniforms.projectionId, 1, GL_TRUE,
                     (GLfloat*)proj);
}

void
sg_shader_set_model_view(sg_shader_t *shader, const float4x4 modelview)
{
  sg_shader_bind(shader);
  glUniformMatrix4fv(shader->uniforms.modelViewId, 1, GL_TRUE,
                     (GLfloat*)modelview);
}

void
sg_shader_bind_param_3fv(sg_shader_t *shader, sg_param_id_t param, float3 p)
{
  sg_shader_bind(shader);
  glUniform3fv(sg_shader_get_location(shader, param), 1, (float*)&p);
}

void
sg_shader_bind_param_4fv(sg_shader_t *shader, sg_param_id_t param, float4 p)
{
  sg_shader_bind(shader);
  glUniform4fv(sg_shader_get_location(shader, param), 1, (float*)&p);
}

void
sg_shader_bind_texture(sg_shader_t *shader, unsigned tex_num, sg_texture_t *tex)
{
  sg_shader_bind(shader);
  glUniform1i(shader->uniforms.texIds[tex_num], sg_texture_get_id(tex));
}

void
sg_shader_bind_light(sg_shader_t *shader, unsigned light_num,
                     sg_light_t *light)
{
  sg_shader_bind(shader);

  float3 pos = sg_light_get_pos(light);
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

  // shader->uniforms.lightIds[light_num].globAmbient =
  // sgGetLocationForParamAndIndex(shaderProgram, SG_LIGHT_MOD_GLOB_AMB, i);

}



void
sg_location_bind(GLuint prog, sg_param_id_t param)
{
  glBindAttribLocation(prog, param, param_names[param]);
}

void
sg_location_bind_at_index(GLuint prog, sg_param_id_t param, unsigned index)
{
  glBindAttribLocation(prog, param, param_names[param] + index);
}


GLint
sgGetLocationForParamAndIndex(GLuint program, sg_param_id_t param,
                              unsigned index)
{
  assert(index < 100);

  char locName[strlen(param_names[param])];
  snprintf(locName, sizeof(locName), param_names[param], index);

  if (param < SG_ATTRIBUTE_END) {
    return glGetAttribLocation (program, locName);
  }
  return glGetUniformLocation(program, locName);
}

void
sgSetShaderTex(GLuint program, unsigned index, GLuint tex)
{
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint loc = sgGetLocationForParamAndIndex(program, SG_TEX, index);
  glUniform1i(loc, index); // Uniform value is texture unit id
}

void
sg_location_bind_matrix(sg_shader_t *prog, sg_param_id_t param)
{
  
}


GLint
sg_shader_get_vertex_attrib(sg_shader_t *program)
{
  return program->attribs.vertexId;
}

GLint
sg_shader_get_normal_attrib(sg_shader_t *program)
{
  return program->attribs.normalId;
}

GLint
sg_shader_get_color_attrib(sg_shader_t *program)
{
  return program->attribs.colourId;
}

GLint
sg_shader_get_texcoord_attrib(sg_shader_t *program, unsigned tex_idx)
{
  return program->attribs.texCoordId[tex_idx];
}
