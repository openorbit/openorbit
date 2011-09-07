/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#import "OOOpenGLView.h"
#import <OpenGL/OpenGL.h>
#import "rendering/render.h"

int init_sim(int argc, const char* argv[argc]);


@implementation OOOpenGLView

- (id)init
{
    self = [super init];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void) awakeFromNib
{
  init_sim(0, NULL);
}

- (void)clearGLContext
{
  [super clearGLContext];
}

- (void)keyDown:(NSEvent *)theEvent
{

}

- (void) reshape
{
  NSRect r = self.bounds;
  ooResizeScreen(r.origin.x, r.origin.y, r.size.width, r.size.height, false);
  [self setNeedsDisplay:YES];
}

- (void) drawRect:(NSRect)dirtyRect
{

  [self.openGLContext flushBuffer];
}

@end
