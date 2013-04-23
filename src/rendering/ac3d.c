/*
 Copyright 2009,2013 Mattias Holm <lorrden(at)openorbit.org>

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


#include "ac3d.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <vmath/vmath-convert.h>
#include "rendering/object.h"
#include "common/palloc.h"
#include <openorbit/log.h>

#define BUFF_SIZE 1024

#define SCAN_CHECK(bf, cnt, str, ...)                 \
  do {                                                \
    int readCnt = 0;\
    if ((readCnt = sscanf((bf), (str), __VA_ARGS__)) != (cnt)) {  \
      fprintf(stderr, "got %d tokens but expected %d from '%s'\n", readCnt, (cnt), (str));\
      goto error;                                     \
    }                                                 \
  } while (0)

struct ac3d_material_t {
  char *name;
  float r, g, b;
  float amb_r, amb_g, amb_b;
  float emis_r, emis_g, emis_b;
  float spec_r, spec_g, spec_b;
  float trans;
  float shi;
};

struct ac3d_refline {
  int vert_idx;
  float tex_x, tex_y;
};
struct ac3d_surface_t {
  int tag;
  int material_idx;
  int refs;
  struct ac3d_refline *ref_lines;
};

struct ac3d_object_t {
  char *kind;
  char *name;
  int data_len;
  uint8_t *data;
  char *texture;
  float texrep_x, texrep_y;
  float rot[3][3];
  float pos[3];
  char *url;
  int num_verts;
  float *verts;
  int num_surfs;
  struct ac3d_surface_t *surfs;
  int num_childs, read_childs;
  struct ac3d_object_t **children;
  struct ac3d_object_t *parent;
};

struct ac3d_file_t {
  size_t mat_count, mat_alen;
  struct ac3d_material_t *materials;
  size_t obj_count, obj_alen;
  struct ac3d_object_t **objs;
};

static char *gLastError = NULL;
int
push_material(struct ac3d_file_t *ac3d, struct ac3d_material_t *mat)
{
  if (ac3d->mat_alen <= ac3d->mat_count) {
    struct ac3d_material_t *mats = realloc(ac3d->materials,
                                           sizeof(struct ac3d_material_t) * (ac3d->mat_count+1) * 2);
    if (mats) {
      ac3d->materials = mats;
      ac3d->mat_alen = (ac3d->mat_count + 1)* 2;
      ac3d->materials[ac3d->mat_count ++] = *mat;
      return 0;
    }
    return -1;
  } else {
    ac3d->materials[ac3d->mat_count ++] = *mat;
    return 0;
  }
}

#define PUSH_MAT(ac, mt) if (push_material((ac), (mt))) goto error;

int
push_object(struct ac3d_file_t *ac3d, struct ac3d_object_t *obj)
{
  if (ac3d->obj_alen <= ac3d->obj_count) {
    struct ac3d_object_t **objs = realloc(ac3d->objs,
                                          sizeof(struct ac3d_object_t*) * (ac3d->obj_count+1) * 2);
    if (objs) {
      ac3d->objs = objs;
      ac3d->obj_alen = (ac3d->obj_count + 1) * 2;
      ac3d->objs[ac3d->obj_count ++] = obj;
      return 0;
    }
    return -1;
  } else {
    ac3d->objs[ac3d->obj_count ++] = obj;
    return 0;
  }
}

struct ac3d_file_t*
make_file(void)
{
  struct ac3d_file_t *ac3d = smalloc(sizeof(struct ac3d_file_t));
  memset(ac3d, 0, sizeof(struct ac3d_file_t));

  ac3d->mat_alen = 8;
  ac3d->mat_count = 0;
  ac3d->materials = calloc(8, sizeof(struct ac3d_material_t));
  ac3d->obj_alen = 8;
  ac3d->obj_count = 0;
  ac3d->objs = calloc(8, sizeof(struct ac3d_object_t*));

  return ac3d;
}

#define PUSH_OBJ(ac, ob) if (push_object((ac), (ob))) goto error;

#define FORCE_GETS(bf, sz, fp)        \
  do {                                \
    if (!fgets(buff, BUFF_SIZE, fp)) {\
      fprintf(stderr, "unexpected end of file, in state %s:%d\n", __FILE__, __LINE__);\
      goto error;                     \
    }                                 \
    if (buff[strlen(buff)-1] == '\n') buff[strlen(buff)-1] = '\0'; \
  } while (0)

struct ac3d_object_t*
make_object(void)
{
  struct ac3d_object_t obj =
    { NULL, NULL, // Kind, NAME
      0, NULL, // DATA
      NULL, 0.0, 0.0, //TEXTURE
      { // ROT
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
      },
      {0.0, 0.0, 0.0}, // POS
      NULL, // URL
      0, // num verts
      NULL, // verts
      0, // num surfaces
      NULL, // surfaces
      0, 0, // num children
      NULL, // children
      NULL // parent
    };

  struct ac3d_object_t *newObj = smalloc(sizeof(struct ac3d_object_t));
  memset(newObj, 0, sizeof(struct ac3d_object_t));

  memcpy(newObj, &obj, sizeof(struct ac3d_object_t));
  return newObj;
}

void
ac3d_obj_delete(struct ac3d_object_t *obj)
{
  for (int i = 0 ; i < obj->num_childs ; ++ i) {
    ac3d_obj_delete(obj->children[i]);
  }

  free(obj->kind);
  free(obj->name);
  free(obj->texture);
  free(obj->data);
  free(obj->url);
  free(obj->verts);
  free(obj->surfs);
  free(obj->children);
  free(obj);
}

void
ac3d_delete(struct ac3d_file_t *ac3d)
{
  for (int i = 0 ; i < ac3d->obj_count ; ++ i) {
    ac3d_obj_delete(ac3d->objs[i]);
  }

  free(ac3d->objs);
  free(ac3d->materials);
  free(ac3d);
}

float vecangle(float3 a, float3 b)
{
  float dotprod = vf3_dot(a, b);
  float anorm = vf3_abs(a);
  float bnorm = vf3_abs(b);

  float cosang = dotprod/(anorm*bnorm);
  float acos = acosf(cosang);
  return fabsf(acos);
}

// This is maybe not the most efficient way, since it makes copys of all the
// data, the imgload lib steals the buffers and transfers their ownership
// instead
sg_object_t*
ac3d_obj_to_model(struct ac3d_file_t *ac3d, struct ac3d_object_t *obj,
                  sg_shader_t *shader)
{
  assert(ac3d != NULL);
  assert(obj != NULL);
  assert(shader != NULL);

  bool has_warned_for_materials = false;

  sg_object_t *model = sg_new_object(shader, obj->name);

  // Transfer rotation matrix, but ensure it is transposed for GL
  // Note, no longer applicable for GL 3, we simply flag matrix uniforms as
  // needing transposition.
  double3x3 rot;
  md3_ident(rot);

  // Does not transpose anymore
  for (int i = 0 ; i < 3 ; ++ i) {
    rot[i] = vd3_set(obj->rot[i][0], obj->rot[i][1], obj->rot[i][2]);
  }

  sg_object_set_rot(model, rot);
  sg_object_set_parent_offset(model,
                              vd3_set(obj->pos[0], obj->pos[1], obj->pos[2]));

  // BUG: assume this object has the same material for all faces, this is not
  //      correct, and we should ensure that a face with its own material
  //      is placed in its own vertex array, for now this is most likelly OK
  //      but we should address it at some point.

  sg_material_t *mat = sg_new_material();
  sg_object_set_material(model, mat);

  if (obj->num_surfs > 0) {
    struct ac3d_material_t *acmat = &ac3d->materials[obj->surfs[0].material_idx];
    sg_material_set_diff4f(mat, acmat->r, acmat->g, acmat->b, 1.0-acmat->trans);
    sg_material_set_emiss4f(mat, acmat->emis_r, acmat->emis_g, acmat->emis_b,
                            1.0-acmat->trans);
    sg_material_set_spec4f(mat, acmat->spec_r, acmat->spec_g, acmat->spec_b,
                           1.0-acmat->trans);
    sg_material_set_amb4f(mat, acmat->amb_r, acmat->amb_g, acmat->amb_b,
                          1.0-acmat->trans);
    sg_material_set_shininess(mat, acmat->shi);
  }

  float_array_t face_normals;
  float_array_init(&face_normals);

  int_array_t vertex_sharing[obj->num_verts];
  for (int i = 0 ; i < obj->num_verts ; ++ i) {
    int_array_init(&vertex_sharing[i]);
  }

  // Compute face normals and shared vertices
  // Tag vertex as being shared by the one here, and compute the face normals
  for (int i = 0 ; i < obj->num_surfs ; ++ i) {
    // First the vertex sharing
    for (int j = 0 ; j < obj->surfs[i].refs ; ++ j) {
      int_array_push(&vertex_sharing[obj->surfs[i].ref_lines[j].vert_idx], i);
    }

    // Cross product to get face normal
    float *p0 = &obj->verts[obj->surfs[i].ref_lines[0].vert_idx*3];
    float *p1 = &obj->verts[obj->surfs[i].ref_lines[1].vert_idx*3];
    float *p2 = &obj->verts[obj->surfs[i].ref_lines[2].vert_idx*3];

    float3 vb = vf3_set(p0[0] - p1[0], p0[1] - p1[1], p0[2] - p1[2]);
    float3 va = vf3_set(p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]);
    float3 vc = vf3_normalise(vf3_cross(va, vb));

    float_array_push(&face_normals, vc[0]);
    float_array_push(&face_normals, vc[1]);
    float_array_push(&face_normals, vc[2]);

    if (i > 0 && has_warned_for_materials == false) {
      if (obj->surfs[0].material_idx != obj->surfs[i].material_idx) {
        has_warned_for_materials = true;
        fprintf(stderr,
                "different materials not supported for the same object, "
                "material forced\n");
      }
    }
  }

  float_array_t normals, vertices, texcoords, colours;
  float_array_init(&normals);
  float_array_init(&vertices);
  float_array_init(&texcoords);
  float_array_init(&colours);

  // Go through each face again and add the vertices
  for (int i = 0 ; i < obj->num_surfs ; ++ i) {
    if (obj->surfs[i].refs < 3) {
      fprintf(stderr, "trying to load primitive that is not a polygon, "
                      "not supported (refs = %d)\n", obj->surfs[i].refs);
    }

    // Build up an index vector for on the fly triangulation of the polygon
    int triangles = 1 + (obj->surfs[i].refs - 3);
    int idx_seq_len = triangles * 3;
    int idxseq[idx_seq_len];
    for (int j = 0 ; j < triangles ; ++ j) {
      idxseq[j*3+0] = 0;
      idxseq[j*3+1] = j + 1;
      idxseq[j*3+2] = j + 2;
    }

    for (int j = 0 ; j < idx_seq_len ; ++ j) {
      float3 normal;
      float3 faceNormal;
      faceNormal[0] = face_normals.elems[i*3+0];
      faceNormal[1] = face_normals.elems[i*3+1];
      faceNormal[2] = face_normals.elems[i*3+2];

      normal = faceNormal;

      int vertexId = obj->surfs[i].ref_lines[idxseq[j]].vert_idx;

      // Compute vertex normals, compensating for the crease angle
      // TODO: Read crease angle from file, currently fixed to 45.0 deg
      for (int k = 0 ; k < vertex_sharing[vertexId].length ; ++ k) {
        int faceId = vertex_sharing[vertexId].elems[k];

        if (faceId != i) {
          assert(faceId >= 0);
          float3 nextFaceNormal;
          nextFaceNormal[0] = face_normals.elems[faceId*3+0];
          nextFaceNormal[1] = face_normals.elems[faceId*3+1];
          nextFaceNormal[2] = face_normals.elems[faceId*3+2];

          if (vecangle(faceNormal, nextFaceNormal) < DEG_TO_RAD(45.0f)) {
            normal += nextFaceNormal;
          }
        }
      }

      normal = vf3_normalise(normal);

      float_array_push(&normals, normal[0]);
      float_array_push(&normals, normal[1]);
      float_array_push(&normals, normal[2]);

      float_array_push(&vertices,
                       obj->verts[3*obj->surfs[i].ref_lines[idxseq[j]].vert_idx+0]);
      float_array_push(&vertices,
                       obj->verts[3*obj->surfs[i].ref_lines[idxseq[j]].vert_idx+1]);
      float_array_push(&vertices,
                       obj->verts[3*obj->surfs[i].ref_lines[idxseq[j]].vert_idx+2]);

      float_array_push(&texcoords,
                       obj->surfs[i].ref_lines[idxseq[j]].tex_x);
      float_array_push(&texcoords,
                       obj->surfs[i].ref_lines[idxseq[j]].tex_y);

      float_array_push(&colours,
                       ac3d->materials[obj->surfs[i].material_idx].r);
      float_array_push(&colours,
                       ac3d->materials[obj->surfs[i].material_idx].g);
      float_array_push(&colours,
                       ac3d->materials[obj->surfs[i].material_idx].b);
    }
  }

  float *verts = vertices.elems;
  float *texc = texcoords.length ? texcoords.elems : NULL;
  float *norm = normals.length ? normals.elems : NULL;

  sg_object_set_geo(model, GL_TRIANGLES, vertices.length / 3,
                    verts, norm, texc);


  float_array_dispose(&face_normals);
  for (int i = 0 ; i < obj->num_verts ; ++ i) {
    free(vertex_sharing[i].elems);
  }

  float_array_dispose(&normals);
  float_array_dispose(&vertices);
  float_array_dispose(&texcoords);
  float_array_dispose(&colours);

  for (int i = 0 ; i < obj->num_childs ; ++ i) {
    sg_object_add_child(model, ac3d_obj_to_model(ac3d, obj->children[i], shader));
  }

  //  model->texture = (obj->texture) ? strdup(obj->texture) : NULL;

  //model->model = mod;

  return model;
}

sg_object_t*
ac3d_to_model(struct ac3d_file_t *ac3d, sg_shader_t *shader)
{
  sg_object_t *model = NULL;

  if (ac3d->obj_count > 1) {
    model = sg_new_object(shader, "");
    for (int i = 0 ; i < ac3d->obj_count ; ++ i) {
      sg_object_t *child = ac3d_obj_to_model(ac3d, ac3d->objs[i], shader);
      assert(child != NULL);
      sg_object_add_child(model, child);
    }
  } else {
    model = ac3d_obj_to_model(ac3d, ac3d->objs[0], shader);
  }

  return model;
}

struct ac3d_object_t *
ac3d_read_obj(FILE *fp, const char *name)
{
  char buff[BUFF_SIZE];
  char nameBuff[BUFF_SIZE];

  struct ac3d_object_t *obj = make_object();
  obj->kind = strdup(name);


  while (!feof(fp)) {
    FORCE_GETS(buff, BUFF_SIZE, fp);
    if (!memcmp(buff, "name", 4)) {
      SCAN_CHECK(buff, 1, "name %s", nameBuff);
    } else if (!memcmp(buff, "data", 4)) {
      SCAN_CHECK(buff, 1, "data %d", &obj->data_len);
    } else if (!memcmp(buff, "texture", 7)) {
      SCAN_CHECK(buff,  1, "texture %s", nameBuff);
      obj->texture = strdup(nameBuff);
    } else if (!memcmp(buff, "texrep", 6)) {
      SCAN_CHECK(buff, 2, "texrep %f %f", &obj->texrep_x, &obj->texrep_y);
    } else if (!memcmp(buff, "rot", 3)) {
      SCAN_CHECK(buff, 9, "rot %f %f %f  %f %f %f  %f %f %f",
                 &obj->rot[0][0], &obj->rot[0][1], &obj->rot[0][2],
                 &obj->rot[1][0], &obj->rot[1][1], &obj->rot[1][2],
                 &obj->rot[2][0], &obj->rot[2][1], &obj->rot[2][2]);
    } else if (!memcmp(buff, "loc", 3)) {
      SCAN_CHECK(buff, 3, "loc %f %f %f",
                 &obj->pos[0], &obj->pos[1], &obj->pos[2]);
    } else if (!memcmp(buff, "url", 3)) {
      SCAN_CHECK(buff, 1, "url %s", nameBuff);
    } else if (!memcmp(buff, "numvert", 7)) {
      SCAN_CHECK(buff, 1, "numvert %d", &obj->num_verts);
      obj->verts = calloc(obj->num_verts*3, sizeof(float));

      for (size_t i = 0 ; i < obj->num_verts ; ++ i) {
        FORCE_GETS(buff, BUFF_SIZE, fp);
        SCAN_CHECK(buff, 3, "%f %f %f",
                   &obj->verts[i*3], &obj->verts[i*3+1], &obj->verts[i*3+2]);
      }
    } else if (!memcmp(buff, "numsurf", 7)) {
      SCAN_CHECK(buff, 1, "numsurf %d", &obj->num_surfs);
      obj->surfs = calloc(obj->num_surfs, sizeof(struct ac3d_surface_t));

      for (size_t i = 0 ; i < obj->num_surfs ; ++ i) {
        FORCE_GETS(buff, BUFF_SIZE, fp);

        SCAN_CHECK(buff, 1, "SURF %d", &obj->surfs[i].tag);
        FORCE_GETS(buff, BUFF_SIZE, fp);
        SCAN_CHECK(buff, 1, "mat %d", &obj->surfs[i].material_idx);

        FORCE_GETS(buff, BUFF_SIZE, fp);
        SCAN_CHECK(buff, 1, "refs %d", &obj->surfs[i].refs);
        obj->surfs[i].ref_lines = calloc(obj->surfs[i].refs, sizeof(struct ac3d_refline));

        for (int j = 0 ; j < obj->surfs[i].refs ; ++ j) {
          FORCE_GETS(buff, BUFF_SIZE, fp);
          SCAN_CHECK(buff, 3, "%d %f %f",
                     &obj->surfs[i].ref_lines[j].vert_idx,
                     &obj->surfs[i].ref_lines[j].tex_x,
                     &obj->surfs[i].ref_lines[j].tex_y);
        }
      }
    } else if (!memcmp(buff, "kids", 4)) {
      SCAN_CHECK(buff, 1, "kids %d", &obj->num_childs);

      if (obj->num_childs > 0) {
        obj->children = calloc(obj->num_childs, sizeof(struct ac3d_object_t*));
        if (!obj->children) {
          fprintf(stderr, "alloc failed\n");
          return NULL;
        }

        for (size_t i = 0 ; i < obj->num_childs ; ++ i) {
          //fprintf(stderr, "%p: %d/%d\n", (void*)obj, i, obj->num_childs);
          nameBuff[0] = '\0';
          FORCE_GETS(buff, BUFF_SIZE, fp);
          SCAN_CHECK(buff, 1, "OBJECT %s", nameBuff);
          obj->children[i] = ac3d_read_obj(fp, nameBuff);
          if (obj->children[i]) {
            obj->children[i]->parent = obj;
            obj->read_childs ++;
          } else {
            fprintf(stderr, "error reading file...\n");
          }
        }
      }

      return obj;
    } else {
      if (gLastError && strcmp(gLastError, buff)) {
        free(gLastError);
        gLastError = strdup(buff);
        fprintf(stderr, "invalid line '%s' ignored\n", buff);
      } else if (gLastError == NULL) {
        gLastError = strdup(buff);
        fprintf(stderr, "invalid line '%s' ignored\n", buff);
      } // Otherwise ignore and do not print error message
    }
  }

error:
  free(obj);
  return NULL;
}

sg_object_t*
ac3d_load(const char *path, sg_shader_t *shader)
{
  FILE *fp = fopen(path, "r");
  if (!fp) {
    return NULL;
  }
  unsigned int vers = INT_MAX;
  /* Read header */
  if (fscanf(fp, "AC3D%x\n", &vers) != 1) {
    fclose(fp);
    return NULL;
  }

  if (vers > 0xb) {
    fprintf(stderr,
            "ac3d versions greater than 0xb not supported, file vers is 0x%x\n",
            vers);
    fclose(fp);
    return NULL;
  }

  struct ac3d_file_t *ac3d = make_file();
  struct ac3d_object_t *obj = NULL;

  char buff[BUFF_SIZE];
  while (fgets(buff, BUFF_SIZE, fp)) {
    if (!memcmp(buff, "MATERIAL", 8)) {
      char nameBuff[BUFF_SIZE]; nameBuff[0] = '\0';
      struct ac3d_material_t mat;
      SCAN_CHECK(buff, 15,
                 "MATERIAL %s rgb %f %f %f  "
                 "amb %f %f %f  emis %f %f %f  "
                 "spec %f %f %f  shi %f  trans %f",
                 nameBuff,
                 &mat.r, &mat.g, &mat.b,
                 &mat.amb_r, &mat.amb_g, &mat.amb_b,
                 &mat.emis_r, &mat.emis_g, &mat.emis_b,
                 &mat.spec_r, &mat.spec_g, &mat.spec_b,
                 &mat.shi, &mat.trans);

      mat.name = strdup(nameBuff);

      PUSH_MAT(ac3d, &mat);
    } else if (!memcmp(buff, "OBJECT", 6)) {
      char nameBuff[BUFF_SIZE]; nameBuff[0] = '\0';
      SCAN_CHECK(buff, 1, "OBJECT %s", nameBuff);
      obj = ac3d_read_obj(fp, nameBuff);
      PUSH_OBJ(ac3d, obj);
    }
  }

  if (ferror(fp)) {
    log_error("error reading file '%s'\n", path);
    fclose(fp);
    ac3d_delete(ac3d);
    return NULL;
  }

  sg_object_t *model = ac3d_to_model(ac3d, shader);
  ac3d_delete(ac3d);

  return model;
error:
  fclose(fp);
  ac3d_delete(ac3d);
  log_error("ac3d load failed");

  return NULL;
}
