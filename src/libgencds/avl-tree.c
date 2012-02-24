/*
 Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>
 
 This file is part of Open Orbit. Open Orbit is free software: you can
 redistribute it and/or modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.
 
 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 
 Some files of Open Orbit have relaxed licensing conditions. This file is
 licenced under the 2-clause BSD licence.
 
 Redistribution and use of this file in source and binary forms, with or
 without modification, are permitted provided that the following conditions are
 met:
 
 - Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sysexits.h>
#include <err.h>
#include <gencds/avl-tree.h>

static avl_node_t*
avl_left_single(avl_node_t * restrict root)
{
  avl_node_t *newRoot = root->right;
  root->right = newRoot->left;
  newRoot->left = root;
  
  return newRoot;
}

static avl_node_t*
avl_right_single(avl_node_t * restrict root)
{
  avl_node_t *newRoot = root->left;
  root->left = newRoot->right;
  newRoot->right = root;
  
  return newRoot;
}

static avl_node_t*
avl_left_double(avl_node_t * restrict root)
{
  avl_node_t *newRoot = root->right->left;
  
  root->right->left = newRoot->right;
  newRoot->right = root->right;
  root->right = newRoot;
  
  newRoot = root->right;
  root->right = newRoot->left;
  newRoot->left = root;
  
  return newRoot;
}

static avl_node_t*
avl_right_double(avl_node_t * restrict root)
{
  avl_node_t *newRoot = root->left->right;
  
  root->left->right = newRoot->left;
  newRoot->left = root->left;
  root->left = newRoot;
  
  newRoot = root->left;
  root->left = newRoot->right;
  newRoot->right = root;
  
  return newRoot;
}


static void
avl_update_balance_left(avl_node_t *root, int8_t bal)
{
  avl_node_t *n = root->left;
  avl_node_t *nn = n->right;
  
  if (nn->balance == 0) {
    root->balance = 0;
    n->balance = 0; 
  } else if (nn->balance == bal) {
    root->balance = -bal;
    n->balance = 0;
  } else {
    root->balance = 0;
    n->balance = bal;
  }
  
  nn->balance = 0;
}

static void
avl_update_balance_right(avl_node_t *root, int8_t bal)
{
  avl_node_t *n = root->right;
  avl_node_t *nn = n->left;
  
  if (nn->balance == 0) {
    root->balance = 0;
    n->balance = 0; 
  } else if (nn->balance == bal) {
    root->balance = -bal;
    n->balance = 0;
  } else {
    root->balance = 0;
    n->balance = bal;
  }
  
  nn->balance = 0;
}

static avl_node_t*
avl_insert_balance_left(avl_node_t *root)
{
  avl_node_t *n = root->left;
  if (n->balance == -1) {
    root->balance = 0;
    n->balance = 0;
    root = avl_right_single(root);
  } else {
    avl_update_balance_left(root, -1);
    root = avl_right_double(root);
  }
  return root;
}



static avl_node_t*
avl_insert_balance_right(avl_node_t *root)
{
  avl_node_t *n = root->right;
  if (n->balance == 1) {
    root->balance = 0;
    n->balance = 0;
    root = avl_left_single(root);
  } else {
    avl_update_balance_right(root, 1);
    root = avl_left_double(root);
  }
  return root;
}

static avl_node_t*
avl_insert__(avl_node_t *root, avl_node_t *node, bool *done)
{
  if (root == NULL) {
    root = node;
  } else if (node->key < root->key) {
    root->left = avl_insert__(root->left, node, done);
    
    if (!*done) {
      root->balance --;
      
      if (root->balance == 0) {
        *done = true;
      } else if ((root->balance < -1) || (root->balance > 1)) {
        root = avl_insert_balance_left(root);
        *done = true;
      }
    }
  } else {
    root->right = avl_insert__(root->right, node, done);
    
    if (!*done) {
      root->balance ++;
      
      if (root->balance == 0) {
        *done = true;
      } else if ((root->balance < -1) || (root->balance > 1)) {
        root = avl_insert_balance_right(root);
        *done = true;
      }
    }
  }
  
  return root;
}

static void
avl_insert_(avl_tree_t *tree, avl_node_t *node)
{
  bool done = false;
  tree->root = avl_insert__(tree->root, node, &done);
}

void
avl_insert(avl_tree_t *tree, uintptr_t key, void *data)
{
  avl_node_t *node = malloc(sizeof(avl_node_t));

  node->data = data;
  node->key = key;
  node->left = NULL;
  node->right = NULL;
  node->balance = 0;

  avl_insert_(tree, node);
}


// naive recursive version
static void*
avl_find_node(avl_node_t *node, uintptr_t key)
{
  if (node == NULL) return NULL;
  
  if (node->key < key) {
    return avl_find_node(node->left, key);
  } else if (node->key > key) {
    return avl_find_node(node->right, key);
  } else {
    return node;
  }
}

void*
avl_find(avl_tree_t *tree, uintptr_t key)
{
  avl_node_t *node = tree->root;
  
  return avl_find_node(node, key);
}


void
avl_remove(avl_tree_t *tree, uintptr_t key)
{
  //avl_node_t *node = tree->root;
  assert(0 && "avl_remove not supported yet");
  // TODO: Add remove support
}


static void
avl_dump_node(FILE *file, avl_node_t *node, unsigned id)
{
  fprintf(file,
          "\tnode%d [label=\"{<parent> parent|%p:%d|{<left> left|<right> right}}\"];\n",
          id, (void*)node->key, node->balance);
  
  if (node->left) {
    fprintf(file, "\tnode%d:left->node%d:parent;\n", id, id << 1);
    avl_dump_node(file, node->left, id << 1); 
  }

  if (node->right) {
    fprintf(file, "\tnode%d:right->node%d:parent;\n", id, id << 1 | 1);
    avl_dump_node(file, node->right, id << 1 | 1);
  }
}

// dump graphwis dot rep of tree
void
avl_dump_tree(FILE *file, avl_tree_t *tree)
{
  assert(tree != NULL);
  assert(tree->root != NULL);
  
  avl_node_t *root = tree->root;

  fprintf(file, "digraph T {\n");
  fprintf(file, "\tnode [shape=record];\n");
  
  unsigned node = 1;
  fprintf(file,
          "\tnode%d [label=\"{<parent> parent|%p:%d|{<left> left|<right> right}}\"];\n",
          node, (void*)root->key, root->balance);
  
  if (root->left) {
    fprintf(file, "\tnode%d:left->node%d:parent;\n", node, node << 1);
    avl_dump_node(file, root->left, node << 1);
  }

  if (root->right) {
    fprintf(file, "\tnode%d:right->node%d:parent;\n", node, node << 1 | 1);
    avl_dump_node(file, root->right, node << 1 | 1);
  }
  
  fprintf(file, "}\n");
}


avl_tree_t*
avl_new()
{
  avl_tree_t * tree = malloc(sizeof(avl_tree_t));
  if (tree == NULL) {
    err(EX_SOFTWARE, "allocation of avl tree failed\n");
  }
  tree->root = NULL;
  return tree;
}

void
avl_delete2(avl_node_t *node)
{
  if (node == NULL) {
    return;
  }

  avl_delete2(node->left);
  avl_delete2(node->right);

  free(node);
}

void
avl_delete(avl_tree_t *tree)
{
  avl_delete2(tree->root);
  free(tree);
}

static void
avl_apply2(avl_node_t *node, avl_func f)
{
  if (node == NULL) {
    return;
  }
  
  f(node);
  
  avl_apply2(node->left, f);
  avl_apply2(node->right, f);
}


void
avl_apply(avl_tree_t *tree, avl_func f)
{
  avl_apply2(tree->root, f);
}
