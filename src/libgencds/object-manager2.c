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
    created by the Initial Developer are Copyright (C) 2008 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)openorbit.org>.

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
//#include "log.h"

#include <gencds/object-manager2.h>
static void
omDumpNode(OMtreenode *node, unsigned id)
{
//    struct OMtreenode *parent;
//    struct OMtreenode *left;
//    struct OMtreenode *right;
//    OMobjdesc *objDesc;
//    int8_t balanceFactor;


  printf("\tnode%d [label=\"{<parent> parent|%p|{<left> left|<right> right}}\"];\n", id, (void*)node->objDesc);
  
  if (node->left) {
    printf("\tnode%d:left->node%d:parent;\n", id, id << 1);
    omDumpNode(node->left, id << 1); 
  }

  if (node->right) {
    printf("\tnode%d:right->node%d:parent;\n", id, id << 1 | 1);
    omDumpNode(node->right, id << 1 | 1);
  }
}

// dump graphwis dot rep of tree
void
omDbgDumpTree(OMtreenode *root)
{
  printf("digraph T {\n");
  printf("\tnode [shape=record];\n");
  
  unsigned node = 1;
  printf("\tnode%d [label=\"{<parent> parent|%p|{<left> left|<right> right}}\"];\n", node, (void*)root->objDesc);
  
  if (root->left) {
    printf("\tnode%d:left->node%d:parent;\n", node, node << 1);
    omDumpNode(root->left, node << 1);
  }

  if (root->right) {
    printf("\tnode%d:right->node%d:parent;\n", node, node << 1 | 1);
    omDumpNode(root->right, node << 1 | 1);
  }
  
  printf("}\n");
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
omAddProp(OMclass **cls, const char *name, OMpropkind typ, size_t offset)
{
  if ((*cls)->propCountAlloc <= (*cls)->propCount) {
    cls = realloc(cls, sizeof(OMclass) +
                       sizeof(OMproperty)*((*cls)->propCountAlloc * 2));
    //ooLogFatalIfNull(cls, "Failed to realloc class object");
  
    (*cls)->propCountAlloc *= 2;
  }
  
  (*cls)->props[(*cls)->propCount].name = strdup(name);
  (*cls)->props[(*cls)->propCount].type = typ;
  (*cls)->props[(*cls)->propCount].offset = offset;

  (*cls)->propCount ++;
}

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

void
omTreeInsert(OMtreenode *root, OMobjdesc *desc)
{
  OMtreenode *node = malloc(sizeof(OMtreenode));
  node->balanceFactor = 0;
  node->objDesc = desc;
  node->left = NULL;
  node->right = NULL;
  node->balanceFactor = 0;
  
  OMtreenode *next;
  
  while (root) {
    if (root->objDesc->obj < desc->obj) {
      next = root->right;
      if (! next) {
        root->right = node;
        node->parent = root;
        root->balanceFactor ++;
        root->parent->balanceFactor ++;
        break;
      }
    } else if (root->objDesc->obj > desc->obj) {
      next = root->left;
      if (! next) {
        root->left = node;
        node->parent = root;
        root->balanceFactor --;
        root->parent->balanceFactor --;
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

OMobjdesc*
omTreeLookup(const OMtreenode *root, OOobject *obj)
{  
  while (root) {
    if (root->objDesc->obj < obj) {
      root = root->left;
    } else if (root->objDesc->obj > obj) {
      root = root->right;
    } else {
      return root->objDesc;
    }
  }
}

void*
omObjGetIface(OOobject *obj, const char *iface)
{
  OMobjdesc *desc = omTreeLookup(NULL, obj);
  
  return omGetIface(desc->cls, iface);
}

OMclass*
omGetClassObject(const char *cls)
{
  return NULL;
}

OOobject*
omNewObject(const char *clsName)
{
  OMclass *cls = omGetClassObject(clsName);
  
  return malloc(cls->objSize);
}

OMclass*
omNewClass(const char *clsName, size_t objLen)
{
  OMclass *cls = malloc(sizeof(OMclass) + sizeof(OMproperty)*4);
  cls->name = strdup(clsName);
  cls->objSize = objLen;

  cls->ifaceAlloc = 4;
  cls->ifaceCount = 0;
  cls->ifaces = calloc(4, sizeof(OMinterface));
  cls->propCountAlloc = 4;
  cls->propCount = 0;
  
  return cls;
}
