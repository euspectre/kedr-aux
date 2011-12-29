/*
 * Describe template of binary code.
 * 
 * This template may be instantiated in Elf file with different relocations.
 */

#ifndef BINARY_CODE_TEMPLATE_H
#define BINARY_CODE_TEMPLATE_H

#include <libelf.h>
#include "elf_writer.h"

struct code_template_rel;
struct code_template
{
    // Pointer to buffer with section code
    char* code;
    // Size of code.
    size_t code_size;
    //
    Elf64_Word align;
    /*
     * Array of relocations in the intermediate format.
     */
    struct code_template_rel* relocations;
    size_t n_relocations;
    // parameters affected on relocations
    int is_elf32;
    int rel_type;// SHT_RELA or SHT_REL
    // Array of symbol names, used in relocations
    char** symbols;
};



// Relocation in the intermediate format
struct code_template_rel
{
    Elf64_Addr offset;
    int32_t type; //type of relocation extracted from 'info' field.
    size_t symbol;//index of the symbol in the array
    Elf64_Sxword addend;
};


/*
 * Correspondence of symbols used for template instantiation.
 */
struct ct_symbol_correspondence
{
    // Name of symbol in the template.
    const char* name;
    // Index of symbol in the destination ELF.
    size_t index;
};

//For debug
void ct_print(struct code_template* template);

/*
 * Instantiate code template in the given section.
 * 
 * Relocations for the instantiated code are formed according to
 * 'symbols_correspondence' array.
 * Last element in this array should have 'name' field equal to NULL.
 * 
 * If section is newly created, its header should be set when call
 * given function.
 */
int ct_instantiate(struct code_template* template,
    ElfWriter* ew, Elf_Scn* scn,
    const struct ct_symbol_correspondence* symbol_correspondences,
    size_t* offset);

/*
 * Load section template from elf-file.
 */
struct code_template* ct_load(Elf* e, Elf_Scn* scn, size_t offset,
    size_t size, size_t align);

/*
 * Destroy code template.
 */
void ct_destroy(struct code_template* template);

#endif /* BINARY_CODE_TEMPLATE_H */