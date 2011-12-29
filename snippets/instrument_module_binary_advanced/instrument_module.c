#include <err.h>
#include <fcntl.h>

#include <gelf.h>
#include <libelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include <string.h>

#include <assert.h>

#include <sys/stat.h>

#include <errno.h>

#include "binary_instrument_utilities.h"
#include "binary_code_template.h"

#define REPLACEMENTS_SECTION ".replacements"

#define NEW_SECTION ".replacements.text"
#define NEW_SECTION_INIT ".init.replacements.text"
#define NEW_SECTION_EXIT ".exit.replacements.text"


/*
 * Load template of the function described by the given symbol.
 */
static struct code_template* function_template_load32(Elf* e, Elf32_Sym* func32)
{
    Elf_Scn* scn = elf_getscn(e, func32->st_shndx);
    assert(scn);
    
    Elf32_Shdr* shdr32 = elf32_getshdr(scn);
    assert(shdr32);
    
    return ct_load(e, scn, func32->st_value, func32->st_size,
        shdr32->sh_addralign);
}

static struct code_template* function_template_load64(Elf* e, Elf64_Sym* func64)
{
    Elf_Scn* scn = elf_getscn(e, func64->st_shndx);
    assert(scn);
    
    Elf64_Shdr* shdr64 = elf64_getshdr(scn);
    assert(shdr64);
    
    return ct_load(e, scn, func64->st_value, func64->st_size,
        shdr64->sh_addralign);
}

/*
 * Instantiate given template in the section with given name.
 * If such section doesn't exist, create it.
 * 
 * Then add symbol and fill it as a function, which code is continaed in
 * the template.
 * 
 * Return added symbol and, if 'index' is not NULL, set it to the index
 * of the symbol created.
 */
static Elf32_Sym* function_template_instantiate32(
    struct code_template* template,
    ElfWriter* ew,
    const char* section_name,
    const struct ct_symbol_correspondence* correspondences,
    const char* function_name,
    size_t *index)
{
    int result;
    
    Elf* e = elf_writer_get_elf(ew);

    Elf_Scn* added_scn = utils_elf_find_section(e, section_name);
    // Create additional section if it doesn't exist.
    if(added_scn == NULL)
    {
        size_t strndx;
        result = elf_getshdrstrndx(e, &strndx);
        CHECK_ELF_FUNCTION_RESULT(result == 0, elf_getshdrstrndx);

        size_t added_scn_name_offset;
        result = utils_elf_add_string(ew, elf_getscn(e, strndx),
            section_name, &added_scn_name_offset);
        assert(result == 0);

        added_scn = elf_newscn(e);
        CHECK_ELF_FUNCTION_RESULT(added_scn, elf_newscn);
        
        Elf32_Shdr* added_shdr32 = elf32_getshdr(added_scn);
        CHECK_ELF_FUNCTION_RESULT(added_shdr32, elf32_getshdr);
        
        added_shdr32->sh_name = added_scn_name_offset;
        added_shdr32->sh_type = SHT_PROGBITS;
        added_shdr32->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
        added_shdr32->sh_link = 0;
        added_shdr32->sh_info = 0;
        added_shdr32->sh_addralign = template->align;
        added_shdr32->sh_entsize = 0;
    }
    // Instantiate template
    size_t instantiate_offset;
    result = ct_instantiate(template, ew, added_scn, correspondences,
        &instantiate_offset);
    assert(result == 0);
    
    // Add function symbol
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    assert(symbols_scn);
    
    Elf32_Shdr* symbols_shdr32 = elf32_getshdr(symbols_scn);
    CHECK_ELF_FUNCTION_RESULT(symbols_shdr32, elf32_getshdr);
    
    Elf_Scn* symbols_name_scn = elf_getscn(e, symbols_shdr32->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_name_scn, elf_getscn);
   
    size_t function_name_offset;
    result = utils_elf_add_string(ew, symbols_name_scn,
        function_name, &function_name_offset);
    assert(result == 0);
    
    Elf32_Sym* function_sym32 = utils_elf32_add_symbol(ew,
         STT_FUNC, STB_GLOBAL, index);
    assert(function_sym32);

    function_sym32->st_name = function_name_offset;
    function_sym32->st_value = instantiate_offset;
    function_sym32->st_size = template->code_size;
    function_sym32->st_shndx = elf_ndxscn(added_scn);

    return function_sym32;
}

