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

#ifndef openaltest_wav_h
#define openaltest_wav_h

#include <OpenAL/al.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
  char ckID[4];
  uint32_t cksize;
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;

  uint16_t cbSize;
  uint16_t wValidBitsPerSample;
  uint32_t dwChannelMask;
  char SubFormat[16];
} wav_format_chunk_t;

typedef enum {
  WAVE_FORMAT_PCM = 0x0001,
  WAVE_FORMAT_IEEE_FLOAT = 0x0003,
  WAVE_FORMAT_ALAW = 0x0006,
  WAVE_FORMAT_MULAW = 0x0007,
  WAVE_FORMAT_EXTENSIBLE = 0xfffe,
} wav_format_t;

#define WAVE_CHUNK_FILE "RIFF"
#define WAVE_WAVE_ID "WAVE"
#define WAVE_CHUNK_FORMAT "fmt "
#define WAVE_CHUNK_FACT "fact"
#define WAVE_CHUNK_DATA "data"


typedef struct {
  char ckID[4];
  uint32_t cksize;
  uint32_t dwSampleLength;
} wav_fact_chunk_t;

typedef struct {
  char ckID[4];
  uint32_t cksize;
  char samples[];
} wav_data_chunk_t;

typedef struct {
  char ckID[4];
  uint32_t cksize;  
} wav_chunk_t;

typedef struct {
  char ckID[4];
  uint32_t cksize;
  char WAVEID[4];
} wav_header_t;

typedef struct { 
  wav_format_t format;
  uint32_t nSamplesPerSec;
  uint16_t bitsPerSample;
  uint16_t nChannels;

  size_t sampleByteSize;
  char *samples;
} wav_file_t;

wav_file_t* wav_open(const char *fileName);
void wav_dispose(wav_file_t *wav);
void wav_info(wav_file_t *wav);


ALuint wav_getbuffer(wav_file_t *wav);

#endif
