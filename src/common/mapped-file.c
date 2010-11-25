/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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
