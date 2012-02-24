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

#include "pcm.h"
#include "wav.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

pcm_audio_t*
pcm_load(const char *path)
{
  pcm_audio_t *pcm = NULL;
  wav_file_t *wav = NULL;
  const char *ext = strrchr(path, '.');
  if (!strcmp(ext, ".wav")) {
    wav = wav_open(path);
    if (wav) {
      pcm = malloc(sizeof(pcm_audio_t));
      
      if (pcm) {
        pcm->data = NULL;
        if (wav->format == WAVE_FORMAT_PCM) {
          if (wav->nChannels == 1 && wav->sampleByteSize == 8) {
            pcm->format = PCM_MONO8;
          } else if (wav->nChannels == 1 && wav->bitsPerSample == 16) {
            pcm->format = PCM_MONO16;
          } else if (wav->nChannels == 2 && wav->bitsPerSample == 8) {
            pcm->format = PCM_STEREO8;
          } else if (wav->nChannels == 2 && wav->bitsPerSample == 16) {
            pcm->format = PCM_STEREO16;
          } else {
            fprintf(stderr, "unsupported wav-format '%s'\n", path);
            goto error;
          }
          
          pcm->samplesPerSec = wav->nSamplesPerSec;
          pcm->data = malloc(wav->sampleByteSize);
          if (pcm->data) {
            pcm->bytesLen = wav->sampleByteSize;
            memcpy(pcm->data, wav->samples, wav->sampleByteSize);
            wav_dispose(wav);
            return pcm;
          }
        } else {
          fprintf(stderr, "unsupported wav-format '%s'\n", path);
          goto error;
        }
      }
    }
  }

error:
  wav_dispose(wav);
  if (pcm) free(pcm->data);
  free(pcm);
  return NULL;
}
