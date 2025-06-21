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


#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "setup.h"
#include "header.h"
#include "symbol.h"


void IO_bin_fp(next_read *nx)
{

    const char *path = DEFAULT_TEST_PATH;
    DIR *dir;

    char * point = NULL;
    struct dirent *entry;


    int count = 0;
    nx -> nx_entries = malloc(MAX_PROGRAM_RUNTIME * sizeof(struct dirent *));


    if (!nx -> nx_entries)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);

    }

    dir = opendir(path);

    if (!dir)
    {
        perror("opendir failed");
        free(nx -> nx_entries);
        exit(EXIT_FAILURE);

    }

    while ((entry = readdir(dir)) != NULL && count < MAX_PROGRAM_RUNTIME)
    {

     if(strrchr(entry -> d_name, '.') != NULL)
     {

      point = strrchr(entry -> d_name, '.');

     }



     if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || strcmp(point, ".c") == 0)
      continue;


        struct dirent *copy = malloc(sizeof(struct dirent));
        if (!copy)
        {
            perror("malloc copy failed");
            break;
        }

         memcpy(copy, entry, sizeof(struct dirent));
         nx -> nx_entries[count] = copy;
         count++;





    }


    closedir(dir);
    nx -> ssz = count;

}

void IO_read_elf(next_read *nx, int counter)
{

    size_t len = strlen(nx -> nx_entries[counter - 1] -> d_name) + strlen(DEFAULT_TEST_PATH) + 1;
    char *current = malloc(len);

    if (!current)
    {
        perror("malloc failed");
        exit(EXIT_FAILURE);

    }

    strcpy(current, DEFAULT_TEST_PATH);
    strcat(current, nx -> nx_entries[counter - 1] -> d_name);



    const char * bin_fp = current;
    int fd = open(bin_fp, O_RDONLY);


    if(fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if(elf_version(EV_CURRENT) == EV_NONE)
    {
        fprintf(stderr, "ELF library initialization failed: %s\n", elf_errmsg(-1));
        close(fd);
        exit(EXIT_FAILURE);
    }

    Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
    if(!elf)
    {
        fprintf(stderr, "elf_begin() failed: %s\n", elf_errmsg(-1));
        close(fd);
        exit(EXIT_FAILURE);
    }

    nx -> fd = fd;
    nx -> elf = elf;

}




void init_elf_header(elf_w *h, Elf* elf)
{
    GElf_Ehdr *gheader = (GElf_Ehdr *)malloc(sizeof(GElf_Ehdr));


    if (!gelf_getehdr(elf, gheader) || !gheader)
    {
        fprintf(stderr, "Failed to get ELF header: %s\n", elf_errmsg(-1));
        elf_end(elf);
        exit(EXIT_FAILURE);
    }

   h -> elf_header = gheader;


}


void IO_ident_elf(elf_w *elf)
{
     int offset_magic;

     offset_magic = memcmp(ELF_MAGIC, elf -> elf_header -> e_ident , strlen(ELF_MAGIC));
     if(offset_magic != 0)
     {
       perror("This file is not an ELF file I.E does not contain magic number") ;
       exit(EXIT_FAILURE);
     }

     free(elf);

}



void init_ph_table(Elf *elf, int fd)
{


    runtime *r = malloc(sizeof(runtime));
    program *ldl = malloc(sizeof(program));



    size_t phnum = 0;




    if (elf_getphdrnum(elf, &phnum) != 0 || phnum == 0)
    {
        fprintf(stderr, "Failed to get number of program headers: %s\n", elf_errmsg(-1));
        exit(EXIT_FAILURE);
    }

    program_header_table *ph_table = malloc(phnum * sizeof(program_header_table));
    ELF64relocation_table *relocation_table = malloc (sizeof(ELF64relocation_table));
    Elf64_Relocate *relocate = malloc(sizeof(ELF64_relocate));

    if(!ldl || !r  || !ph_table || !relocation_table || !relocate)
    {
        perror("Failed to allocate memory for program struct");
        exit(EXIT_FAILURE);

    }

    ldl -> cursor = (Elf64_Dyn **)malloc(sizeof(Elf64_Dyn *));
    ldl -> fd = fd;
    ldl -> ssz_cur = 0;

    if(!ldl -> cursor)
    {
     perror("Failed to allocate memory to cursor");
     exit(EXIT_FAILURE);

    }


    r -> dynstr = NULL;

    ph_table -> capacity = phnum;
    ph_table -> size = phnum;

    relocation_table -> entries = NULL;
    relocation_table -> count = 0;

    relocate->rela_sz      = 0;
    relocate->rela_ent_sz  = 0;

    IO_program_headers(elf, ldl, ph_table);
    ELF64dynamic_section_load(ldl, r, relocate, relocation_table);
    ELF64_entry_load(r);



}

void init_relocate()
{

 ELF64relocation_table *table = malloc(sizeof(ELF64relocation_table));


 if(!table)
 {

  perror("Failed to allocate memory to cursor");
  exit(EXIT_FAILURE);

 }


 free(table);

}




void map_init()
{

    next_read *nx = malloc(sizeof(next_read));
    elf_w *h = malloc(sizeof(elf_w));

    if(!nx || !h)
    {
     perror("Failed to allocate memory to init file");
     exit(EXIT_FAILURE);
    }

    IO_bin_fp(nx);

    if((nx -> ssz)  > 0)
    {

    IO_read_elf(nx, nx -> ssz);
    nx -> ssz--;

    }

    init_elf_header(h, nx -> elf);
    IO_ident_elf(h);
    init_ph_table(nx -> elf, nx -> fd);



    elf_end(nx -> elf);
    close(nx -> fd);

    free(h);
    free(nx);

}
