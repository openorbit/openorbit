/*
 Copyright 2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

/* Rewrite of the object manager with the following considerations:
    KISS
    strcmp will be vectorised (i.e. 16 bytes per instruction)
    Property lists will be short
    Classes are defined once and this initialisation is not performance hungry
    Log(n) performance for looking up object info is really ok
    We will mostly work from objects, the user should not care about classes
      except for perhaps registering them
    Meta interfaces was a stupid idea, the interfaces will now be attached to
      a single class only.
    Interfaces may be a single function pointer or a struct, it is up to the
      interface requester to reinterprete the pointer
*/

#include <openorbit/openorbit.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
//#include <openorbit/log.h>

#include <gencds/object-manager2.h>


OMcontext*
omCtxtNew(void)
{
  OMcontext *ctxt = malloc(sizeof(OMcontext));
  ctxt->classes = hashtable_new_with_str_keys(1024);
  ctxt->objects = omTreeNew();
  return ctxt;
}

OMtree*
omTreeNew(void)
{
  OMtree *tree = malloc(sizeof(OMtree));
  tree->root = NULL;
  return tree;
}

OMobjdesc*
omObjDescNew(const char *name, OMclass *cls, OOobject *obj)
{
  OMobjdesc *objDesc = malloc(sizeof(OMobjdesc));
  objDesc->name = strdup(name);
  objDesc->cls = cls;
  objDesc->obj = obj;

  return objDesc;
}

static void
omDumpNode(FILE *file, OMtreenode *node, unsigned id)
{
//    struct OMtreenode *parent;
//    struct OMtreenode *left;
//    struct OMtreenode *right;
//    OMobjdesc *objDesc;
//    int8_t balanceFactor;


  fprintf(file, "\tnode%d [label=\"{<parent> parent|%p:%d|{<left> left|<right> right}}\"];\n", id, (void*)node->key, node->balance);

  if (node->left) {
    fprintf(file, "\tnode%d:left->node%d:parent;\n", id, id << 1);
    omDumpNode(file, node->left, id << 1);
  }

  if (node->right) {
    fprintf(file, "\tnode%d:right->node%d:parent;\n", id, id << 1 | 1);
    omDumpNode(file, node->right, id << 1 | 1);
  }
}

// dump graphwis dot rep of tree
void
omDbgDumpTree(FILE *file, OMtree *tree)
{
  assert(tree != NULL);
  assert(tree->root != NULL);

  OMtreenode *root = tree->root;

  fprintf(file, "digraph T {\n");
  fprintf(file, "\tnode [shape=record];\n");

  unsigned node = 1;
  fprintf(file, "\tnode%d [label=\"{<parent> parent|%p:%d|{<left> left|<right> right}}\"];\n", node, (void*)root->key, root->balance);

  if (root->left) {
    fprintf(file, "\tnode%d:left->node%d:parent;\n", node, node << 1);
    omDumpNode(file, root->left, node << 1);
  }

  if (root->right) {
    fprintf(file, "\tnode%d:right->node%d:parent;\n", node, node << 1 | 1);
    omDumpNode(file, root->right, node << 1 | 1);
  }

  fprintf(file, "}\n");
}


void
omAddIface(OMclass *cls, const char *name, void *iface)
{
  if (cls->ifaceAlloc <= cls->ifaceCount) {
    cls->ifaces = realloc(cls->ifaces, sizeof(OMinterface)*(cls->ifaceAlloc*2));
    cls->ifaceAlloc *= 2;
  }

  cls->ifaces[cls->ifaceCount].name = strdup(name);
  cls->ifaces[cls->ifaceCount].ifacePtr = iface;

  cls->ifaceCount ++;
}

void*
omGetIface(OMclass *cls, const char *name)
{
  for (size_t i = 0 ; i < cls->ifaceCount ; i ++) {
    if (!strcmp(cls->ifaces[i].name, name)) {
      return cls->ifaces[i].ifacePtr;
    }
  }

  return NULL;
}



