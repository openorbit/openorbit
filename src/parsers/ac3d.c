/*
 Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include "ac3d.h"
#include "model.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
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
  int shi;
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
  struct ac3d_file_t *ac3d = malloc(sizeof(struct ac3d_file_t));
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

  struct ac3d_object_t *newObj = malloc(sizeof(struct ac3d_object_t));
  *newObj = obj;
  return newObj;
}

void
ac3d_delete(struct ac3d_file_t *ac3d)
{

}

model_t*
ac3d_obj_to_model(struct ac3d_object_t *obj)
{
  assert(obj != NULL);
  model_t *model = malloc(sizeof(model_t));

  if (obj->num_childs > 10000) {
    fprintf(stderr, "really huge number of objects\n");
  }
  model->childCount = obj->num_childs;

  if (model->childCount > 0) {
    model->children = calloc(obj->num_childs, sizeof(model_t*));
  } else {
    model->children = NULL;
  }

  for (int i = 0 ; i < obj->num_childs ; ++ i) {
    model->children[i] = ac3d_obj_to_model(obj->children[i]);
  }

  return model;
}

model_t*
ac3d_to_model(struct ac3d_file_t *ac3d)
{
  model_t *model = malloc(sizeof(model_t));
  model->childCount = ac3d->obj_count;
  model->children = calloc(ac3d->obj_count, sizeof(model_t*));

  for (int i = 0 ; i < ac3d->obj_count ; ++ i) {
    model->children[i] = ac3d_obj_to_model(ac3d->objs[i]);
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
        }

        for (size_t i = 0 ; i < obj->num_childs ; ++ i) {
          fprintf(stderr, "%p: %d/%d\n", (void*)obj, i, obj->num_childs);
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
      fprintf(stderr, "invalid line '%s'\n", buff);
    }
  }

error:
  return NULL;
}

model_t*
ac3d_load(const char *path)
{
  FILE *fp = fopen(path, "r");
  if (!fp) {
    return NULL;
  }
  unsigned int vers = INT_MAX;
  /* Read header */
  if (fscanf(fp, "AC3D%x\n", &vers) != 1) {
    goto error;
  }

  if (vers > 0xb) {
    fprintf(stderr,
            "ac3d versions greater than 0xb not supported, file vers is 0x%x\n",
            vers);
    goto error;
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
                 "spec %f %f %f  shi %d  trans %f",
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
    fprintf(fp, "error reading file '%s'\n", path);
    fclose(fp);
    ac3d_delete(ac3d);
    return NULL;
  }

  // TODO: Convert ac3d to model
  model_t *model = ac3d_to_model(ac3d);
  ac3d_delete(ac3d);

  return model; // TODO: fix
error:
  fclose(fp);
  ac3d_delete(ac3d);

  return NULL;
}

