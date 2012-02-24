/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>
 
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



#include "mapped-file.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
static off_t
get_flen(const char *path)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    return 0;
  }

  off_t len = lseek(fd, 0, SEEK_END);
  close(fd);

  return len;
}

mapped_file_t
map_file(const char *path)
{
  mapped_file_t mf = {0, -1, NULL};
  off_t len = get_flen(path);
  if (len == 0) {
    // No data or no file with that name
    return mf;
  }

  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror(NULL);
    assert(0 && "file not opened");
  }

  mf.fd = fd;
  mf.data = mmap(NULL, (size_t)len, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
  mf.fileLenght = len;

  return mf;
}

void
unmap_file(mapped_file_t *mf)
{
  munmap(mf->data, mf->fileLenght);
  close(mf->fd);

  mf->fd = -1;
  mf->data = NULL;
  mf->fileLenght = 0;
}