void
omAddProp(OMclass *cls, const char *name, OMpropkind typ, size_t offset)
{
  if (cls->propAlloc <= cls->propCount) {
    cls->props = realloc(cls->props, sizeof(OMproperty)*(cls->propAlloc * 2));
    //ooLogFatalIfNull(cls, "Failed to realloc class object");

    cls->propAlloc *= 2;
  }

  cls->props[cls->propCount].name = strdup(name);
  cls->props[cls->propCount].type = typ;
  cls->props[cls->propCount].offset = offset;

  cls->propCount ++;
}
#if 0
void
omTreeRotateLeft(OMtreenode *root)
{
  OMtreenode *pivot = root->right;
  root->right = pivot->right; root->right->parent = root;
  pivot->right = root;

  if (root->parent->left = root) {
    root->parent->left = pivot;
  } else {
    root->parent->right = pivot;
  }

  root->parent = pivot;
}

void
omTreeRotateRight(OMtreenode *root)
{
  OMtreenode *pivot = root->left;
  root->left = pivot->right; root->left->parent = root;
  pivot->right = root;

  if (root->parent->left = root) {
    root->parent->left = pivot;
  } else {
    root->parent->right = pivot;
  }

  root->parent = pivot;
}


void
omTreeRebalance(OMtreenode *node) {

  if (node->balanceFactor > 1) {
    omTreeRotateLeft(node);
  } else if (node->balanceFactor < -1) {
    omTreeRotateRight(node);
  }
}

static void
omTreeInsert_(OMtreenode * restrict tree, OMtreenode * restrict node)
{
  if (node->key < tree->key) {
    if (tree->left == NULL) {
      tree->left = node;
    } else {
      omTreeInsert_(tree->left, node);
    }
  } else {
    if (tree->right == NULL) {
      tree->right = node;
    } else {
      omTreeInsert_(tree->right, node);
    }
  }
}

void
omTreeInsert(OMtree *tree, uintptr_t key, void *data)
{
  assert(tree != NULL);
  assert(data != NULL);

  OMtreenode *root = tree->root;

  OMtreenode *node = malloc(sizeof(OMtreenode));

  node->data = data;
  node->key = key;
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->balance = 0;

  if (tree->root == NULL) {
    tree->root = node;
    return;
  }

  OMtreenode *next;

  while (root) {
    if (root->key < key) {
      next = root->right;
      if (! next) {
        root->right = node;
        node->parent = root;
        root->balance ++;

        if (root->parent) root->parent->balance ++;
        break;
      }
    } else if (root->key > key) {
      next = root->left;
      if (! next) {
        root->left = node;
        node->parent = root;
        root->balance --;
        if (root->parent) root->parent->balance --;
        break;
      }
    } else {
      //ooLogError("objectree: can't insert multiple copies");
      return;
    }

    root = next;
  }

  omTreeRebalance(root);
}
#endif
OMobjdesc*
omTreeLookup(const OMtreenode *root, OOobject *obj)
{
  while (root) {
    if (root->key < (uintptr_t)obj) {
      root = root->left;
    } else if (root->key > (uintptr_t)obj) {
      root = root->right;
    } else {
      return root->data;
    }
  }
  return NULL;
}

void*
omObjGetIface(OOobject *obj, const char *iface)
{
  OMobjdesc *desc = omTreeLookup(NULL, obj);
  if (desc == NULL) return NULL;
  return omGetIface(desc->cls, iface);
}

OMclass*
omGetClassObject(const char *cls)
{
  return NULL;
}

void*
omNewObject(OMcontext *ctxt, const char *clsName)
{
  OMclass *cls = omGetClassObject(clsName);
  if (cls == NULL) return NULL;
  return malloc(cls->objSize);
}

OMclass*
omNewClass(OMcontext *ctxt, const char *clsName, size_t objLen)
{
  OMclass *cls = malloc(sizeof(OMclass));
  cls->name = strdup(clsName);
  cls->objSize = objLen;

  cls->ifaceAlloc = 4;
  cls->ifaceCount = 0;
  cls->ifaces = calloc(4, sizeof(OMinterface));
  cls->propAlloc = 4;
  cls->propCount = 0;
  cls->props = calloc(4, sizeof(OMproperty));

  hashtable_insert(ctxt->classes, clsName, cls);

  return cls;
}