static Elf64_Sym* function_template_instantiate64(
    struct code_template* template,
    ElfWriter* ew,
    const char* section_name,
    const struct ct_symbol_correspondence* correspondences,
    const char* function_name,
    size_t *index)
{
    int result;
    
    Elf* e = elf_writer_get_elf(ew);

    Elf_Scn* added_scn = utils_elf_find_section(e, section_name);
    // Create additional section if it doesn't exist.
    if(added_scn == NULL)
    {
        size_t strndx;
        result = elf_getshdrstrndx(e, &strndx);
        CHECK_ELF_FUNCTION_RESULT(result == 0, elf_getshdrstrndx);

        size_t added_scn_name_offset;
        result = utils_elf_add_string(ew, elf_getscn(e, strndx),
            section_name, &added_scn_name_offset);
        assert(result == 0);

        added_scn = elf_newscn(e);
        CHECK_ELF_FUNCTION_RESULT(added_scn, elf_newscn);
        
        Elf64_Shdr* added_shdr64 = elf64_getshdr(added_scn);
        CHECK_ELF_FUNCTION_RESULT(added_shdr64, elf64_getshdr);
        
        added_shdr64->sh_name = added_scn_name_offset;
        added_shdr64->sh_type = SHT_PROGBITS;
        added_shdr64->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
        added_shdr64->sh_link = 0;
        added_shdr64->sh_info = 0;
        added_shdr64->sh_addralign = template->align;
        added_shdr64->sh_entsize = 0;
    }
    // Instantiate template
    size_t instantiate_offset;
    result = ct_instantiate(template, ew, added_scn, correspondences,
        &instantiate_offset);
    assert(result == 0);
    
    // Add function symbol
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    assert(symbols_scn);
    
    Elf64_Shdr* symbols_shdr64 = elf64_getshdr(symbols_scn);
    CHECK_ELF_FUNCTION_RESULT(symbols_shdr64, elf64_getshdr);
    
    Elf_Scn* symbols_name_scn = elf_getscn(e, symbols_shdr64->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_name_scn, elf_getscn);
   
    size_t function_name_offset;
    result = utils_elf_add_string(ew, symbols_name_scn,
        function_name, &function_name_offset);
    assert(result == 0);
    
    Elf64_Sym* function_sym64 = utils_elf64_add_symbol(ew,
         STT_FUNC, STB_GLOBAL, index);
    assert(function_sym64);

    function_sym64->st_name = function_name_offset;
    function_sym64->st_value = instantiate_offset;
    function_sym64->st_size = template->code_size;
    function_sym64->st_shndx = elf_ndxscn(added_scn);

    return function_sym64;
}



// Structure corresponded to 'struct template_replacement'.
struct template_replacement32
{
    Elf32_Addr orig;
    Elf32_Addr repl;
    Elf32_Addr intermediate;
    
    Elf32_Addr unused;
};

struct template_replacement64
{
    Elf64_Addr orig;
    Elf64_Addr repl;
    Elf64_Addr intermediate;
    
    Elf64_Addr unused;
};

/*
 * Structure contained all information needed for replace
 * one imported function in the target.
 */

struct imported_replacement_info
{
    char* orig_name;
    char* repl_name;
    char* intermediate_name;
    // crc of the replaced function(it is imported by template)
    int64_t repl_crc;
    // code template for intermediate function
    struct code_template* intermediate_template;
};

/*
 * Structure contained all information needed for replace
 * one link function in the target.
 * 
 * Examples of such functions: module_init, module_exit
 * 
 */
struct link_replacement_info
{
    char* link_name;// e.g. 'init_module'
    char* repl_name;// e.g. 'payload_init_module'
    char* intermediate_name;// e.g. 'init_module_intermediate'
    char* link_placeholder_name;// e.g. 'init_module_dummy'
    // crc of the replaced function(it is imported by template)
    int64_t repl_crc;
    // code template for intermediate function
    struct code_template* intermediate_template;
};

/*
 * Fill imported_replacement_info structure with needed values.
 */
