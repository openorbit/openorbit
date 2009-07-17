/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef TEXTURE_
#define TEXTURE_
#ifdef __cplusplus
extern "C" {
#endif 

#include "SDL_opengl.h"
typedef struct {
    GLint width, height;
    GLuint texId;
    GLenum texType;
    GLint bytesPerTex;
    void *data;
} OOtexture;

/*! \brief initialises the texture manager objects */
void    ooTexInit(void);

/*! \brief loads a named resource texture file */
int     ooTexLoad(const char *key, const char *name);
/*! \brief binds the texture specified by key as the current texture
 
    Do not use the bind function to much, it is acceptable to use it in non
    performant rendering code (e.g. user interfaces). In normal cases, use
    tex_num and cache the texid.
 */
int     ooTexBind(const char *key);

/*! \brief Get the OpenGL texture name associated with the key */    
GLuint  ooTexNum(const char *key);
/*! \brief Remove a texture from the global texture dictionary.
 
    Also deletes the texture name 
 */
int     ooTexUnload(const char *key);

OOtexture *ooTexGet(const char *key);

#ifdef __cplusplus
}
#endif 
#endif /*TEXTURE_*/
