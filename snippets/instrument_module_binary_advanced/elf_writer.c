#include "elf_writer.h"
#include "binary_instrument_utilities.h"

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

struct data_block;
struct data_block
{
    struct data_block* next;
    void* data;
};

struct _ElfWriter
{
    Elf* e;
    struct data_block* first_data_block;
};

ElfWriter* elf_writer_create(Elf* e)
{
    ElfWriter* ew = malloc(sizeof(*ew));
    if(ew == NULL)
    {
        printf("Failed to allocate ElfWriter object.\n");
        return NULL;
    }
    ew->e = e;
    ew->first_data_block = NULL;
    
    return ew;
}

Elf* elf_writer_get_elf(ElfWriter* ew)
{
    return ew->e;
}

void elf_writer_destroy(ElfWriter* ew)
{
    struct data_block* block;
    struct data_block* next_block;
    for(block = ew->first_data_block; block != NULL; block = next_block)
    {
        next_block = block->next;
        free(block->data);
        free(block);
    }
    free(ew);
}

void* elf_writer_extend_section(ElfWriter* ew, Elf_Scn* scn,
    size_t additional_size, int align, size_t* offset)
{
    int result;
    Elf_Data* data;

    /*
     * Before adding new block of data touch all existing ones.
     * Otherwise there content will be zeroed by libelf(bug).
     * 
     *
     */
    for(data = elf_getdata(scn, NULL); data != NULL; data = elf_getdata(scn, data))
    {
        /*
         * Another bug in ELF - if read data from newly created section,
         * library return some object, in which d_version field will
         * be checked when elf_update().
         */
        data->d_version = EV_CURRENT;
    }
    
    void* buf = malloc(additional_size);
    if(buf == NULL)
    {
        printf("Failed to allocate additional buffer.\n");
        goto err_buf;
    }
    
    struct data_block* block = malloc(sizeof(*block));
    if(block == NULL)
    {
        printf("Failed to allocate block for store additional data.");
        goto err_block;
    }

    data = elf_newdata(scn);
    if(data == NULL)
    {
        printf("Failed to add new block of data to section: %s.\n",
            elf_errmsg(-1));
        // debug
        //Elf32_Shdr* shdr32 = elf32_getshdr(scn);
        //printf("Section alignment is %zu.\n", (size_t)shdr32->sh_addralign);
        goto err_newdata;
    }

    data->d_size = additional_size;
    data->d_buf = buf;
    data->d_align = align;
    data->d_off = 0LL;
    data->d_version = EV_CURRENT;
    
    block->data = buf;
    
    block->next = ew->first_data_block;
    ew->first_data_block = block;
    
    result = elf_update(ew->e, ELF_C_NULL);
    CHECK_ELF_FUNCTION_RESULT(result >= 0, elf_update);
    
    if(offset)
    {
        *offset = (size_t)data->d_off;
        //printf("Offset of the data appended: %zu.\n", *offset);
    }
    
    return buf;

err_newdata:
    free(block);
err_block:
    free(buf);
err_buf:
    return NULL;
}

int elf_writer_store_data(ElfWriter* ew, Elf_Data* data)
{
    struct data_block* block = malloc(sizeof(*block));
    if(block == NULL)
    {
        printf("Failed to allocate block for store existed data.");
        return -ENOMEM;
    }
    
    block->data = malloc(data->d_size);
    if(block->data == NULL)
    {
        printf("Failed to allocate buffer for store existed data.");
        free(block);
        return -ENOMEM;
    }
    
    memcpy(block->data, data->d_buf, data->d_size);
    data->d_buf = block->data;

    block->next = ew->first_data_block;
    ew->first_data_block = block;
    
    int result = elf_update(ew->e, ELF_C_NULL);
    CHECK_ELF_FUNCTION_RESULT(result >= 0, elf_update);
    
    return 0;
}