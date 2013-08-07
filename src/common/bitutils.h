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


#ifndef BITUTILS_H_HBYMFJ6H
#define BITUTILS_H_HBYMFJ6H
#include <assert.h>
#include <stdint.h>

#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
  #if defined(PPC) || defined(SPARC)
    #define BIG_ENDIAN 1
  #else
    #define LITTLE_ENDIAN 1
  #endif
#endif

#if defined(LITTLE_ENDIAN)
static inline uint16_t ooLittleToHost16(uint16_t a) {return a;}
static inline uint32_t ooLittleToHost32(uint32_t a) {return a;}
static inline uint64_t ooLittleToHost64(uint64_t a) {return a;}

static inline uint16_t
ooBigToHost16(uint16_t a)
{
  return a << 8 | a >> 8;
}

static inline uint32_t
ooBigToHost32(uint32_t a)
{
  return (ooBigToHost16(a & 0x0ffff) << 16) | ooBigToHost16(a >> 16);
}

static inline uint64_t
ooBigToHost64(uint64_t a)
{
  assert(0 && "not implemented");

  return a;
}

#elif defined(BIG_ENDIAN)

static inline uint16_t
ooLittleToHost16(uint16_t a)
{
  return a << 8 | a >> 8;
}

static inline uint32_t
ooLittleToHost32(uint32_t a)
{
  return (ooLittleToHost16(a & 0x0ffff) << 16) | ooLittleToHost16(a >> 16);
}

static inline uint64_t
ooLittleToHost64(uint64_t a)
{
  assert(0 && "not implemented");
  return a;
}

static inline uint16_t ooBigToHost16(uint16_t a) {return a;}
static inline uint32_t ooBigToHost32(uint32_t a) {return a;}
static inline uint64_t ooBigToHost64(uint64_t a) {return a;}

#else
#error "Unknown endianess of target"
#endif

// Hackers Delight p48, round up to closest power of 2
static inline uint32_t
clp2_32(uint32_t x)
{
  x = x - 1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x + 1;
}


#endif /* end of include guard: BITUTILS_H_HBYMFJ6H */
