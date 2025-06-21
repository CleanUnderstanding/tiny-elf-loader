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


#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <libelf.h>
#include <gelf.h>

#define ELF_MAGIC "\x7f\x45\x4c\x46"
#define ELF_CLAS_32 0x01
#define ELF_CLASS_64 0x02
#define BASE_ADDRESS 0x70000000

#define PAGE_SIZE 4096
#define PAGE_ALIGN_DOWN(x) ((x) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(x)   (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

#define SIG_P_STACK 1
#define SIG_IGNORE 0


#define LD_LIBRARY_PATH() getenv("LD_LIBRARY_PATH")
#define PATH_MAX 4096

static const char *paths[] = {
    "/lib",
    "/lib64",
    "/usr/lib",
    "/usr/lib64",
    "/lib/x86_64-linux-gnu",
    "/usr/lib/x86_64-linux-gnu",
    "/lib32",
    "/usr/lib32",
    "/app/lib",
    "/var/lib/flatpak/runtime",
};



typedef struct
{

    GElf_Ehdr *elf_header;

} elf_w;



typedef struct
{

    Elf64_Dyn *dyn;
    char *bytes_channel_zero;
    char *bytes_channel_one;
    Elf64_Dyn **cursor;
    ssize_t bytes_channel_zero_read;
    ssize_t bytes_channel_one_read;
    size_t ssz_cur;
    ssize_t ssz_dynstr;
    size_t diff_lib;
    int fd;

} program;

typedef struct
{

  char **lib_name;
  char **rpath;
  char **fpath;

  char *dynstr;
  Elf64_Sym *symtab;

  ssize_t ssz_lib;
  ssize_t ssz_rpath;
  ssize_t ssz_fpath;
  char *init_func;

} runtime;


typedef struct
{
    Elf64_Phdr *entry;
    size_t size;
    size_t capacity;
} program_header_table;



void ELF64_mapping(program_header_table *ph_table, program *ldl, int fd);
void ELF64_entry_load(runtime * r);

void IO_basic_map(Elf64_Phdr *ph, void *program_struct, int fd, uintptr_t flags);
void IO_program_headers(Elf *elf, program *ldl, program_header_table *ph_table);

#endif
