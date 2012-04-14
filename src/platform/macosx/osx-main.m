/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#import <Cocoa/Cocoa.h>

#import "common/moduleinit.h"
void hidInit(void);

//#import <SDL/SDL.h> // Needed for joystick access
int
main(int argc, const char *argv[argc])
{
  module_initialize();
  //if (SDL_Init(SDL_INIT_JOYSTICK) != 0) {
  //  NSLog(@"Could not initialize SDL, bailing out!");
  //  exit(0);
  //} else {
  //  atexit(SDL_Quit);
  //}
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  hidInit();
  [pool release];
  return NSApplicationMain(argc, argv);
}
