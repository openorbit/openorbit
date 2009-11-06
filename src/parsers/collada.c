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


#include "collada.h"

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>

model_t*
collada_load(const char *path)
{
  LIBXML_TEST_VERSION
  
  model_t *model = NULL;
  xmlTextReaderPtr reader = xmlReaderForFile(path, NULL, 0);
  if (reader == NULL) {
    fprintf(stderr, "no such file: '%s'\n", path);
    return NULL;
  }
  
  
  int ret = xmlTextReaderRead(reader);
  while (ret == 1) {
    // Handle node
    
    ret = xmlTextReaderRead(reader);
  }

  if (ret != 0) {
    fprintf(stderr, "error parsing xml file: '%s'\n", path);
  }
  
  xmlFreeTextReader(reader);
  xmlCleanupParser();
  return model;
}