static int imported_replacement_info_fill(
    struct imported_replacement_info *info,
    Elf* e,
    size_t orig_index,
    size_t repl_index,
    size_t intermediate_index)
{
    /*
     *  Assume that template elf is read only, so every its section
     * consists from only one data block.
     */

    void* result_gelf;
    
    int is_elf32 = utils_elf_is32(e);
    int use_crc = utils_elf_require_symbol_versions(e);
    
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    assert(symbols_scn);
    Elf_Data* symbols_data = elf_getdata(symbols_scn, NULL);
    assert(symbols_data);
    
    GElf_Shdr symbols_shdr;
    result_gelf = gelf_getshdr(symbols_scn, &symbols_shdr);
    assert(result_gelf);
    
    size_t symbols_name_scn_index = symbols_shdr.sh_link;
   
    GElf_Sym orig_sym;
    result_gelf = gelf_getsym(symbols_data, orig_index, &orig_sym);
    assert(result_gelf);
    
    const char* orig_name = elf_strptr(e, symbols_name_scn_index,
        orig_sym.st_name);
    assert(orig_name);
    
    info->orig_name = strdup(orig_name);
    assert(info->orig_name);
    
    GElf_Sym repl_sym;
    result_gelf = gelf_getsym(symbols_data, repl_index, &repl_sym);
    assert(result_gelf);
    
    const char* repl_name = elf_strptr(e, symbols_name_scn_index,
        repl_sym.st_name);
    assert(repl_name);
    
    info->repl_name = strdup(repl_name);
    assert(info->repl_name);
    
    if(use_crc)
    {
        info->repl_crc = is_elf32
            ? utils_elf32_get_symbol_required_version(e, info->repl_name)
            : utils_elf64_get_symbol_required_version(e, info->repl_name);
    }
    
    // Extract intermediate name and set its template depended on ELF bits
    const char* intermediate_name;
    if(is_elf32)
    {
        assert(intermediate_index * sizeof(Elf32_Sym) < symbols_data->d_size);
        Elf32_Sym* intermediate_sym32 =
            (Elf32_Sym*)symbols_data->d_buf + intermediate_index;
        intermediate_name = elf_strptr(e, symbols_name_scn_index,
            intermediate_sym32->st_name);
        info->intermediate_template =
            function_template_load32(e, intermediate_sym32);
    }
    else
    {
        assert(intermediate_index * sizeof(Elf64_Sym) < symbols_data->d_size);
        Elf64_Sym* intermediate_sym64 =
            (Elf64_Sym*)symbols_data->d_buf + intermediate_index;
        intermediate_name = elf_strptr(e, symbols_name_scn_index,
            intermediate_sym64->st_name);
        info->intermediate_template =
            function_template_load64(e, intermediate_sym64);
    }
    assert(info->intermediate_template);
    
    info->intermediate_name = strdup(intermediate_name);
    assert(info->intermediate_name);
    
    return 0;
}

static void imported_replacement_info_destroy(
    struct imported_replacement_info* info)
{
    ct_destroy(info->intermediate_template);
    free(info->intermediate_name);
    free(info->repl_name);
    free(info->orig_name);
}


// Common function for fill replacement information for link function.
static int link_replacement_info_fill(
    struct link_replacement_info *info,
    Elf* e,
    const char* link_name,
    const char* repl_name,
    const char* intermediate_name,
    const char* link_placeholder_name)
{
    int is_elf32 = utils_elf_is32(e);
#define STORE_STR(var) info->var = strdup(var); assert(info->var)
    STORE_STR(link_name);
    STORE_STR(repl_name);
    STORE_STR(intermediate_name);
    STORE_STR(link_placeholder_name);
#undef STORE_STR

    if(utils_elf_require_symbol_versions(e))
    {
        info->repl_crc = is_elf32
            ? utils_elf32_get_symbol_required_version(e, repl_name)
            : utils_elf64_get_symbol_required_version(e, repl_name);
    }

    if(is_elf32)
    {
        Elf32_Sym* sym32 = utils_elf32_find_symbol(e, intermediate_name, NULL);
        assert(sym32);
        
        info->intermediate_template = function_template_load32(e, sym32);
    }
    else
    {
        Elf64_Sym* sym64 = utils_elf64_find_symbol(e, intermediate_name, NULL);
        assert(sym64);
        
        info->intermediate_template = function_template_load64(e, sym64);
    }
    assert(info->intermediate_template);
    
    return 0;
}

