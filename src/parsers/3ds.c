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


#include "3ds.h"
#include "common/bitutils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <setjmp.h>

// Chunk constants
#define M3DMAGIC 0x4d4d
#define M3D_VERSION 0x0002
#define MDATA 0x3d3d
#define MESH_VERSION 0x3d3e
#define MAT_ENTRY 0xafff

#define MAT_NAME 0xa000

#define NAMED_OBJECT 0x4000
#define OBJ_HIDDEN 0x4010
#define OBJ_VIS_LOFTER 0x4011
#define OBJ_DOESNT_CAST 0x4012
#define OBJ_MATTE 0x4013
#define OBJ_FAST 0x4014
#define OBJ_PROCEDURAL 0x4015
#define OBJ_FROZEN 0x4016
#define OBJ_DONT_RCVSHADOW 0x4017
#define N_TRI_OBJECT 0x4100

#define POINT_ARRAY 0x4110
#define POINT_FLAG_ARRAY 0x4111
#define FACE_ARRAY 0x4120
#define MSH_MAT_GROUP 0x4130
#define TEX_VERTS 0x4140
#define SMOOTH_GROUP 0x4150
#define MESH_MATRIX 0x4160
#define MESH_COLOR 0x4165
#define MESH_TEXTURE_INFO 0x4170
#define N_DIRECT_LIGHT 0x4600
#define N_AMBIENT_LIGHT 0x4680
#define N_CAMERA 0x4700




typedef struct chunk_3ds_t {
  uint16_t chunkId;
  uint32_t chunkSize;
} chunk_3ds_t;
#define CHUNK_HDR_SIZE 6

float readFloat(int fd)
{
  union {float f; uint32_t i;} val;
  assert(read(fd, &val.i, 4) == 4);

  val.i = ooLittleToHost32(val.i);

  return val.f;
}

uint8_t readU8(int fd)
{
  uint8_t val;
  assert(read(fd, &val, 1) == 1);

  return val;
}

char *readStr(int fd)
{
  char str[1024];
  int i = 0;
  char val;
  while ((read(fd, &val, 1) == 1) && (val != '\0')) {
    str[i ++] = val;
  }
  str[i] = '\0';

  return strdup(str);
}


uint16_t readU16(int fd)
{
  uint16_t val;
  assert(read(fd, &val, 2) == 2);
  val = ooLittleToHost16(val);

  return val;
}

uint32_t readU32(int fd)
{
  uint32_t val;
  assert(read(fd, &val, 4) == 4);
  val = ooLittleToHost32(val);

  return val;
}


static inline int readChunkHeader(chunk_3ds_t *chnk, int fd)
{
  int readBytes = 0;

  if ((readBytes = read(fd, &chnk->chunkId, 2)) != 2) {
    if (readBytes == 0) {
      // EOF
      return -1;
    }
    return -2; // Unexpected
  }
  if ((readBytes = read(fd, &chnk->chunkSize, 4)) != 4) {
    if (readBytes == 0) {
      // EOF, not expected
      return -2;
    }
  }

  // Swap endianess if needed
  chnk->chunkId = ooLittleToHost16(chnk->chunkId);
  chnk->chunkSize = ooLittleToHost32(chnk->chunkSize);

  return 0;
}

static inline void
skipBytes(int fd, off_t bytes)
{
  assert(lseek(fd, bytes, SEEK_CUR) != -1);
}

// Main reader
static jmp_buf m3d_err;

