/*

Tiny Loader loader for 64 bit ELF files
Copyright (C) 2025  CleanUnderstanding

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see
<https://www.gnu.org/licenses/>.

*/


#ifndef SETUP_H
#define SETUP_H

#include <stdio.h>
#include <dirent.h>
#include "header.h"

#define MAX_PROGRAM_RUNTIME 5
#define DEFAULT_TEST_PATH "../test/"


typedef struct
{

 struct dirent **nx_entries;
 Elf *elf;
 int fd;
 int ssz;

} next_read;

void IO_bin_fp(next_read *nx);
void IO_read_elf(next_read *nx, int counter);
void IO_ident_elf(elf_w *elf);
void map_init();
#endif