static void link_replacement_info_destroy(
    struct link_replacement_info* info)
{
    free(info->link_name);
    free(info->repl_name);
    free(info->intermediate_name);
    free(info->link_placeholder_name);
    
    ct_destroy(info->intermediate_template);
}

/*
 * Special variants for init and exit replacements.
 */
static int link_replacement_info_fill_init(
    struct link_replacement_info* info,
    Elf* e)
{
    return link_replacement_info_fill(info, e, "init_module",
        "payload_init_target", "init_module_intermediate",
        "init_module_dummy");
}

static int link_replacement_info_fill_exit(
    struct link_replacement_info* info,
    Elf* e)
{
    return link_replacement_info_fill(info, e, "cleanup_module",
        "payload_exit_target", "exit_module_intermediate",
        "exit_module_dummy");
}


/*
 * Extract replacements from template.
 * 
 * Returning array contains symbols indexes in corresponded fields
 * (not a symbol values).
 * 
 * After call, 'n_replacements' will contain number of replacements.
 * 
 * Returning array should be freed when not needed.
 */
static struct template_replacement32*
get_template_replacements32(Elf* template_e,
    Elf_Scn* replacements_scn,
    size_t* n_replacements)
{
    /*
     *  Assume that template elf is read only, so every its section
     * consists from only one data block.
     */

    struct template_replacement32* replacements;
    
    Elf_Scn* replacements_rel_scn = utils_elf_find_relocation_section(
        template_e, elf_ndxscn(replacements_scn));
    assert(replacements_rel_scn);
    
    Elf32_Shdr* replacements_shdr32 = elf32_getshdr(replacements_scn);
    assert(replacements_shdr32);
    
    assert(replacements_shdr32->sh_size % sizeof(*replacements) == 0);
    
    *n_replacements = replacements_shdr32->sh_size / sizeof(*replacements);
    
    replacements = malloc(*n_replacements * sizeof(*replacements));
    assert(replacements);
    
    Elf32_Shdr* replacements_rel_shdr32 =
        elf32_getshdr(replacements_rel_scn);
    assert(replacements_rel_shdr32);
    
    Elf_Data* replacements_rel_data = elf_getdata(replacements_rel_scn, NULL);
    assert(replacements_rel_data);
    
    if(replacements_rel_shdr32->sh_type == SHT_RELA)
    {
        Elf32_Rela* rela32;
        Elf32_Rela* rela32_end = (Elf32_Rela*)
            ((char*)replacements_rel_data->d_buf
            + replacements_rel_data->d_size);
        for(rela32 = replacements_rel_data->d_buf;
            rela32 < rela32_end;
            rela32++)
        {
            Elf32_Addr* addr = (Elf32_Addr*)
                ((char*)replacements + rela32->r_offset);
            *addr = ELF32_R_SYM(rela32->r_info);
        }
    }
    else
    {
        Elf32_Rel* rel32;
        Elf32_Rel* rel32_end = (Elf32_Rel*)
            ((char*)replacements_rel_data->d_buf
            + replacements_rel_data->d_size);
        for(rel32 = replacements_rel_data->d_buf;
            rel32 < rel32_end;
            rel32++)
        {
            Elf32_Addr* addr = (Elf32_Addr*)
                ((char*)replacements + rel32->r_offset);
            *addr = ELF32_R_SYM(rel32->r_info);
        }
    }
    
    return replacements;
}

