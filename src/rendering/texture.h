/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
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


#ifdef __cplusplus
}
#endif 
#endif /*TEXTURE_*/
