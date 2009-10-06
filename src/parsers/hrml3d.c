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

#include "hrml3d.h"
#include "hrml.h"

static void
hrml3d_model(model_t *model, HRMLobject *obj)
{
  
}

int
hrml3d_load(model_t *model, const char *fileName)
{
  HRMLdocument *doc = hrmlParse(fileName);
  if (doc == NULL) {
    return -1;
  }
  
  HRMLobject *rootNode = hrmlGetRoot(doc);
  
  if (!strcmp(rootNode->name, "h3d")) {
    for (HRMLobject *obj = rootNode->children; obj != NULL; obj = obj->next) {
      
    }
  }
}
