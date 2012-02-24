/*
 Copyright 2011,2012 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#include "wav.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// BUG: Only works on little endian machines... fix this

wav_file_t*
wav_open(const char *fileName)
{
  FILE *fd = fopen(fileName, "rb");
  if (!fd) {
    return NULL;
  }

  wav_header_t header;

  fread(&header, sizeof(wav_header_t), 1, fd);
  if (memcmp(header.ckID, WAVE_CHUNK_FILE, 4)) {
    printf("file header does not match\n");
    return NULL;
  }
  if (memcmp(header.WAVEID, WAVE_WAVE_ID, 4)) {
    printf("file header does not match\n");
    return NULL;
  }

  wav_file_t *wav = malloc(sizeof(wav_file_t));
  wav->sampleByteSize = 0;
  wav->samples = NULL;

  wav_chunk_t chunk;
  fread(&chunk, sizeof(wav_chunk_t), 1, fd);
  while (!feof(fd) && !ferror(fd)) {
    if (!memcmp(chunk.ckID, WAVE_CHUNK_FORMAT, 4)) {
      printf("format chunk\n");
      if (!(chunk.cksize == 16 || chunk.cksize == 18 || chunk.cksize == 40)) {
        printf("format chunk of invalid size\n");
        goto error;
      }
      wav_format_chunk_t format;
      fread(&format.wFormatTag, chunk.cksize, 1, fd);
      if (feof(fd) || ferror(fd)) {
        printf("premature eof\n");
        goto error;
      }

      wav->format = format.wFormatTag;
      wav->nSamplesPerSec = format.nSamplesPerSec;
      wav->bitsPerSample = format.wBitsPerSample;
      wav->nChannels = format.nChannels;

      switch (format.wFormatTag) {
      case WAVE_FORMAT_EXTENSIBLE:
      case WAVE_FORMAT_PCM:
      case WAVE_FORMAT_IEEE_FLOAT:
      case WAVE_FORMAT_ALAW:
      case WAVE_FORMAT_MULAW:
        break;
      default:
        printf("unknown format\n");
        goto error;
      }
    } else if (!memcmp(chunk.ckID, WAVE_CHUNK_FACT, 4)) {
      printf("fact chunk\n");
      fseek(fd, (long)chunk.cksize, SEEK_CUR);
    } else if (!memcmp(chunk.ckID, WAVE_CHUNK_DATA, 4)) {
      printf("data chunk\n");
      if (wav->samples) {
        printf("samples already read");
        goto error;
      }
 
      wav->samples = malloc(chunk.cksize);
      wav->sampleByteSize = chunk.cksize;
      if (wav->samples == NULL) goto error;
      
      fread(wav->samples, chunk.cksize, 1, fd);
      if (chunk.cksize & 1) fseek(fd, 1, SEEK_CUR); // chunks are even number of bytes
      if (feof(fd) || ferror(fd)) {
        printf("premature eof\n");
        goto error;
      }
    } else {
      printf("other chunk\n");
      fseek(fd, (long)chunk.cksize, SEEK_CUR);
    }
    fread(&chunk, sizeof(wav_chunk_t), 1, fd);
  }

  fclose(fd);
  return wav;

error:
  fclose(fd);
  if (wav) free(wav->samples);
  free(wav);
  return NULL;
}

void
wav_dispose(wav_file_t *wav)
{
  free(wav->samples);
  memset(wav, 0, sizeof(wav_file_t)); // Crash early if data is used
  free(wav);
}

void
wav_info(wav_file_t *wav)
{
  switch (wav->format) {
    case WAVE_FORMAT_EXTENSIBLE:
      printf("wavinfo: extensible\n");
      break;
    case WAVE_FORMAT_PCM:
      printf("wavinfo: pcm\n");
      break;
    case WAVE_FORMAT_IEEE_FLOAT:
      printf("wavinfo: ieee float\n");
      break;
    case WAVE_FORMAT_ALAW:
      printf("wavinfo: alaw\n");
      break;
    case WAVE_FORMAT_MULAW:
      printf("wavinfo: mulaw\n");
      break;
    default:
      printf("wavinfo: unknown format\n");
  }

  printf("wavinfo: %u Hz\n", (unsigned)wav->nSamplesPerSec);
  printf("wavinfo: %u bits/sample\n", (unsigned)wav->bitsPerSample);
  printf("wavinfo: %u channels\n", (unsigned)wav->nChannels);
  printf("wavinfo: %u bytes of samples\n", (unsigned)wav->sampleByteSize);
}

ALuint
wav_getbuffer(wav_file_t *wav)
{
  ALuint buff;
  alGenBuffers(1, &buff);
  ALenum err = alGetError();
  
  if (err != AL_NO_ERROR) {
    return -1;
  }

  ALenum format;
  if (wav->nChannels == 1) {
    if (wav->bitsPerSample == 8) {
      format = AL_FORMAT_MONO8;
    } else if (wav->bitsPerSample == 16) {
      format = AL_FORMAT_MONO16;
    } else {
      printf("only supports 8 or 16 bits per sample, %d bits per sample found", (unsigned)wav->bitsPerSample);
      exit(1);
    }
  } else if (wav->nChannels == 2) {
    if (wav->bitsPerSample == 8) {
      format = AL_FORMAT_STEREO8;
    } else if (wav->bitsPerSample == 16) {
      format = AL_FORMAT_STEREO16;
    } else {
      printf("only supports 8 or 16 bits per sample, %d bits per sample found", (unsigned)wav->bitsPerSample);
      exit(1);
    }
  
  } else {
    printf("only mono and sterio supported, the file has %u channels", (unsigned)wav->nChannels);
    exit(1);
  }

  alBufferData(buff, AL_FORMAT_STEREO16, wav->samples, (unsigned)wav->sampleByteSize, wav->nSamplesPerSec);
  return buff;
}
