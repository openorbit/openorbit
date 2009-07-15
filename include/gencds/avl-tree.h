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

#ifndef AVL_TREE_H_VVXFFKFK
#define AVL_TREE_H_VVXFFKFK

#include <stdint.h>
#include <stdio.h>

/*!
    Generic implementation of an avl tree. It allows you to store pointers in
    the tree that is accessed by an integer identifier (that is large enough to
    hold a pointer).
*/

typedef struct avl_node_t {
  struct avl_node_t *left;
  struct avl_node_t *right;
  uintptr_t key; // Can be any integer, also a pointer
  void *data;
  int8_t balance; // Constrained to +/-1
} avl_node_t;

typedef struct avl_tree_t {
  avl_node_t *root;
} avl_tree_t;

typedef void (*avl_func)(void*);

avl_tree_t* avl_new();
void avl_delete(avl_tree_t *tree);
void avl_apply(avl_tree_t *tree, avl_func f);

void* avl_find(avl_tree_t *tree, uintptr_t key);
void avl_remove(avl_tree_t *tree, uintptr_t key);
void avl_insert(avl_tree_t *tree, uintptr_t key, void *data);
void avl_dump_tree(FILE *file, avl_tree_t *tree);

#endif /* end of include guard: AVL_TREE_H_VVXFFKFK */
