/*
 Copyright 2014 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <stdio.h>

#include <OpenGL/gl3.h>
#include <jansson.h>

#include "rendering/gltf.h"

void
read_uniform(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "semantic")) {
    } else if (!strcmp(key, "symbol")) {
    } else if (!strcmp(key, "type")) {
    }
  }
}


void
read_technique(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "parameters")) {
    } else if (!strcmp(key, "pass")) {
    }
  }
}

void
read_states(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "blendEnable")) {
    } else if (!strcmp(key, "blendEquation")) {
    } else if (!strcmp(key, "blendEquationSeparate")) {

    } else if (!strcmp(key, "blendFunc")) {
    } else if (!strcmp(key, "blendFuncSeparate")) {
    } else if (!strcmp(key, "colorMask")) {
    } else if (!strcmp(key, "cullFace")) {
    } else if (!strcmp(key, "cullFaceEnable")) {
    } else if (!strcmp(key, "depthFunc")) {
    } else if (!strcmp(key, "depthMask")) {
    } else if (!strcmp(key, "depthRange")) {
    } else if (!strcmp(key, "depthTestEnable")) {
    } else if (!strcmp(key, "ditherEnable")) {
    } else if (!strcmp(key, "frontFace")) {
    } else if (!strcmp(key, "lineWidth")) {
    } else if (!strcmp(key, "pointSize")) {
    } else if (!strcmp(key, "polygonOffset")) {
    } else if (!strcmp(key, "polygonOffsetFillEnable")) {
    } else if (!strcmp(key, "sampleAlphaToCoverageEnable")) {
    } else if (!strcmp(key, "sampleCoverage")) {
    } else if (!strcmp(key, "sampleCoverageEnable")) {
    } else if (!strcmp(key, "scissor")) {
    } else if (!strcmp(key, "scissorTestEnable")) {
    } else if (!strcmp(key, "stencilFunc")) {
    } else if (!strcmp(key, "stencilFuncSeparate")) {
    } else if (!strcmp(key, "stencilMask")) {
    } else if (!strcmp(key, "stencilOp")) {
    } else if (!strcmp(key, "stencilOpSeparate")) {
    } else if (!strcmp(key, "stencilTestEnable")) {
    }
  }
}


void
read_shader(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "name")) {
    } else if (!strcmp(key, "path")) {
    }
  }
}


void
read_program(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "attributes")) {
    } else if (!strcmp(key, "fragmentShader")) {
    } else if (!strcmp(key, "uniforms")) {
    } else if (!strcmp(json_string_value(obj), "vertexShader")) {
    }
  }
}



void
read_primitive(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "indices")) {
    } else if (!strcmp(key, "material")) {
    } else if (!strcmp(key, "primitive")) {
      if (!strcmp(json_string_value(obj), "POINTS")) {

      } else if (!strcmp(json_string_value(obj), "LINE_STRIP")) {
      } else if (!strcmp(json_string_value(obj), "LINE_LOOP")) {
      } else if (!strcmp(json_string_value(obj), "LINES")) {
      } else if (!strcmp(json_string_value(obj), "TRIANGLE_STRIP")) {
      } else if (!strcmp(json_string_value(obj), "TRIANGLE_FAN")) {
      } else if (!strcmp(json_string_value(obj), "TRIANGLES")) {
      }

    } else if (!strcmp(key, "semantics")) {
    }
  }
}


void
read_node(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "children")) {
    }  else if (!strcmp(key, "children")) {
    } else if (!strcmp(key, "matrix")) {
      double defaultMatrix[] = {1.0, 0.0, 0.0, 0.0,
                                0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0,
                                0.0, 0.0, 0.0, 1.0};

    } else if (!strcmp(key, "meshes")) {
    } else if (!strcmp(key, "name")) {
    }
  }
}

void
read_material(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "name")) {
    } else if (!strcmp(key, "parameters")) {
    } else if (!strcmp(key, "technique")) {
    } else if (!strcmp(key, "techniques")) {
    }
  }
}


void
read_indices(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "bufferView")) {
    } else if (!strcmp(key, "byteOffset")) {
    } else if (!strcmp(key, "count")) {
    } else if (!strcmp(key, "type")) {
      if (!strcmp(json_string_value(obj), "UNSIGNED_SHORT")) {
      }
    }
  }
}


void
read_image(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "path")) {
    } else if (!strcmp(key, "generateMipmap")) {
    } else if (!strcmp(key, "name")) {
    }
  }
}


void
read_geographicLocation(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "longitude")) {
    } else if (!strcmp(key, "latitude")) {
    } else if (!strcmp(key, "altitude")) {
      // default = 0
    } else if (!strcmp(key, "altitudeMode")) {
      if (!strcmp(json_string_value(obj), "absolute")) {
      } else if (!strcmp(json_string_value(obj), "relativeToGround")) {
      }
    }
  }
}

void
read_bufferView(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "buffer")) {
    } else if (!strcmp(key, "byteOffset")) {
    } else if (!strcmp(key, "byteLength")) {
    } else if (!strcmp(key, "target")) {
      if (!strcmp(json_string_value(obj), "ARRAY_BUFFER")) {
      } else if (!strcmp(json_string_value(obj), "ELEMENT_ARRAY_BUFFER")) {
      }
    }
  }
}


void
read_buffer(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "path")) {
    } else if (!strcmp(key, "byteLength")) {
    }
  }
}


void
read_asset(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "copyright")) {
    } else if (!strcmp(key, "geographicLocation")) {
    }
  }
}

void
read_light(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "ambient")) {
    } else if (!strcmp(key, "directional")) {
    } else if (!strcmp(key, "point")) {
    } else if (!strcmp(key, "spot")) {
    } else if (!strcmp(key, "type")) {
    }

  }
}

void
read_texture(json_t *obj)
{
  const char *key;
  json_t *value;

  GLint format = GL_RGBA;
  GLint internalFormat = GL_RGBA;
  GLint target = GL_TEXTURE_2D;
  GLint type = GL_UNSIGNED_BYTE;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "format")) {
      format = json_integer_value(obj);
    } else if (!strcmp(key, "internalFormat")) {
      internalFormat = json_integer_value(obj);
    } else if (!strcmp(key, "sampler")) {
    } else if (!strcmp(key, "source")) {
    } else if (!strcmp(key, "target")) {
      target = json_integer_value(obj);
    } else if (!strcmp(key, "type")) {
      type = json_integer_value(obj);
    }

  }
}
void
read_mesh(json_t *obj)
{
  const char *key;
  json_t *value;

  json_object_foreach(obj, key, value) {
    if (!strcmp(key, "bufferView")) {
    } else if (!strcmp(key, "byteOffset")) {
    } else if (!strcmp(key, "byteStride")) {
    } else if (!strcmp(key, "count")) {
    } else if (!strcmp(key, "type")) {
    } else if (!strcmp(key, "normalized")) {
    } else if (!strcmp(key, "max")) {
    } else if (!strcmp(key, "min")) {
    }

  }
}

sg_object_t*
gltf_load(const char *path)
{
  sg_object_t *obj = NULL;

  json_error_t err;
  json_t* gltf_data = json_load_file(path, 0, &err);

  if (gltf_data) {
    const char *key;
    json_t *value;

    json_object_foreach(gltf_data, key, value) {
      if (!strcmp(key, "allExtensions")) {
      } else if (!strcmp(key, "accessors")) {
      } else if (!strcmp(key, "animations")) {
      } else if (!strcmp(key, "asset")) {
      } else if (!strcmp(key, "buffers")) {
      } else if (!strcmp(key, "bufferViews")) {
      } else if (!strcmp(key, "cameras")) {
      } else if (!strcmp(key, "images")) {
      } else if (!strcmp(key, "lights")) {
      } else if (!strcmp(key, "materials")) {
      } else if (!strcmp(key, "meshes")) {
      } else if (!strcmp(key, "nodes")) {
      } else if (!strcmp(key, "programs")) {
      } else if (!strcmp(key, "samplers")) {
      } else if (!strcmp(key, "scene")) {
      } else if (!strcmp(key, "scenes")) {
      } else if (!strcmp(key, "shaders")) {
      } else if (!strcmp(key, "skins")) {
      } else if (!strcmp(key, "techniques")) {
      } else if (!strcmp(key, "textures")) {
      }
    }


    json_delete(gltf_data);
  }

  return obj;
}
