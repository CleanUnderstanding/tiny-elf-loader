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


#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include "header.h"



typedef struct
{
  Elf64_Addr address;
  size_t rela_sz;
  size_t rela_ent_sz;

} Elf64_Relocate;

typedef struct
{

  Elf64_Relocate *entries;
  size_t count;

} ELF64relocation_table;





void ELF64dynamic_section_load(program *ldl, runtime *r, Elf64_Relocate *relocate, ELF64relocation_table *relocate_table);
void ELF64_DT_rpath(runtime * r);
void ELF64_relocate(ELF64relocation_table *table, runtime *r);
void ELF64_entry_load(runtime * r);


void IO_add_relocate(ELF64relocation_table *table, Elf64_Addr address, ssize_t sz, ssize_t entsz);
void IO_dt_lib(Elf64_Dyn *dyn, runtime *r);
void IO_dt_rpth(Elf64_Dyn *dyn, runtime *r);
void IO_cur_update(Elf64_Dyn *dyn, program *ldl);
void IO_concat_fpath(runtime *r);
void IO_rqk_add(Elf64_Relocate *relocate, ELF64relocation_table * relocate_table);
Elf64_Addr IO_lookup_path(runtime * r, char *name);




#endif
