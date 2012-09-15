/*
  Copyright 2006,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef TEXTURE_
#define TEXTURE_
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include "rendering/types.h"

/*! \brief Loads a named resource texture file
 *
 * The function loads the file with the given name and binds it to the key. If
 * the key has already been bound, the memoized texture is returned and no file
 * is loaded.
 *
 * \param key Name of resource file
 * \result Returns the texture number. Will return 0 on a failed load.
 */
sg_texture_t* sg_load_texture(const char *key);
/*! \brief binds the texture specified by key as the current texture

    Do not use the bind function to much, it is acceptable to use it in non
    performant rendering code (e.g. user interfaces). In normal cases, use
    tex_num and cache the texid.
 */
void sg_texture_bind(sg_texture_t *tex, sg_shader_t *shader);

  /*! \brief Get the OpenGL texture name associated with the key */
GLuint sg_texture_get_id(sg_texture_t *tex);
/*! \brief Remove a texture from the global texture dictionary.

    Also deletes the texture name
 */
void sg_texture_unload(sg_texture_t *tex);

sg_texture_t *sg_find_texture(const char *key);

#ifdef __cplusplus
}
#endif
#endif /*TEXTURE_*/
