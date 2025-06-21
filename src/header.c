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
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <elf.h>
#include "header.h"
#include "symbol.h"


void IO_basic_map(Elf64_Phdr *ph, void *program_struct, int fd, uintptr_t flags)
{
    program *ldl = (program *)program_struct;

    if (ph->p_filesz > ph->p_memsz)
    {
        fprintf(stderr, "Invalid segment: filesz > memsz \n");
        exit(1);
    }

    int prot = 0;
    if (ph->p_flags & PF_R) prot |= PROT_READ;
    if (ph->p_flags & PF_W) prot |= PROT_WRITE;
    if (ph->p_flags & PF_X) prot |= PROT_EXEC;
    if (prot & PROT_EXEC) prot |= PROT_READ;

    uintptr_t vaddr         = ph -> p_vaddr + BASE_ADDRESS;
    uintptr_t aligned_addr  = PAGE_ALIGN_DOWN(vaddr);
    off_t aligned_offset = PAGE_ALIGN_DOWN(ph -> p_offset);
    size_t offset_in_page = vaddr - aligned_addr;
    size_t mem_size       = PAGE_ALIGN_UP(ph -> p_memsz + offset_in_page);

    lseek(fd, 0, SEEK_SET);


    void *mapping = mmap((void *)aligned_addr, mem_size, prot, MAP_PRIVATE | MAP_FIXED_NOREPLACE, fd, aligned_offset);

    if (mapping == MAP_FAILED)
    {
        if (errno == EEXIST || errno == EPERM || errno == EINVAL)
        {

            mapping = mmap((void *)aligned_addr, mem_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);

            if (mapping == MAP_FAILED)
            {
                exit(1);
            }

            void *segment_dest = (char *)mapping + offset_in_page;

            if (lseek(fd, ph->p_offset, SEEK_SET) == (off_t) - 1)
            {
                perror("lseek failed");
                exit(1);
            }

            if (read(fd, segment_dest, ph -> p_filesz) != (ssize_t)ph -> p_filesz)
            {
                perror("read failed");
                exit(1);
            }

            if (ph -> p_memsz > ph -> p_filesz)
            {
                memset((char *)segment_dest + ph -> p_filesz, 0, ph -> p_memsz - ph -> p_filesz);
            }
        }
        else
        {
            perror("mmap failed");
            exit(1);
        }
    }

    if (flags == SIG_P_STACK)
    {
        ldl->dyn = (Elf64_Dyn *) (ph -> p_vaddr + BASE_ADDRESS);
    }

}


void ELF64_mapping(program_header_table *ph_table, program *ldl, int fd)
{

 for (size_t i = 0; i < ph_table -> capacity ; i++)
 {

   Elf64_Phdr *ph = ph_table[i].entry;
   switch (ph->p_type)
    {

     case PT_LOAD:
      IO_basic_map(ph, ldl, fd, SIG_IGNORE);
      break;


     case PT_DYNAMIC:
      IO_basic_map(ph, ldl, fd, SIG_P_STACK);
      break;


     case PT_NULL:
      break;

        }
    }
}


void IO_program_headers(Elf *elf, program *ldl, program_header_table *ph_table)
{

    Elf64_Phdr *ph_array = malloc(ph_table -> capacity * sizeof(Elf64_Phdr));

    for (size_t i = 0; i < ph_table -> capacity ; i++)
    {
        GElf_Phdr gph;
        if (gelf_getphdr(elf, i, &gph) == NULL)
        {
            fprintf(stderr, "Failed to get program header %zu: %s\n", i, elf_errmsg(-1));
            continue;
        }

        ph_array[i] = gph;
        ph_table[i].entry = &ph_array[i];

    }


    ELF64_mapping(ph_table, ldl, ldl -> fd);
    free(ph_array);
}