static struct template_replacement64*
get_template_replacements64(Elf* template_e,
    Elf_Scn* replacements_scn,
    size_t* n_replacements)
{
    /*
     *  Assume that template elf is read only, so every its section
     * consists from only one data block.
     */

    struct template_replacement64* replacements;
    
    Elf_Scn* replacements_rel_scn = utils_elf_find_relocation_section(
        template_e, elf_ndxscn(replacements_scn));
    assert(replacements_rel_scn);
    
    Elf64_Shdr* replacements_shdr64 = elf64_getshdr(replacements_scn);
    assert(replacements_shdr64);
    
    assert(replacements_shdr64->sh_size % sizeof(*replacements) == 0);
    
    *n_replacements = replacements_shdr64->sh_size / sizeof(*replacements);
    
    replacements = malloc(*n_replacements * sizeof(*replacements));
    assert(replacements);
    
    Elf64_Shdr* replacements_rel_shdr64 =
        elf64_getshdr(replacements_rel_scn);
    assert(replacements_rel_shdr64);
    
    Elf_Data* replacements_rel_data = elf_getdata(replacements_rel_scn, NULL);
    assert(replacements_rel_data);
    
    if(replacements_rel_shdr64->sh_type == SHT_RELA)
    {
        Elf64_Rela* rela64;
        Elf64_Rela* rela64_end = (Elf64_Rela*)
            ((char*)replacements_rel_data->d_buf
            + replacements_rel_data->d_size);
        for(rela64 = replacements_rel_data->d_buf;
            rela64 < rela64_end;
            rela64++)
        {
            Elf64_Addr* addr = (Elf64_Addr*)
                ((char*)replacements + rela64->r_offset);
            *addr = ELF64_R_SYM(rela64->r_info);
        }
    }
    else
    {
        Elf64_Rel* rel64;
        Elf64_Rel* rel64_end = (Elf64_Rel*)
            ((char*)replacements_rel_data->d_buf
            + replacements_rel_data->d_size);
        for(rel64 = replacements_rel_data->d_buf;
            rel64 < rel64_end;
            rel64++)
        {
            Elf64_Addr* addr = (Elf64_Addr*)
                ((char*)replacements + rel64->r_offset);
            *addr = ELF64_R_SYM(rel64->r_info);
        }
    }
    
    return replacements;
}


/*
 * Perform replacement in the target module, if needed.
 * 
 * Return 0 on success, negative error code on fail and 1 if
 * nothing to replace(ELF doesn't contain original symbol).
 * 
 */
static int replace_import32(struct imported_replacement_info* replacement_info,
    ElfWriter* ew)
{
    int result;
    Elf* e = elf_writer_get_elf(ew);
    
    Elf32_Sym* orig_sym = utils_elf32_find_symbol(e,
        replacement_info->orig_name, NULL);
    if(orig_sym == NULL) return 1; //Nothing to replace
    
    struct ct_symbol_correspondence correspondences[] =
    {
        { .name = NULL/*replacement should be here*/, .index = -1},
        { .name = "mcount", .index = -1},
        { .name = "__this_module", .index = -1},
        { .name = NULL }
    };

    // Add replacement function as symbol, and add its index to correspondences.
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    assert(symbols_scn);
    
    Elf32_Shdr* symbols_shdr32 = elf32_getshdr(symbols_scn);
    CHECK_ELF_FUNCTION_RESULT(symbols_shdr32, elf32_getshdr);
    
    Elf_Scn* symbols_name_scn = elf_getscn(e, symbols_shdr32->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_name_scn, elf_getscn);

    correspondences[0].name = replacement_info->repl_name;

    size_t repl_name_offset;
    result = utils_elf_add_string(ew, symbols_name_scn,
        replacement_info->repl_name, &repl_name_offset);
    assert(result == 0);
    
    result = utils_elf_add_symbol_imported(ew, repl_name_offset,
        &correspondences[0].index);
    assert(result == 0);
    
    if(utils_elf_require_symbol_versions(e))
    {
        result = utils_elf32_add_symbol_version(ew,
            replacement_info->repl_name, replacement_info->repl_crc);
        assert(result == 0);
    }

    Elf32_Sym* sym32;
    // Fill mcount symbol info
    sym32 = utils_elf32_find_symbol(e, correspondences[1].name,
        &correspondences[1].index);
    assert(sym32);
    
    // Fill __this_module symbol info
    sym32 = utils_elf32_find_symbol(e, correspondences[2].name,
        &correspondences[2].index);
    assert(sym32);

    // Add intermediate as function symbol...
    Elf32_Sym* intermediate_sym32 = function_template_instantiate32(
        replacement_info->intermediate_template,
        ew,
        NEW_SECTION,
        correspondences,
        replacement_info->intermediate_name,
        NULL);
    assert(intermediate_sym32);
    
    //.. And exchange it with original symbol
    Elf32_Sym sym_tmp;
    memcpy(&sym_tmp, orig_sym, sizeof(sym_tmp));
    memcpy(orig_sym, intermediate_sym32, sizeof(sym_tmp));
    memcpy(intermediate_sym32, &sym_tmp, sizeof(sym_tmp));
    
    return 0;
}