OMtreenode*
omAVLLeftSingle(OMtreenode * restrict root)
{
  OMtreenode *newRoot = root->right;
  root->right = newRoot->left;
  newRoot->left = root;

  return newRoot;
}

OMtreenode*
omAVLRightSingle(OMtreenode * restrict root)
{
  OMtreenode *newRoot = root->left;
  root->left = newRoot->right;
  newRoot->right = root;

  return newRoot;
}

OMtreenode*
omAVLLeftDouble(OMtreenode * restrict root)
{
  OMtreenode *newRoot = root->right->left;

  root->right->left = newRoot->right;
  newRoot->right = root->right;
  root->right = newRoot;

  newRoot = root->right;
  root->right = newRoot->left;
  newRoot->left = root;

  return newRoot;
}

OMtreenode*
omAVLRightDouble(OMtreenode * restrict root)
{
  OMtreenode *newRoot = root->left->right;

  root->left->right = newRoot->left;
  newRoot->left = root->left;
  root->left = newRoot;

  newRoot = root->left;
  root->left = newRoot->right;
  newRoot->right = root;

  return newRoot;
}


//OMtreenode*
//omAVLInsert(OMtreenode * restrict tree, OMtreenode * restrict node)
//{
//  if (tree == NULL) {
//    return node;
//  } else if (node->key < tree->key) {
//    tree->left = omAVLInsert(tree->left, node);
//  } else {
//    tree->right = omAVLInsert(tree->right, node);
//  }
//
//  return tree;
//}

void
omAVLUpdateBalanceLeft(OMtreenode *root, int8_t bal)
{
  OMtreenode *n = root->left;
  OMtreenode *nn = n->right;

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

void
omAVLUpdateBalanceRight(OMtreenode *root, int8_t bal)
{
  OMtreenode *n = root->right;
  OMtreenode *nn = n->left;

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

OMtreenode*
omAVLInsertBalanceLeft(OMtreenode *root)
{
  OMtreenode *n = root->left;
  if (n->balance == -1) {
    root->balance = 0;
    n->balance = 0;
    root = omAVLRightSingle(root);
  } else {
    omAVLUpdateBalanceLeft(root, -1);
    root = omAVLRightDouble(root);
  }
  return root;
}



OMtreenode*
omAVLInsertBalanceRight(OMtreenode *root)
{
  OMtreenode *n = root->right;
  if (n->balance == 1) {
    root->balance = 0;
    n->balance = 0;
    root = omAVLLeftSingle(root);
  } else {
    omAVLUpdateBalanceRight(root, 1);
    root = omAVLLeftDouble(root);
  }
  return root;
}

OMtreenode*
omAVLInsert_(OMtreenode *root, OMtreenode *node, bool *done)
{
  if (root == NULL) {
    root = node;
  } else if (node->key < root->key) {
    root->left = omAVLInsert_(root->left, node, done);

    if (!*done) {
      root->balance --;

      if (root->balance == 0) {
        *done = true;
      } else if ((root->balance < -1) || (root->balance > 1)) {
        root = omAVLInsertBalanceLeft(root);
        *done = true;
      }
    }
  } else {
    root->right = omAVLInsert_(root->right, node, done);

    if (!*done) {
      root->balance ++;

      if (root->balance == 0) {
        *done = true;
      } else if ((root->balance < -1) || (root->balance > 1)) {
        root = omAVLInsertBalanceRight(root);
        *done = true;
      }
    }
  }

  return root;
}

void
omAVLInsert(OMtree *tree, OMtreenode *node)
{
  bool done = false;
  tree->root = omAVLInsert_(tree->root, node, &done);
}
void
omTreeInsert2(OMtree *tree, uintptr_t key, void *data)
{
  OMtreenode *node = malloc(sizeof(OMtreenode));

  node->data = data;
  node->key = key;
  node->left = NULL;
  node->right = NULL;
  //node->parent = NULL;
  node->balance = 0;


  omAVLInsert(tree, node);
}
