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
#include <string.h>
#include <stddef.h>
#include <dlfcn.h>
#include <stdint.h>
#include "symbol.h"

#define NUM_FUNCTIONS 6
#define OFFSET_DUN 8


void IO_add_relocate(ELF64relocation_table *table, Elf64_Addr address, ssize_t sz, ssize_t entsz)
{


   Elf64_Relocate *new_entries = realloc(table->entries, (table->count + 1) * sizeof(Elf64_Relocate));
   if (!new_entries)

   {
    perror("Failed to realloc relocate table");
    exit(EXIT_FAILURE);

   }

   table->entries = new_entries;

   table->entries[table->count].address = address;
   table->entries[table->count].rela_sz = sz;
   table->entries[table->count].rela_ent_sz = entsz;

   table->count++;
}


void IO_dt_lib(Elf64_Dyn *dyn, runtime *r)
{


      const char *needed_lib =  r -> dynstr + dyn -> d_un.d_val;
      char **temp = realloc(r -> lib_name, (r -> ssz_lib + 1) * sizeof(char *));

      if (!temp)
      {

      perror("Failed to realloc lib_name array");
      exit(EXIT_FAILURE);

      }

     r -> lib_name = temp;
     r -> lib_name[r -> ssz_lib] = strdup(needed_lib);
     r -> ssz_lib++;

}

void IO_dt_rpth(Elf64_Dyn *dyn, runtime *r)
{

     const char *runpath =  r -> dynstr + dyn -> d_un.d_val;
     char **temp = realloc(r -> rpath, (r -> ssz_rpath + 1) * sizeof(char *));


     if (!temp)
     {
      perror("Failed to realloc rpath array");
      exit(EXIT_FAILURE);

     }

     r -> rpath = temp;
     r -> rpath[r->ssz_rpath] = strdup(runpath);
     r -> ssz_rpath++;



}

void IO_cur_update(Elf64_Dyn *dyn, program *ldl)
{
     const Elf64_Dyn * s_dyn = dyn;
     Elf64_Dyn **temp = realloc(ldl -> cursor, (ldl -> ssz_cur + 1) * sizeof(Elf64_Dyn *));

     if (!temp)
     {

      perror("Failed to realloc lib_name array");
      exit(EXIT_FAILURE);

     }

     ldl -> cursor = temp;
     ldl -> cursor [ldl -> ssz_cur] = dyn;
     ldl ->ssz_cur++;

}



void IO_concat_fpath(runtime *r)
{
    for (int i = 0; i < r->ssz_rpath; i++)
    {
        for (int j = 0; j < r->ssz_lib; j++)
        {

            char fullpath[PATH_MAX];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", r->rpath[i], r->lib_name[j]);

            if (access(fullpath, R_OK) == 0)
            {
                char **temp = realloc(r->fpath, (r->ssz_fpath + 1) * sizeof(char *));

                if (!temp)
                {
                    perror("realloc failed for fpath");
                    exit(EXIT_FAILURE);
                }

                r->fpath = temp;

                r->fpath[r->ssz_fpath] = strdup(fullpath);

                if (!r->fpath[r->ssz_fpath])
                {
                    perror("strdup failed");
                    exit(EXIT_FAILURE);
                }

                r->ssz_fpath++;
                break;
            }
        }
    }
}

Elf64_Addr IO_lookup_path(runtime * r, char *name)
{

   void *handle;

   for(int i = 0; i < r -> ssz_fpath; i++)
   {
      handle = dlopen(r -> fpath[i], RTLD_LOCAL | RTLD_LAZY);

      if(!handle)
      {
          continue;
      }

      void *fptr = dlsym(handle, name);

      if(fptr != NULL)
      {
       return (Elf64_Addr)fptr;
      }

   }

   return 0;

}



void IO_rqk_add(Elf64_Relocate *relocate, ELF64relocation_table *relocate_table)
{
    if (relocate -> address && relocate -> rela_sz && relocate -> rela_ent_sz)
    {
        IO_add_relocate(relocate_table, relocate -> address, relocate -> rela_sz, relocate -> rela_ent_sz);

    }

}



void ELF64_DT_rpath(runtime * r)
{


  r -> ssz_rpath = r -> ssz_lib;


  if(LD_LIBRARY_PATH())
  {

  for (int i = 0; i < r -> ssz_rpath; i++)
  {
    char **temp = realloc(r->rpath, (r->ssz_rpath + 1) * sizeof(char *));
    const char *runpath = NULL;

    if(!temp )
    {

     perror("Failed to allocate memory to temp");
     exit(1);
    }
    runpath = getenv(r -> lib_name[i]);

    if(!runpath)
    {
      printf("No run path found for library, %s \n", r -> lib_name[i]);
      break;

    }

    r->rpath = temp;
    r->rpath[i] = strdup(runpath);
    printf("RPATH/RUNPATH: %s\n", r -> rpath[i]);

  }
}

 else
 {

  for(int k = 0; k < r -> ssz_rpath; k++ )
  {

   for (int i = 0; paths[i]; i++)
   {

    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", paths[i], r -> lib_name[i]);

    if (access(fullpath, R_OK) == 0)
    {
        char **temp = realloc(r->fpath, (r->ssz_fpath + 1) * sizeof(char *));

        r->fpath = temp;
        r->fpath[r -> ssz_fpath] = fullpath;

        r -> ssz_fpath ++;
        break;
    }
   }

  }

 }
}