static int replace_import64(struct imported_replacement_info* replacement_info,
    ElfWriter* ew)
{
    int result;
    Elf* e = elf_writer_get_elf(ew);
    
    Elf64_Sym* orig_sym = utils_elf64_find_symbol(e,
        replacement_info->orig_name, NULL);
    if(orig_sym == NULL) return 1; //Nothing to replace
    
    struct ct_symbol_correspondence correspondences[] =
    {
        { .name = NULL/*replacement should be here*/, .index = -1},
        { .name = "mcount", .index = -1},
        { .name = "__this_module", .index = -1},
        { .name = NULL }
    };

    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    assert(symbols_scn);
    
    Elf64_Shdr* symbols_shdr64 = elf64_getshdr(symbols_scn);
    CHECK_ELF_FUNCTION_RESULT(symbols_shdr64, elf64_getshdr);
    
    Elf_Scn* symbols_name_scn = elf_getscn(e, symbols_shdr64->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_name_scn, elf_getscn);
    // Add replacement function as symbol, and add its index to correspondences.
    correspondences[0].name = replacement_info->repl_name;

    size_t repl_name_offset;
    result = utils_elf_add_string(ew, symbols_name_scn,
        replacement_info->repl_name, &repl_name_offset);
    assert(result == 0);
    
    result = utils_elf_add_symbol_imported(ew, repl_name_offset,
        &correspondences[0].index);
    assert(result == 0);
    
    if(utils_elf_require_symbol_versions(e))
    {
        result = utils_elf64_add_symbol_version(ew,
            replacement_info->repl_name, replacement_info->repl_crc);
        assert(result == 0);
    }

    Elf64_Sym* sym64;
    // Fill mcount symbol info
    sym64 = utils_elf64_find_symbol(e, correspondences[1].name,
        &correspondences[1].index);
    assert(sym64);
    
    // Fill __this_module symbol info
    sym64 = utils_elf64_find_symbol(e, correspondences[2].name,
        &correspondences[2].index);
    assert(sym64);

    // Add intermediate as function symbol...
    Elf64_Sym* intermediate_sym64 = function_template_instantiate64(
        replacement_info->intermediate_template,
        ew,
        NEW_SECTION,
        correspondences,
        replacement_info->intermediate_name,
        NULL);
    assert(intermediate_sym64);
    
    //.. And exchange it with original symbol
    Elf64_Sym sym_tmp;
    memcpy(&sym_tmp, orig_sym, sizeof(sym_tmp));
    memcpy(orig_sym, intermediate_sym64, sizeof(sym_tmp));
    memcpy(intermediate_sym64, &sym_tmp, sizeof(sym_tmp));
    
    return 0;
}

// Helper for found original symbol for link
static Elf32_Sym* link_get_orig32(Elf* e, Elf32_Sym* link32, size_t* offset)
{
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    assert(symbols_scn);
    
    Elf_Data* symbols_data;
    for(symbols_data = elf_getdata(symbols_scn, NULL);
        symbols_data != NULL;
        symbols_data = elf_getdata(symbols_scn, symbols_data))
    {
        Elf32_Sym* sym32;
        Elf32_Sym* sym32_end = (Elf32_Sym*)
            ((char*)symbols_data->d_buf + symbols_data->d_size);
        for(sym32 = symbols_data->d_buf; sym32 < sym32_end; sym32++)
        {
            if((sym32->st_shndx == link32->st_shndx)
                && (sym32->st_value == link32->st_value)
                && (sym32->st_size == link32->st_size))
            {
                if(sym32->st_name == link32->st_name) continue;// Ignore link itself.
                if(offset)
                {
                    *offset = sym32 - (Elf32_Sym*)
                        ((char*)symbols_data->d_buf - symbols_data->d_off);
                }
                return sym32;
            }
        }
    }
    
    return NULL;
}

