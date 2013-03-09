
/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "moduleinit.h"
#include <stdio.h>

#include <gencds/avl-tree.h>
#include <gencds/array.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "palloc.h"

static obj_array_t roots;
static avl_tree_t *modules;
typedef struct module_t {
  unsigned dep_count;
  const char *name;
  module_init_t init;
  str_array_t deps;
  obj_array_t dependees;
} module_t;

module_t*
module_alloc(const char *name, module_init_t module_init)
{
  module_t *mod = smalloc(sizeof(module_t));
  bzero(mod, sizeof(module_t));
  mod->name = strdup(name);
  str_array_init(&mod->deps);
  obj_array_init(&mod->dependees);
  mod->init = module_init;
  mod->dep_count = 0;
  return mod;
}

void
module_register(const char *module_name, module_init_t module_init,
                char **dependencies)
{
  if (modules == NULL) modules = avl_str_new();

  module_t *mod = module_alloc(module_name, module_init);

  for (int i = 0 ; dependencies[i] != NULL ; i ++) {
    char *dep = dependencies[i];
    str_array_push(&mod->deps, dep);
    mod->dep_count ++;
  }
  avl_insert(modules, module_name, mod);
}

void
module_recurse(avl_str_node_t *node)
{
  if (node->super.left) module_recurse((avl_str_node_t*)node->super.left);
  if (node->super.right) module_recurse((avl_str_node_t*)node->super.right);

  module_t *mod = node->super.data;


  ARRAY_FOR_EACH(i, mod->deps) {
    module_t *mod2 = avl_find(modules, ARRAY_ELEM(mod->deps, i));
    if (mod2) {
      obj_array_push(&mod2->dependees, mod);
    }
  }
}

void
module_find_roots(avl_str_node_t *node)
{
  if (node->super.left) module_find_roots((avl_str_node_t*)node->super.left);
  if (node->super.right) module_find_roots((avl_str_node_t*)node->super.right);

  module_t *mod = node->super.data;

  if (mod->dep_count == 0) obj_array_push(&roots, mod);
}


// Call this before anything is done in main except potentially loading of
// dynamic libraries
void
module_initialize(void)
{
  obj_array_init(&roots);
  // First compute the dependency count and the dependees in all modules
  module_recurse((avl_str_node_t*)modules->root);
  module_find_roots((avl_str_node_t*)modules->root);

  // Now, time to invoke the modules, we start with the roots, then we
  // push additional modules on the roots array when their dep counts
  // are reduced to 0
  ARRAY_FOR_EACH(i, roots) {
    module_t *mod = ARRAY_ELEM(roots, i);
    mod->init();
    ARRAY_FOR_EACH(j, mod->dependees) {
      module_t *dependee = ARRAY_ELEM(mod->dependees, j);
      dependee->dep_count --;
      if (dependee->dep_count == 0) {
        obj_array_push(&roots, dependee);
      }
    }
  }
}