void ELF64_entry_load(runtime * r)
{

   if (r -> init_func)
   {

    ((char (*)())r->init_func)();


   }


}

void ELF64dynamic_section_load(program *ldl, runtime *r, Elf64_Relocate *relocate, ELF64relocation_table *relocate_table)
{



    Elf64_Dyn *dynamic = ldl -> dyn;

    r -> lib_name = (char **)malloc(sizeof(char *));
    r -> rpath = (char **)malloc(sizeof(char *));
    r -> fpath = (char **)malloc(sizeof(char *));
    r->ssz_rpath = 0;

    if (!r->lib_name || !r -> rpath || !r -> fpath)
    {
        perror("Failed to allocate memory for string array");
        exit(EXIT_FAILURE);
    }


    for ( Elf64_Dyn *dyn = dynamic; dyn->d_tag != DT_NULL; ++dyn)
    {
        switch (dyn->d_tag)
        {

            case DT_STRTAB:
                r -> dynstr = (char *)(dyn->d_un.d_ptr + BASE_ADDRESS);
                IO_rqk_add(relocate, relocate_table);
                break;

            case DT_INIT:
                r -> init_func = (char *) dyn -> d_un.d_ptr + BASE_ADDRESS;
                break;

            case DT_SYMTAB:
                r -> symtab = (Elf64_Sym *) (dyn->d_un.d_ptr + BASE_ADDRESS);
                break;

            case DT_RELA:
                relocate -> address =  (Elf64_Addr ) (dyn->d_un.d_ptr + BASE_ADDRESS);
                IO_rqk_add(relocate, relocate_table);
                break;

            case DT_RELASZ:
                relocate -> rela_sz = (size_t) dyn->d_un.d_val;
                IO_rqk_add(relocate, relocate_table);
                break;

            case DT_RELAENT:
                relocate -> rela_ent_sz = (size_t) dyn->d_un.d_val;
                IO_rqk_add(relocate, relocate_table);
                break;

            default:
                IO_cur_update(dyn, ldl);
                break;
        }



    }

    if(r -> dynstr)
    {

     for(size_t i = 0; i < ldl -> ssz_cur  ; i++)
      {

        switch(ldl -> cursor[i] -> d_tag)
        {

         case DT_RPATH:
          IO_dt_rpth(ldl -> cursor[i], r);
          break;

         case DT_NEEDED:
          IO_dt_lib(ldl -> cursor[i], r);
          break;


        }
      }
    }

    else
    {
      perror("No dynstr found");
      exit(EXIT_FAILURE);
    }

    IO_concat_fpath(r);
    ldl -> diff_lib = (r -> ssz_lib - r -> ssz_rpath);

    if(ldl -> diff_lib != 0 || !r -> rpath)
    {
        ELF64_DT_rpath(r);

    }

    ELF64_relocate(relocate_table, r);

}

void ELF64_relocate(ELF64relocation_table *table, runtime *r)
{

    for (size_t i = 0; i < table->count; i++)
    {
        size_t rela_sz = table -> entries[i].rela_sz;
        size_t rela_ent_sz = table -> entries[i].rela_ent_sz;

        if (rela_sz == 0 || rela_ent_sz == 0 || !table -> entries[i].address)
        {
            fprintf(stderr, "Relocation entry %zu is invalid (empty size or NULL address)\n", i);
            exit(EXIT_FAILURE);
        }

        size_t count = rela_sz / rela_ent_sz;
        Elf64_Rela *relocs = (Elf64_Rela *)table -> entries[i].address;

        for (size_t j = 0; j < count; j++)
        {

            Elf64_Addr offset = (Elf64_Addr) relocs[j].r_offset;
            Elf64_Addr addend = relocs[j].r_addend;

            uint32_t type = ELF64_R_TYPE(relocs[j].r_info);
            Elf64_Addr *patch_addr = (Elf64_Addr *)(offset + BASE_ADDRESS);

            switch (type)
            {
                case R_X86_64_RELATIVE:
                {
                    *patch_addr = BASE_ADDRESS + addend;
                    break;

                }

               case R_X86_64_GLOB_DAT:
               {

                uint32_t sym_index = ELF64_R_SYM(relocs[j].r_info);
                Elf64_Sym *sym = &r->symtab[sym_index];

                if (!r->symtab)
               {
                   fprintf(stderr, "symtab is NULL \n");
                   exit(EXIT_FAILURE);
               }


               if (ELF64_ST_BIND(sym->st_info) != STB_GLOBAL && ELF64_ST_TYPE(sym->st_info) != STT_FUNC && sym->st_name != 0)
               {

                    const char *symbol_name = &r->dynstr[sym->st_name];
                    void *resolved = dlsym(RTLD_DEFAULT, symbol_name);


                    if (!resolved)
                    {

                      if (strcmp(symbol_name, "_ITM_deregisterTMCloneTable") == 0 ||  strcmp(symbol_name, "_ITM_registerTMCloneTable") == 0 || strcmp(symbol_name, "__gmon_start__") == 0)
                      {
                         break;
                       }

                        printf("ERROR %s at %p \n", symbol_name, resolved);
                        exit(EXIT_FAILURE);
                    }


                    *patch_addr = (Elf64_Addr)resolved;

               }

                break;

               }

                default:
                    break;
            }
        }
    }
}