// Same for link replacements
static int replace_link32(struct link_replacement_info* replacement_info,
    ElfWriter* ew, const char* new_section_name)
{
    int result;
    
    Elf* e = elf_writer_get_elf(ew);
    
    Elf32_Sym* link_sym32 = utils_elf32_find_symbol(e,
        replacement_info->link_name, NULL);
    if(link_sym32 == NULL) return 1;// nothing to replace
    
    struct ct_symbol_correspondence correspondences[] =
    {
        { .name = NULL/*replacement should be here*/, .index = -1},
        { .name = NULL/*link_placeholder->orig should be here*/, .index = -1},
        { .name = "mcount", .index = -1},
        { .name = "__this_module", .index = -1},
        { .name = NULL }
    };

    // Add replacement function as symbol, and add its index to correspondences.
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    assert(symbols_scn);
    
    Elf32_Shdr* symbols_shdr32 = elf32_getshdr(symbols_scn);
    CHECK_ELF_FUNCTION_RESULT(symbols_shdr32, elf32_getshdr);
    
    Elf_Scn* symbols_name_scn = elf_getscn(e, symbols_shdr32->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_name_scn, elf_getscn);

    correspondences[0].name = replacement_info->repl_name;

    size_t repl_name_offset;
    result = utils_elf_add_string(ew, symbols_name_scn,
        replacement_info->repl_name, &repl_name_offset);
    assert(result == 0);
    
    result = utils_elf_add_symbol_imported(ew, repl_name_offset,
        &correspondences[0].index);
    assert(result == 0);
    
    if(utils_elf_require_symbol_versions(e))
    {
        result = utils_elf32_add_symbol_version(ew,
            replacement_info->repl_name, replacement_info->repl_crc);
        assert(result == 0);
    }

    // As link placeholder set symbol to which link points
    correspondences[1].name = replacement_info->link_placeholder_name;
    
    Elf32_Sym* orig_sym32 = link_get_orig32(e, link_sym32,
        &correspondences[1].index);
    
    if(orig_sym32 == NULL)
    {
        printf("Cannot find original symbol for link %s.\n",
            replacement_info->link_name);
        return -EINVAL;
    }
    
    Elf32_Sym* sym32;
    // Fill mcount symbol info
    sym32 = utils_elf32_find_symbol(e, correspondences[2].name,
        &correspondences[2].index);
    assert(sym32);
    
    // Fill __this_module symbol info
    sym32 = utils_elf32_find_symbol(e, correspondences[3].name,
        &correspondences[3].index);
    assert(sym32);

    // Add intermediate as function symbol...
    Elf32_Sym* intermediate_sym32 = function_template_instantiate32(
        replacement_info->intermediate_template,
        ew,
        new_section_name,
        correspondences,
        replacement_info->intermediate_name,
        NULL);
    assert(intermediate_sym32);

    //.. and set link pointed to it instead of original symbol
    link_sym32->st_shndx = intermediate_sym32->st_shndx;
    link_sym32->st_value = intermediate_sym32->st_value;
    link_sym32->st_size = intermediate_sym32->st_size;
    
    return 0;
}


