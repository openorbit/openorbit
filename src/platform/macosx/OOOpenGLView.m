/*
 Copyright 2011,2012 Mattias Holm <mattias.holm(at)openorbit.org>

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

#import "OOOpenGLView.h"

#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>

#import "rendering/render.h"
#import "rendering/scenegraph.h"
#import "sim.h"
#import "io-manager.h"
#import "settings.h"

int init_sim(int argc, const char* argv[argc]);

// Virtual key codes from inside macintosh, figure C-2 (figure does not
// include codes for right modifier keys)
static io_keycode_t keymap [256] = {
  [56] = IO_LSHIFT,
  [60] = IO_RSHIFT,
  [55] = IO_LMETA,
  [54] = IO_RMETA,
  [59] = IO_LCTRL,
  [62] = IO_RCTRL,
  [58] = IO_LALT,
  [61] = IO_RALT,

  //[SDL_SCANCODE_MODE]   = IO_MODE,
  [114]   = IO_HELP,
  //[SDL_SCANCODE_SYSREQ] = IO_SYSREQ,
  //[SDL_SCANCODE_CLEAR]  = IO_CLEAR,
  //[SDL_SCANCODE_MENU]   = IO_MENU,
  //[SDL_SCANCODE_POWER]  = IO_POWER,

  [36] = IO_RETURN,
  [49]  = IO_SPACE,
  [50]    = IO_TAB,
  [51] = IO_BACKSPACE,
  [53] = IO_ESCAPE,
  [47] = IO_PERIOD,
  [43]  = IO_COMMA,
  //[SDL_SCANCODE_PAUSE]  = IO_PAUSE,
  [27]  = IO_MINUS,
  [44]  = IO_SLASH,

  [41] = IO_SEMICOLON,
  [24] = IO_EQUALS,
  [33] = IO_LEFTBRACKET,
  [42] = IO_BACKSLASH,
  [30] = IO_RIGHTBRACKET,

  [29] = IO_0,
  [18] = IO_1,
  [19] = IO_2,
  [20] = IO_3,
  [21] = IO_4,
  [23] = IO_5,
  [22] = IO_6,
  [26] = IO_7,
  [28] = IO_8,
  [25] = IO_9,

  [82] = IO_KP_0,
  [83] = IO_KP_1,
  [84] = IO_KP_2,
  [85] = IO_KP_3,
  [86] = IO_KP_4,
  [87] = IO_KP_5,
  [88] = IO_KP_6,
  [89] = IO_KP_7,
  [91] = IO_KP_8,
  [92] = IO_KP_9,

  [65] = IO_KP_PERIOD,
  [75] = IO_KP_DIV,
  [67] = IO_KP_MUL,
  [78] = IO_KP_MIN,
  [69] = IO_KP_PLUS,
  [76] = IO_KP_ENTER,
  [81] = IO_KP_EQ,
  [71] = IO_KP_CLR,

  [117]= IO_DEL,


  [0] = IO_A,
  [11] = IO_B,
  [8] = IO_C,
  [2] = IO_D,
  [14] = IO_E,
  [3] = IO_F,
  [5] = IO_G,
  [4] = IO_H,
  [34] = IO_I,
  [38] = IO_J,
  [40] = IO_K,
  [37] = IO_L,
  [46] = IO_M,
  [45] = IO_N,
  [31] = IO_O,
  [35] = IO_P,
  [12] = IO_Q,
  [15] = IO_R,
  [1] = IO_S,
  [17] = IO_T,
  [32] = IO_U,
  [9] = IO_V,
  [13] = IO_W,
  [7] = IO_X,
  [16] = IO_Y,
  [6] = IO_Z,

  [122]  = IO_F1,
  [120]  = IO_F2,
  [99]   = IO_F3,
  [118]  = IO_F4,
  [96]   = IO_F5,
  [97]   = IO_F6,
  [98]   = IO_F7,
  [100]  = IO_F8,
  [101]  = IO_F9,
  [109]  = IO_F10,
  [103]  = IO_F11,
  [111]  = IO_F12,
  [105]  = IO_F13,
  [107]  = IO_F14,
  [113]  = IO_F15,

  [126]  = IO_UP,
  [125]  = IO_DOWN,
  [123]  = IO_LEFT,
  [124]  = IO_RIGHT,

  //[SDL_SCANCODE_INSERT]   = IO_INSERT,
  //[SDL_SCANCODE_HOME]     = IO_HOME,
  [119]      = IO_END,
  [116]   = IO_PAGEUP,
  [121] = IO_PAGEDOWN,
};


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
  NSOpenGLPixelFormatAttribute attributes [] = {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    NSOpenGLPFADoubleBuffer, 0};

  NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
  self.openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];

}

- (void)simTicker:(NSTimer*)theTimer
{
  //NSLog(@"sim step");
  ooSimStep(theTimer.timeInterval);
  [self setNeedsDisplay:YES];
}

- (BOOL)acceptsFirstResponder
{
  return YES;
}

- (void) prepareOpenGL
{
  init_sim(0, NULL);

  extern SIMstate gSIM_state;
  float freq;
  ooConfGetFloatDef("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  float wc_period = 1.0 / freq; // Period in s
  float sim_period;
  ooConfGetFloatDef("openorbit/sim/period", &sim_period, wc_period);

  timer = [NSTimer scheduledTimerWithTimeInterval:sim_period target:self selector:@selector(simTicker:) userInfo:nil repeats:YES];
}

- (void)clearGLContext
{
  [super clearGLContext];
}

- (void)keyDown:(NSEvent *)theEvent
{
  unsigned short key_code = theEvent.keyCode;

  if (key_code < 256) {
    io_keycode_t code = keymap[key_code];
    if (code != 0) {
      uint16_t mask = 0;
      if (theEvent.modifierFlags & NSCommandKeyMask) mask |= OO_IO_MOD_LMETA;
      if (theEvent.modifierFlags & NSControlKeyMask) mask |= OO_IO_MOD_LCTRL;
      if (theEvent.modifierFlags & NSAlternateKeyMask) mask |= OO_IO_MOD_LALT;
      if (theEvent.modifierFlags & NSShiftKeyMask) mask |= OO_IO_MOD_LSHIFT;
      ioDispatchKeyDown(code, mask);
    }
  } else {
    NSLog(@"unknown key pressed, keycode = %d", (int)key_code);
  }
}

- (void)keyUp:(NSEvent *)theEvent
{
  unsigned short key_code = theEvent.keyCode;

  if (key_code < 256) {
    io_keycode_t code = keymap[key_code];
    if (code != 0) {
      uint16_t mask = 0;
      if (theEvent.modifierFlags & NSCommandKeyMask) mask |= OO_IO_MOD_LMETA;
      if (theEvent.modifierFlags & NSControlKeyMask) mask |= OO_IO_MOD_LCTRL;
      if (theEvent.modifierFlags & NSAlternateKeyMask) mask |= OO_IO_MOD_LALT;
      if (theEvent.modifierFlags & NSShiftKeyMask) mask |= OO_IO_MOD_LSHIFT;
      ioDispatchKeyUp(code, mask);
    }
  } else {
    NSLog(@"unknown key pressed, keycode = %d", (int)key_code);
  }
}


- (void) reshape
{
  NSRect r = self.bounds;
  ooResizeScreen(r.origin.x, r.origin.y, r.size.width, r.size.height, false);
  [self setNeedsDisplay:YES];
}

- (void) drawRect:(NSRect)dirtyRect
{
  extern SIMstate gSIM_state;
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  sgPaint(gSIM_state.sg);

  [self.openGLContext flushBuffer];
}

@end