void m3derror(const char *str)
{
  fputs(str, stderr);
  longjmp(m3d_err, 1);
}
int readFaces(int fd, size_t maxBytes)
{
  chunk_3ds_t chnk = {0, 0};
  fprintf(stderr, "reading faces... %d\n", (int)maxBytes);
  size_t remBytes = maxBytes;

  uint16_t nfaces = readU16(fd);
  struct {
    short vertex1, vertex2, vertex3;
    short flags;
  } facearray[nfaces];
  for (int i = 0 ; i < nfaces ; i ++) {
    facearray[i].vertex1 = readU16(fd);
    facearray[i].vertex2 = readU16(fd);
    facearray[i].vertex3 = readU16(fd);
    facearray[i].flags = readU16(fd);
  }
  assert(remBytes >= 2 + 2 * 4 * nfaces);

  remBytes -= 2 + nfaces * 4 * 2;

  while (remBytes > 0 && (readChunkHeader(&chnk, fd) == 0)) {
    switch (chnk.chunkId) {
    case SMOOTH_GROUP:
      {
    //      short grouplist[n]; determined by length, seems to be 4 per face
        fprintf(stderr, "...got smooth group... skipping\n");
        skipBytes(fd, chnk.chunkSize - CHUNK_HDR_SIZE); // skip for now
      }
      break;
    case MSH_MAT_GROUP:
      {
        char *material_name = readStr(fd);
        uint16_t nfaces = readU16(fd);
        short facenum[nfaces];
        for (int i = 0 ; i < nfaces ; i ++) {
          facenum[i] = readU16(fd);
        }
        assert(chnk.chunkSize == CHUNK_HDR_SIZE + strlen(material_name) + 1 + 2 + nfaces * 2);
        fprintf(stderr, "...got mesh material group \"%s\"\n", material_name);
        free(material_name);
      }
      break;
    default:
      skipBytes(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      m3derror("invalid object chunk");
    }
    remBytes -= chnk.chunkSize;
  }


  fprintf(stderr, "...got face array\n");
  return -1; // TODO: Fix return value
}
int readTriangles(int fd, size_t maxBytes)
{
  chunk_3ds_t chnk = {0, 0};
  fprintf(stderr, "reading triangles... %d\n", (int)maxBytes);
  size_t remBytes = maxBytes;

  while (remBytes > 0 && (readChunkHeader(&chnk, fd) == 0)) {
    switch (chnk.chunkId) {
    case POINT_ARRAY:
    {
      uint16_t npoints = readU16(fd);
      struct {
        float x, y, z;
      } points[npoints];
      for (int i = 0 ; i < npoints ; i ++) {
        points[i].x = readFloat(fd);
        points[i].y = readFloat(fd);
        points[i].z = readFloat(fd);
      }
      assert(chnk.chunkSize == CHUNK_HDR_SIZE + 2 + npoints * 4 * 3);
      fprintf(stderr, "...got point array\n");
    }
    break;
    case POINT_FLAG_ARRAY:
    {
      uint16_t nflags = readU16(fd);
      uint16_t flags[nflags];
      for (int i = 0 ; i < nflags ; i ++) {
        flags[i] = readU16(fd);
      }
      assert(chnk.chunkSize == CHUNK_HDR_SIZE + 2 + nflags * 2);
      fprintf(stderr, "...got flag array\n");
    }
    break;
    case FACE_ARRAY:
      readFaces(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      break;
    case TEX_VERTS:
    {
      uint16_t nverts = readU16(fd);
      struct {
        float x, y;
      } vertices[nverts];

      for (int i = 0 ; i < nverts ; i ++) {
        vertices[i].x = readFloat(fd);
        vertices[i].y = readFloat(fd);
      }
      assert(chnk.chunkSize == CHUNK_HDR_SIZE + 2 + nverts * 4 * 2);

      fprintf(stderr, "...got tex vertices\n");

    }
      break;
    case MESH_MATRIX:
    {
      float matrix[4][3];
      for (int i = 0 ; i < 4; i ++) {
        for (int j = 0 ; j < 3 ; j ++) {
          matrix[i][j] = readFloat(fd);
        }
      }
      assert(chnk.chunkSize == CHUNK_HDR_SIZE + 4 * 3 * 4);

      fprintf(stderr, "...mesh matrix\n");

    }
    break;
    case MESH_COLOR:
    {
      uint16_t color_index = readU16(fd);
      assert(chnk.chunkSize == CHUNK_HDR_SIZE + 2);
      fprintf(stderr, "...mesh colour index\n");
    }
      break;
    case MESH_TEXTURE_INFO:
    {
      uint16_t map_type = readU16(fd);
      float x_tiling = readFloat(fd);
      float y_tiling = readFloat(fd);
      float icon_x = readFloat(fd);
      float icon_y = readFloat(fd);
      float icon_z = readFloat(fd);
      float matrix[4][3];
      for (int i = 0 ; i < 4; i ++) {
        for (int j = 0 ; j < 3 ; j ++) {
          matrix[i][j] = readFloat(fd);
        }
      }

      float scaling = readFloat(fd);
      float plan_icon_w = readFloat(fd);
      float plan_icon_h = readFloat(fd);
      float cyl_icon_h = readFloat(fd);
      assert(chnk.chunkSize == CHUNK_HDR_SIZE + 2 + 5 * 4 + 4 * 3 * 4 + 4 * 4);

      fprintf(stderr, "...mesh texture info\n");

    }
      break;
    default:
      skipBytes(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      m3derror("invalid object chunk");
    }
    remBytes -= chnk.chunkSize;
  }
  return 0;
}
int readObject(int fd, size_t maxBytes)
{
  chunk_3ds_t chnk = {0, 0};
  size_t remBytes = maxBytes;

  char *str = readStr(fd);
  remBytes -= (strlen(str) + 1);
  fprintf(stderr, "NAMED_OBJECT \"%s\": %d\n", str, chnk.chunkSize);

  while (remBytes > 0 && (readChunkHeader(&chnk, fd) == 0)) {
    switch (chnk.chunkId) {
    case N_TRI_OBJECT:
      readTriangles(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      break;
    case N_DIRECT_LIGHT:
    case OBJ_DOESNT_CAST:
    case N_CAMERA:
    case OBJ_HIDDEN:
      fprintf(stderr, "skipping unknown object chunk 0x%x\n", chnk.chunkId);
      skipBytes(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      break;
    default:
      skipBytes(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      m3derror("invalid object chunk");
    }
    remBytes -= chnk.chunkSize;
  }

  free(str);
  return 0;
}
int
readMaterial(int fd, size_t maxBytes)
{
  chunk_3ds_t chnk = {0, 0};
  size_t remBytes = maxBytes;
  while (remBytes > 0 && (readChunkHeader(&chnk, fd) == 0)) {
    switch (chnk.chunkId) {
    case MAT_NAME:
      {
        char *str = readStr(fd);
        assert(chnk.chunkSize == strlen(str) + 1 + CHUNK_HDR_SIZE);
        fprintf(stderr, "material \"%s\" defined\n", str);
        free(str);
      }
      break;
    default:
      fprintf(stderr, "skipping unknown material chunk 0x%x\n", chnk.chunkId);
      skipBytes(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
    }
    remBytes -= chnk.chunkSize;
  }
  return 0;
}

int
readData(int fd, size_t maxBytes)
{
  chunk_3ds_t chnk = {0, 0};
  size_t remBytes = maxBytes;

  while (remBytes > 0 && (readChunkHeader(&chnk, fd) == 0)) {
    switch (chnk.chunkId) {
    case NAMED_OBJECT:
      readObject(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      break;
    case MESH_VERSION:
      {
        uint32_t vers = readU32(fd);
        fprintf(stderr, "mesh version: %u\n", vers);
      }
      break;
    case MAT_ENTRY:
      readMaterial(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      break;
    default:
      fprintf(stderr, "skipping unknown data chunk 0x%x\n", chnk.chunkId);
      skipBytes(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
    }
    remBytes -= chnk.chunkSize;
  }
  return 0; // TODO: Check return values
}

int
readMainChunk(int fd, size_t maxBytes)
{
  chunk_3ds_t chnk = {0, 0};
  size_t remBytes = maxBytes;
  while (remBytes > 0 && (readChunkHeader(&chnk, fd) == 0)) {
    switch (chnk.chunkId) {
    case MDATA:
      readData(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      break;
    case M3D_VERSION:
      {
        uint32_t vers = readU32(fd);
        fprintf(stderr, "3ds version: %u\n", vers);
      }
      break;
    default:
      fprintf(stderr, "skipping unknown main chunk 0x%x\n", chnk.chunkId);
      skipBytes(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
    }
    remBytes -= chnk.chunkSize;
  }
  return 0;
}

int
load_3ds(const char *fileName)
{
  int fd = open(fileName, O_RDONLY);
  if (fd == -1) {
    return -1;
  }

  if (setjmp(m3d_err)) {
    // Error
    return -1;
  } else {
    chunk_3ds_t chnk = {0, 0};

    if (readChunkHeader(&chnk, fd) == 0) {
      if (chnk.chunkId == M3DMAGIC) {
        readMainChunk(fd, chnk.chunkSize - CHUNK_HDR_SIZE);
      } else {
        m3derror("invalid main chunk");
      }
    }
  }

  return 0;
}


#ifdef TESTLOADER
// Simple test program
//cc -DTESTLOADER -DLITTLE_ENDIAN -I../src ../src/parsers/3ds.c

int
main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "no file given\n");
  }

  if (load_3ds(argv[1]) == 0) {

//  if (load_3ds(argv[1]) == 0) {
    fprintf(stderr, "successfully read 3ds file\n");
  } else {
    fprintf(stderr, "failed to read 3ds file\n");
  }
  return 0;
}
#endif
