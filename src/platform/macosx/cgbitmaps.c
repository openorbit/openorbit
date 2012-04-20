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

#include <OpenGL/gl.h>
#include <ApplicationServices/ApplicationServices.h>
#include <stdlib.h>
#include "res-manager.h"
#include "rendering/scenegraph.h"
#include <openorbit/log.h>

typedef struct text_bitmap_t {
  CGContextRef ctxt;
  char *data;
  GLuint tex;
} text_bitmap_t;

void dump_bytes(size_t len, char data[len]);

text_bitmap_t*
text_bitmap_create_label(const char *fontName, float fontsize, const char *text)
{
  CFStringRef fontNameStr = CFStringCreateWithCString(kCFAllocatorDefault, fontName, kCFStringEncodingUTF8);
  CFStringRef string = CFStringCreateWithCString(kCFAllocatorDefault, text, kCFStringEncodingUTF8);

  CTFontRef font = CTFontCreateWithName(fontNameStr, fontsize, NULL);
  CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);

  CGContextRef context =
    CGBitmapContextCreate(NULL, strlen(text)*fontsize,
                          fontsize,
                          8, // Bits per comp
                          strlen(text)*fontsize, // Bytes per row
                          cs, // Color space
                          kCGImageAlphaNone); // No alpha

  // Initialize string, font, and context
  CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName};
  CFTypeRef values[] = { font, CGColorGetConstantColor(kCGColorWhite)};

  CFDictionaryRef attributes =
  CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
                     (const void**)&values, sizeof(keys) / sizeof(keys[0]),
                     &kCFTypeDictionaryKeyCallBacks,
                     &kCFTypeDictionaryValueCallBacks);

  CFAttributedStringRef attrString =
  CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);
  CFRelease(string);
  CFRelease(fontNameStr);
  CFRelease(attributes);


  CTLineRef line = CTLineCreateWithAttributedString(attrString);
  CGRect rect = CTLineGetImageBounds(line, context);
  CFRelease(context);

  //ooLogInfo("rect is: %f %f, %f %f", rect.origin.x, rect.origin.y,
  //          rect.size.width, rect.size.height);

  size_t w = ceil(rect.size.width);
  size_t h = ceil(rect.size.height);
  context =
    CGBitmapContextCreate(NULL, w, h,
                          8, // Bits per comp
                          w, // Bytes per row
                          cs, // Color space
                          kCGImageAlphaNone); // No alpha

  // Set text position and draw the line into the graphics context
  CGContextSetTextPosition(context, 0.0, 0.0);
  CTLineDraw(line, context);
  CGContextFlush(context);

  CFRelease(line);
  CFRelease(cs);

  text_bitmap_t *bitmap = malloc(sizeof(text_bitmap_t));
  bitmap->ctxt = context;
  bitmap->data = CGBitmapContextGetData(context);

  glGenTextures(1, &bitmap->tex);
  SG_CHECK_ERROR;

  glBindTexture(GL_TEXTURE_2D, bitmap->tex);
  SG_CHECK_ERROR;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  SG_CHECK_ERROR;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY,
               CGBitmapContextGetWidth(bitmap->ctxt),
               CGBitmapContextGetHeight(bitmap->ctxt),
               0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap->data);

  //dump_bytes(w*h, bitmap->data);

  return bitmap;
}

text_bitmap_t*
text_bitmap_create(const char *font, float fontsize, unsigned w, unsigned h)
{
#if 0
  char *path = ooResGetPath(font);
  CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                         (uint8_t*)path,
                                                         strlen(path),
                                                         false);
  CFArrayRef fontArr = CTFontManagerCreateFontDescriptorsFromURL(url);
  CFRelease(url);
  CFRelease(fontArr);
  free(path);
#endif
  CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
  text_bitmap_t *bitmap = malloc(sizeof(text_bitmap_t));
  bitmap->data = calloc(w*h, sizeof(char));
  memset(bitmap->data, ~0, w * h * sizeof(char));

  bitmap->ctxt = CGBitmapContextCreate(bitmap->data, w, h,
                                       8, // Bits per comp
                                       w, // Bytes per row
                                       cs, // Color space
                                       kCGImageAlphaNone); // No alpha


  // TODO: Use font in fontArr
  CGContextSelectFont(bitmap->ctxt, "Helvetica", fontsize,
                      kCGEncodingMacRoman);
  CGColorSpaceRelease(cs);

  glGenTextures(1, &bitmap->tex);
  glBindTexture(GL_TEXTURE_2D, bitmap->tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  SG_CHECK_ERROR;
  return bitmap;
}

void
text_bitmap_release(text_bitmap_t *bm)
{
  CGContextRelease(bm->ctxt);
  free(bm->data);
  free(bm);
}

void
dump_bytes(size_t len, char data[len])
{
#define LINE_WIDTH 20
  for (int i = 0 ; i < len/LINE_WIDTH ; i ++) {
    for (int j = 0 ; j < LINE_WIDTH ; j++) {
      if (j%4 == 0) printf(" ");
      printf("%02x", (unsigned char)data[i*LINE_WIDTH+j]);
    }
    printf("\n");
  }

  for (int i = 0 ; i < len % LINE_WIDTH ; i ++) {
    if (i%4 == 0) printf(" ");
    printf("%02x", (unsigned char)data[(len/LINE_WIDTH)*LINE_WIDTH+i]); // Note integer arith
  }
  printf("\n\n");
}

void
text_bitmap_drawtext(text_bitmap_t *bitmap, const char *text)
{
  //dump_bytes(CGBitmapContextGetWidth(bitmap->ctxt)*CGBitmapContextGetHeight(bitmap->ctxt), bitmap->data);
  CGContextShowTextAtPoint(bitmap->ctxt,
                           0.0, CGBitmapContextGetHeight(bitmap->ctxt)-14,
                           text, strlen(text));
  CGContextFlush(bitmap->ctxt);
  glBindTexture(GL_TEXTURE_2D, bitmap->tex);
  SG_CHECK_ERROR;
  glTexImage2D(GL_TEXTURE_2D, 0, 1,
               CGBitmapContextGetWidth(bitmap->ctxt),
               CGBitmapContextGetHeight(bitmap->ctxt),
               0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap->data);
  SG_CHECK_ERROR;
  //dump_bytes(CGBitmapContextGetWidth(bitmap->ctxt)*CGBitmapContextGetHeight(bitmap->ctxt), bitmap->data);

}

void
text_bitmap_dispose(text_bitmap_t *bitmap, bool disposeGL)
{
  if (disposeGL) {
    glDeleteTextures(1, &bitmap->tex);
  }

  text_bitmap_release(bitmap);
}

unsigned
text_bitmap_width(text_bitmap_t *bitmap)
{
  return CGBitmapContextGetWidth(bitmap->ctxt);
}

unsigned
text_bitmap_height(text_bitmap_t *bitmap)
{
  return CGBitmapContextGetHeight(bitmap->ctxt);
}

char*
text_bitmap_data(text_bitmap_t *bitmap)
{
  return bitmap->data;
}

GLuint
text_bitmap_texid(text_bitmap_t *bitmap)
{
  return bitmap->tex;
}