int
main(int argc, char** argv)
{
    int result;
    const char* input_filename;
    const char* template_name;
    const char* filename;
    
    int i;

    if(argc != 4)
    {
        printf("Usage: %s <filename> <template> <outfilename>.\n", argv[0]);
        return 1;
    }
    
    input_filename = argv[1];
    template_name = argv[2];
    filename = argv[3];
    
    if (elf_version(EV_CURRENT) == EV_NONE)
        errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));

    int template_fd;
    Elf* template_e;
    
    if((template_fd = open(template_name, O_RDONLY, 0)) < 0)
        err(EX_NOINPUT, "open \"%s\" failed", template_name);

    template_e = elf_begin(template_fd, ELF_C_READ, NULL);
    if(!template_e)
        errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));
    
    if(elf_kind(template_e) != ELF_K_ELF)
        errx(EX_DATAERR, "Input file should be in ELF format");

    int is_elf32 = utils_elf_is32(template_e);;
    
    Elf_Scn* replacements_scn = utils_elf_find_section(template_e,
        ".replacements");
    assert(replacements_scn);
    
    size_t n_replacements;
    struct imported_replacement_info* replacements_info;
    
    if(is_elf32)
    {
        struct template_replacement32* replacements32 =
            get_template_replacements32(template_e, replacements_scn,
                                        &n_replacements);
        assert(replacements32);
        
        replacements_info = malloc(n_replacements * sizeof(*replacements_info));
        assert(replacements_info);
        

        for(i = 0; i < n_replacements; i++)
        {
            int result = imported_replacement_info_fill(
                &replacements_info[i],
                template_e,
                replacements32[i].orig,
                replacements32[i].repl,
                replacements32[i].intermediate);
            assert(result == 0);
        }
        
        free(replacements32);
    }
    else
    {
        struct template_replacement64* replacements64 =
            get_template_replacements64(template_e, replacements_scn,
                                        &n_replacements);
        assert(replacements64);
        
        replacements_info = malloc(n_replacements * sizeof(*replacements_info));
        assert(replacements_info);
        

        for(i = 0; i < n_replacements; i++)
        {
            int result = imported_replacement_info_fill(
                &replacements_info[i],
                template_e,
                replacements64[i].orig,
                replacements64[i].repl,
                replacements64[i].intermediate);
            assert(result == 0);
        }
        
        free(replacements64);
    }
    // debug
    //printf("Replacements from template:\n");
    //for(i = 0; i < n_replacements; i++)
    //{
        //struct imported_replacement_info* info = &replacements_info[i];
        //printf("\t%d.%s -> %s(use %s with crc=%zu)\n", i + 1, info->orig_name,
            //info->intermediate_name, info->repl_name, (size_t)info->repl_crc);
        //printf("Intermediate template is:\n");
        //ct_print(info->intermediate_template);
        //printf("\n");
    //}

    struct link_replacement_info init_replacement_info;
    result = link_replacement_info_fill_init(&init_replacement_info,
        template_e);
    assert(result == 0);

    struct link_replacement_info exit_replacement_info;
    result = link_replacement_info_fill_exit(&exit_replacement_info,
        template_e);
    assert(result == 0);


    elf_end(template_e);
    close(template_fd);
    
    int input_fd;
    Elf* input_e;
    
    if((input_fd = open(input_filename, O_RDONLY, 0)) < 0)
        err(EX_NOINPUT, "Failed to open \"%s\" file for read.", input_filename);

    input_e = elf_begin(input_fd, ELF_C_READ, NULL);
    if(!input_e)
        errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));
    
    if(elf_kind(input_e) != ELF_K_ELF)
        errx(EX_DATAERR, "Input file should be in ELF format");
    
    if(utils_elf_is32(input_e))
    {
        if(!is_elf32)
            errx(EX_DATAERR, "Input file is 32bit, but template is 64bit");
    }
    else
    {
        if(is_elf32)
            errx(EX_DATAERR, "Input file is 64bit, but template is 32bit");
    }

    int fd;
    Elf* e;
    ElfWriter* ew;

    if((fd = creat(filename, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)) < 0)
        err(EX_NOINPUT, "Failed to create output file \"%s\"", filename);

    e = elf_begin(fd, ELF_C_WRITE, NULL);
    if(!e)
        errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));
    
    ew = elf_writer_create(e);
    assert(ew);
    // Copy ELF content from input file to output and close input.
    result = is_elf32
        ? utils_elf32_copy_elf(ew, input_e)
        : utils_elf64_copy_elf(ew, input_e);
    assert(result == 0);
    
    elf_end(input_e);
    close(input_fd);

    for(i = 0; i < n_replacements; i++)
    {
        result = is_elf32
            ? replace_import32(&replacements_info[i], ew)
            : replace_import64(&replacements_info[i], ew);
        assert(result >= 0);
        if(result == 0)
        {
            printf("Function %s has been replaced.\n",
                replacements_info[i].orig_name);
        }
    }

    result = replace_link32(&init_replacement_info, ew, NEW_SECTION_INIT);
    assert(result >= 0);

    if(result == 0)
        printf("Init function has been replaced.\n");

    result = replace_link32(&exit_replacement_info, ew, NEW_SECTION_EXIT);
    assert(result >= 0);

    if(result == 0)
        printf("Exit function has been replaced.\n");

    result = elf_update(e, ELF_C_WRITE);
    CHECK_ELF_FUNCTION_RESULT(result >= 0, elf_update);
    
    elf_writer_destroy(ew);
    elf_end(e);

    for(i = 0; i < n_replacements; i++)
    {
        imported_replacement_info_destroy(&replacements_info[i]);
    }
    
    free(replacements_info);
    
    link_replacement_info_destroy(&init_replacement_info);
    link_replacement_info_destroy(&exit_replacement_info);
    
    return 0;
}
