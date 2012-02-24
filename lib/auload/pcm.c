/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>
 
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
