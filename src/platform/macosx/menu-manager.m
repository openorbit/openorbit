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

#include <stdlib.h>
#include "menu-manager.h"

#import <Cocoa/Cocoa.h>

// Wrapper menu item class that allows the use of C-functions as menu actions
@interface OOMenuItem : NSMenuItem {
  menu_func_t func;
  void *arg;
}
- (OOMenuItem*) initWithTitle:(const char*)title
                       action:(menu_func_t)function
                          arg:(void*)arg;
- (void)menuDispatch:(id)sender;
@end

@implementation OOMenuItem
- (void)menuDispatch:(id)sender
{
  if (func) func(arg);
}

- (OOMenuItem*) initWithTitle:(const char*)title
                       action:(menu_func_t)function
                          arg:(void*)argument
{
  NSString *menuName = [NSString stringWithFormat:@"%s", title];

  self = [super initWithTitle:menuName
                       action:@selector(menuDispatch:)
                keyEquivalent:@""];
  if (self) {
    self.target = self;
    func = function;
    arg = argument;
  }

  return self;
}

@end

menu_t*
menu_get(const char *menu_name)
{
  char menuPathBuff[strlen(menu_name)+1];
  char *menuPath = &menuPathBuff[0];
  strcpy(menuPathBuff, menu_name);
  NSMenu *menu = [[NSApplication sharedApplication] mainMenu];
  NSMenuItem *item = nil;

  char *menuItemName = strsep(&menuPath, MENU_SEP);

  while (menuItemName) {
    NSString *menuId = [NSString stringWithFormat:@"%s", menu_name];
    item = [menu itemWithTitle:menuId];
    menu = [item submenu];
    menuItemName = strsep(&menuPath, MENU_SEP);
  }
  return (menu_t*)item;
}

menu_t*
menu_new(menu_t *parent, const char *name, menu_func_t f, void *arg)
{
  NSMenuItem *parentItem = (NSMenuItem*)parent;
  NSString *menuName = [NSString stringWithFormat:@"%s", name];

  OOMenuItem *newItem = [[OOMenuItem alloc] initWithTitle:name
                                                   action:f
                                                      arg:arg];
  NSMenu *menu = nil;
  if (parentItem) {
    menu = [parentItem submenu];
    [menu addItem:newItem];
  } else {
    menu = [[NSApplication sharedApplication] mainMenu];
    [menu addItem:newItem];
  }
  
  if (!f) {
    NSMenu *subMenu = [[NSMenu alloc] initWithTitle:menuName];
    [menu setSubmenu:subMenu forItem:newItem];
  }

  return (menu_t*)newItem;
}


void
print_foo(void *foo)
{
  printf("Hello World!\n");
}

void
menu_init(void)
{
  // TODO
  menu_t *spacecraftMenu = menu_new(NULL, "Spacecraft", NULL, NULL);
  NSLog(@"%p", spacecraftMenu);
  /*menu_t *test =*/ menu_new(spacecraftMenu, "Test Menu Item", print_foo, NULL);
  menu_t *scenarioMenu = menu_new(NULL, "Scenario", NULL, NULL);
  NSLog(@"%p", scenarioMenu);
  menu_t *viewMenu = menu_get("View");
  NSLog(@"%p", viewMenu);
  /*menu_t *test2 =*/ menu_new(viewMenu, "Test Menu Item", print_foo, NULL);
  
}

