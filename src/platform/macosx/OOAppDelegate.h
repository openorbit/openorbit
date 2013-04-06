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


#import <OpenGL/gl3.h>
#import <Cocoa/Cocoa.h>
#import "OOOpenGLView.h"

@interface OOAppDelegate : NSObject <NSApplicationDelegate> {
  OOOpenGLView *openGLView;
  NSWindow *mainWindow;
}
@property (assign) IBOutlet NSWindow *mainWindow;

@property (assign) IBOutlet OOOpenGLView *openGLView;

@end
