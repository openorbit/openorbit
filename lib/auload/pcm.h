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

#ifndef orbit_sound_h
#define orbit_sound_h

#include <stddef.h>

typedef enum {
  PCM_MONO8,
  PCM_MONO16,
  PCM_STEREO8,
  PCM_STEREO16,
} pcm_format_t;

typedef struct {
  pcm_format_t format;
  unsigned samplesPerSec;
  size_t bytesLen;
  char *data;
} pcm_audio_t;

pcm_audio_t* pcm_load(const char *path);

#endif
