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

#include "wrapper.h"

#define CHECK_ELF_FUNCTION_RESULT(cond, function) \
    while(!(cond)) {errx(EX_SOFTWARE, "%s() failed: %s.", #function, elf_errmsg(-1)); }

/*
 * Determine, whether ELF class is 32 or 64.
 * 
 * In first case return non-zero, in the second one - zero.
 */
static int elf_is32(Elf* e)
{
    char* ident;

    if(elf_kind(e) != ELF_K_ELF)
        errx(EX_DATAERR, "File should be in ELF format");
    
    ident = elf_getident(e, NULL);
    CHECK_ELF_FUNCTION_RESULT(ident, getident);
    
    switch(ident[EI_CLASS])
    {
    case ELFCLASS32:
        return 1;
    break;
    case ELFCLASS64:
        return 0;
    break;
    default:
        errx(EX_SOFTWARE, "Invalid Elf class.");
    }
}

/*
 * Record about symbol version in ELF-file(crossplatform).
 */
struct symVersion32
{
    int32_t crc;
    char name[0];
};

struct symVersion64
{
    int64_t crc;
    char name[0];
};


#define align_val(val, alignment) (((val) + ((alignment) - 1)) / (alignment) * (alignment))

/*
 * Return next record about symbol version.
 * 
 * (record should be aligned on 64 bytes)
 */
static struct symVersion32* symVersion32_next(struct symVersion32* symVer)
{
    int size = sizeof(symVer->crc) + strlen(symVer->name) + 1;
    
    return (struct symVersion32*)((char*)symVer + align_val(size, 64));
}

static struct symVersion64* symVersion64_next(struct symVersion64* symVer)
{
    int size = sizeof(symVer->crc) + strlen(symVer->name) + 1;
    
    return (struct symVersion64*)((char*)symVer + align_val(size, 64));
}


/*
 * Description of one function for replace.
 */
struct replacement
{
    char* function_name;// allocated
    char* replacement_name;// allocated
    int32_t replacement_crc;
};

/*
 *  Destroy replacements array.
 * 
 * Creation functions will be define later.
 * 
 * Replacements array is array of structures 'replacement', last element
 * in array have NULL as function_name.
 */
static void replacements_destroy(struct replacement* replacements)
{
    struct replacement* replacement_tmp;
    for(replacement_tmp = replacements; replacement_tmp->function_name; replacement_tmp++)
    {
        free(replacement_tmp->function_name);
        free(replacement_tmp->replacement_name);
    }
    
    free(replacements);
}

/*
 * Print replacement array content. Used for debug.
 */
static void replacements_print(const struct replacement* replacements)
{
    const struct replacement* replacement_tmp;
    if(replacements->function_name == NULL)
    {
        printf("Replacement array is empty.\n");
        return;
    }
    printf("Replacement array has next replacements:\n");
    for(replacement_tmp = replacements; replacement_tmp->function_name;
        replacement_tmp++)
    {
        printf("{%s, %s}\n", replacement_tmp->function_name,
            replacement_tmp->replacement_name);
    }
}


/*
 * Return replacement for given function name.
 * If not found, return NULL.
 */
static const struct replacement* replacement_find(const char* function_name,
    const struct replacement* replacements)
{
    const struct replacement* repl;
    for(repl = replacements; repl->function_name != NULL; repl++)
    {
        if(strcmp(function_name, repl->function_name) == 0)
        {
            return repl;
        }
    }
    return NULL;
}

/*
 * If section is a symbol section, replace all needed functions with
 * corresponded ones.
 * 
 * Return 0 if at least one symbol was replaced, 1 otherwise.
 */
static int replace_functions_in_section32(Elf* e, Elf_Scn* scn,
    const struct replacement* replacements)
{
    Elf_Data* data;
    int is_replaced = 0;
    
    Elf32_Sym *sym32, *sym32_end;
    Elf32_Shdr* shdr32 = elf32_getshdr(scn);
    CHECK_ELF_FUNCTION_RESULT(shdr32, getshdr);
    
    if(shdr32->sh_type != SHT_SYMTAB) return 1;
    
    data = elf_getdata(scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, getdata);
    
    sym32_end = (typeof(sym32_end))((char*)data->d_buf + data->d_size);
    
    for(sym32 = (typeof(sym32))data->d_buf; sym32 < sym32_end; sym32++)
    {
        char* symbol_name;
        const struct replacement* repl;
        
        if(ELF32_ST_BIND(sym32->st_info) != STB_GLOBAL) continue;
        if(sym32->st_shndx != SHN_UNDEF) continue;
        
        symbol_name = elf_strptr(e, shdr32->sh_link, sym32->st_name);
        CHECK_ELF_FUNCTION_RESULT(symbol_name, strptr);
        
        repl = replacement_find(symbol_name, replacements);
        
        if(repl)
        {
            strcpy(symbol_name, repl->replacement_name);
            printf("Function %s has been replaced with %s.\n",
                repl->function_name, repl->replacement_name);
            is_replaced = 1;
        }
    }
    
    if(is_replaced) elf_flagdata(data, ELF_C_SET, ELF_F_DIRTY);
    return is_replaced ? 0 : 1;
}


static int replace_functions_in_section64(Elf* e, Elf_Scn* scn,
    const struct replacement* replacements)
{
    Elf_Data* data;
    int is_replaced = 0;
    
    Elf64_Sym *sym64, *sym64_end;
    Elf64_Shdr* shdr64 = elf64_getshdr(scn);
    CHECK_ELF_FUNCTION_RESULT(shdr64, getshdr);
    
    if(shdr64->sh_type != SHT_SYMTAB) return 1;
    
    data = elf_getdata(scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, getdata);
    
    sym64_end = (typeof(sym64_end))((char*)data->d_buf + data->d_size);
    
    for(sym64 = (typeof(sym64))data->d_buf; sym64 < sym64_end; sym64++)
    {
        char* symbol_name;
        const struct replacement* repl;
        
        if(ELF64_ST_BIND(sym64->st_info) != STB_GLOBAL) continue;
        if(sym64->st_shndx != SHN_UNDEF) continue;
        
        symbol_name = elf_strptr(e, shdr64->sh_link, sym64->st_name);
        CHECK_ELF_FUNCTION_RESULT(symbol_name, strptr);
        
        repl = replacement_find(symbol_name, replacements);
        
        if(repl)
        {
            strcpy(symbol_name, repl->replacement_name);
            printf("Function %s has been replaced with %s.\n",
                repl->function_name, repl->replacement_name);
            is_replaced = 1;
        }
    }
    
    if(is_replaced) elf_flagdata(data, ELF_C_SET, ELF_F_DIRTY);
    return is_replaced ? 0 : 1;
}


/*
 * Return identificator for program section with given name.
 * 
 * If such section is absent, return NULL.
 * 
 * Used for different purposes.
 */
static Elf_Scn* get_named_section(Elf* e, const char* section_name)
{
    Elf_Scn* scn;
    int result;
    size_t shdrstrndx;
    
    int is_elf32 = elf_is32(e);
   
    result = elf_getshdrstrndx(e, &shdrstrndx);
    CHECK_ELF_FUNCTION_RESULT(result == 0, getshdrstrndx);

    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        size_t section_name_index;
        
        if(is_elf32)
        {
            Elf32_Shdr* shdr32 = elf32_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr32, getshdr);

            if(shdr32->sh_type != SHT_PROGBITS) continue;
            section_name_index = shdr32->sh_name;
        }
        else
        {
            Elf64_Shdr* shdr64 = elf64_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr64, getshdr);

            if(shdr64->sh_type != SHT_PROGBITS) continue;
            section_name_index = shdr64->sh_name;
        }

        const char* _section_name = elf_strptr(e, shdrstrndx,
            section_name_index);
        CHECK_ELF_FUNCTION_RESULT(_section_name, strptr);

        if(strcmp(_section_name, section_name) == 0) return scn;
    }
   
    return NULL;
}


/*
 * Return identificator for relocation section for given one.
 * 
 * If such section is absent, return NULL.
 * 
 * Used for different purposes.
 */
static Elf_Scn* get_relocation_section(Elf* e, Elf_Scn* base_scn)
{
    Elf_Scn* scn;

    int is_elf32 = elf_is32(e);

    size_t base_ndxscn = elf_ndxscn(base_scn);
    CHECK_ELF_FUNCTION_RESULT(base_ndxscn, ndxscn);

    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        if(is_elf32)
        {
            Elf32_Shdr* shdr32 = elf32_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr32, getshdr);

            if((shdr32->sh_type == SHT_REL) || (shdr32->sh_type == SHT_RELA))
            {
                if(shdr32->sh_info == base_ndxscn) return scn;
            }
        }
        else
        {
            Elf64_Shdr* shdr64 = elf64_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr64, getshdr);

            if((shdr64->sh_type == SHT_REL) || (shdr64->sh_type == SHT_RELA))
            {
                if(shdr64->sh_info == base_ndxscn) return scn;
            }
        }

    }
   
    return NULL;
}

/*
 * Return identificator for section containing symbol versions in
 * kernel module.
 * 
 * If such section is absent, return NULL.
 */
static Elf_Scn* get_versions_section(Elf* e)
{
    return get_named_section(e, "__versions");
}

/*
 * Look for section with symbol versions and replace needed functions
 * and its versions with corresponded ones.
 * 
 * Return 0 if at least one function was replaced, 1 otherwise.
 */
static int replace_functions_versions32(Elf* e, Elf_Scn* scn_versions,
    const struct replacement* replacements)
{
    Elf_Data* data;
    struct symVersion32* symVer32, *symVer32_end;
    int is_replaced = 0;
    
    data = elf_getdata(scn_versions, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, getdata);
    
    symVer32_end = (typeof(symVer32_end))((char*)data->d_buf + data->d_size);
        
    for(symVer32 = (typeof(symVer32))data->d_buf;
        symVer32 < symVer32_end;
        symVer32 = symVersion32_next(symVer32))
    {
        //printf("%.8x %s\n", symVer->crc, symVer->name);
        const struct replacement* repl = replacement_find(
            symVer32->name,
            replacements);
        if(repl)
        {
            symVer32->crc = repl->replacement_crc;
            strcpy(symVer32->name, repl->replacement_name);
            printf("Versioning function %s has been replaced with (%8x, %s).\n",
                repl->function_name, repl->replacement_crc, repl->replacement_name);
            is_replaced = 1;
        }
    }
    
    if(is_replaced) elf_flagdata(data, ELF_C_SET, ELF_F_DIRTY);
    return is_replaced ? 0 : 1;
}

static int replace_functions_versions64(Elf* e, Elf_Scn* scn_versions,
    const struct replacement* replacements)
{
    Elf_Data* data;
    struct symVersion64* symVer64, *symVer64_end;
    int is_replaced = 0;
    
    data = elf_getdata(scn_versions, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, getdata);
    
    symVer64_end = (typeof(symVer64_end))((char*)data->d_buf + data->d_size);
        
    for(symVer64 = (typeof(symVer64))data->d_buf;
        symVer64 < symVer64_end;
        symVer64 = symVersion64_next(symVer64))
    {
        //printf("%.8x %s\n", symVer->crc, symVer->name);
        const struct replacement* repl = replacement_find(
            symVer64->name,
            replacements);
        if(repl)
        {
            symVer64->crc = repl->replacement_crc;
            strcpy(symVer64->name, repl->replacement_name);
            printf("Versioning function %s has been replaced with (%8x, %s).\n",
                repl->function_name, repl->replacement_crc, repl->replacement_name);
            is_replaced = 1;
        }
    }
    
    if(is_replaced) elf_flagdata(data, ELF_C_SET, ELF_F_DIRTY);
    return is_replaced ? 0 : 1;
}

static int replace_functions_in_module(const char* module_filename,
    const struct replacement* replacements, int has_versions)
{
    int fd;
    
    Elf* e;
    
    Elf_Scn* scn;
    Elf_Scn* scn_versions;
    
    int is_elf32;

    if (elf_version(EV_CURRENT) == EV_NONE )
        errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));
    if((fd = open(module_filename, O_RDWR, 0)) < 0)
        err(EX_NOINPUT, "open \"%s\" failed", module_filename);

    e = elf_begin(fd, ELF_C_RDWR, NULL);
    if(!e)
        errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));
    
    if(elf_kind(e) != ELF_K_ELF)
        errx(EX_DATAERR, "Input file should be in ELF format");
    
    is_elf32 = elf_is32(e);
    
    scn_versions = get_versions_section(e);
    if(scn_versions)
    {
        if(!has_versions)
        {
            printf("Wrapper module contains no symbol versions information, but instrumented module requires it.\n");
            return -1;
        }
    }
    else
    {
        if(has_versions)
        {
            printf("Warning: wrapper module contains symbol versions information, but intstrumented module doesn't require it.\n");
            //not an error
        }
    }

    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        int result = is_elf32 ? replace_functions_in_section32(e,
            scn, replacements): replace_functions_in_section64(e,
            scn, replacements);
        
        if(result == 0)
        {
            elf_update(e, ELF_C_WRITE);
            printf("Module content has been updated.\n");
        }
    }
    
    scn_versions = get_versions_section(e);
    if(scn_versions)
    {
        int result = is_elf32 ? replace_functions_versions32(e,
            scn_versions, replacements) : replace_functions_versions64(e,
            scn_versions, replacements);
        
        if(result == 0)
        {
            elf_update(e, ELF_C_WRITE);
            printf("Module content has been updated.\n");
        }
    }
    
    (void)elf_end(e);
    (void)close(fd);
    
    return 0;
}


const char crc_symbol_prefix[] = "__crc_";
#define crc_symbol_prefix_len (sizeof(crc_symbol_prefix) - 1)

/*
 * Fill crc of replacements according to section of kernel module.
 * 
 * Return first replacement, which crc is not filled.
 * Return NULL on error.
 *
 * If any crc-symbol is exist in this section, 'has_versions' will be set to non-zero.
 */
static struct replacement*
fill_replacements_crc_from_section32(Elf* e,
    Elf_Scn* scn, struct replacement* replacements, int *has_versions)
{
    struct replacement* repl_first = replacements;

    Elf_Data* data;
    
    Elf32_Sym *sym32, *sym32_end;
    Elf32_Shdr* shdr32 = elf32_getshdr(scn);
    CHECK_ELF_FUNCTION_RESULT(shdr32, getshdr);
    
    if(shdr32->sh_type != SHT_SYMTAB) return repl_first;
    
    data = elf_getdata(scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, getdata);
    
    sym32_end = (typeof(sym32_end))((char*)data->d_buf + data->d_size);

    if(repl_first->function_name == NULL)
    {
        return repl_first;//nothing to fill
    }
    
    for(sym32 = (typeof(sym32))data->d_buf; sym32 < sym32_end; sym32++)
    {
        const char* symbol_name;
        const char* symbol_name_without_prefix;
        struct replacement* repl;
        
        if(ELF32_ST_BIND(sym32->st_info) != STB_GLOBAL) continue;
        if(sym32->st_shndx != SHN_ABS) continue;
        
        symbol_name = elf_strptr(e, shdr32->sh_link, sym32->st_name);
        CHECK_ELF_FUNCTION_RESULT(symbol_name, strptr);
        
        if(strncmp(symbol_name, crc_symbol_prefix, crc_symbol_prefix_len))
            continue;
        
        *has_versions = 1;
        
        symbol_name_without_prefix = symbol_name + crc_symbol_prefix_len;
        for(repl = repl_first; repl->function_name != NULL; repl++)
        {
            if(repl->replacement_crc) continue;//already set
            if(strcmp(symbol_name_without_prefix, repl->replacement_name))
                continue;
            repl->replacement_crc = sym32->st_value;
            break;
        }
        if(repl == repl_first)
        {
            //shift repl_first
            for(++repl_first; repl_first->function_name != NULL; repl_first++)
            {
                if(repl_first->replacement_crc == 0) break;
            }
            if(repl_first->function_name == NULL)
            {
                break;//Full array is filled
            }
        }
    }
    
    return repl_first;
}


static struct replacement*
fill_replacements_crc_from_section64(Elf* e,
    Elf_Scn* scn, struct replacement* replacements, int* has_versions)
{
    struct replacement* repl_first = replacements;

    Elf_Data* data;
    
    Elf64_Sym *sym64, *sym64_end;
    Elf64_Shdr* shdr64 = elf64_getshdr(scn);
    CHECK_ELF_FUNCTION_RESULT(shdr64, getshdr);
    
    if(shdr64->sh_type != SHT_SYMTAB) return repl_first;
    
    data = elf_getdata(scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, getdata);
    
    sym64_end = (typeof(sym64_end))((char*)data->d_buf + data->d_size);

    if(repl_first->function_name == NULL)
    {
        return repl_first;//nothing to fill
    }
    
    for(sym64 = (typeof(sym64))data->d_buf; sym64 < sym64_end; sym64++)
    {
        const char* symbol_name;
        const char* symbol_name_without_prefix;
        struct replacement* repl;
        
        if(ELF64_ST_BIND(sym64->st_info) != STB_GLOBAL) continue;
        if(sym64->st_shndx != SHN_ABS) continue;
        
        symbol_name = elf_strptr(e, shdr64->sh_link, sym64->st_name);
        CHECK_ELF_FUNCTION_RESULT(symbol_name, strptr);
        
        if(strncmp(symbol_name, crc_symbol_prefix, crc_symbol_prefix_len))
            continue;
        
        *has_versions = 1;
        
        symbol_name_without_prefix = symbol_name + crc_symbol_prefix_len;
        for(repl = repl_first; repl->function_name != NULL; repl++)
        {
            if(repl->replacement_crc) continue;//already set
            if(strcmp(symbol_name_without_prefix, repl->replacement_name))
                continue;
            repl->replacement_crc = sym64->st_value;
            break;
        }
        if(repl == repl_first)
        {
            //shift repl_first
            for(++repl_first; repl_first->function_name != NULL; repl_first++)
            {
                if(repl_first->replacement_crc == 0) break;
            }
            if(repl_first->function_name == NULL)
            {
                break;//Full array is filled
            }
        }
    }
    
    return repl_first;
}


/*
 * Resolve relocation and return pointer to the allocated string,
 * pointed by this relocation.
 */
static char* resolve_relocated_str32(Elf* e,
    Elf_Scn* relocation_scn,
    int relocation_index,
    Elf32_Addr addr)
{
    char *str;
    
    Elf32_Shdr* relocation_shdr32 = elf32_getshdr(relocation_scn);
    CHECK_ELF_FUNCTION_RESULT(relocation_shdr32, getshdr);
    
    Elf_Data* relocation_data = elf_getdata(relocation_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(relocation_data, getdata);

    Elf_Scn* symbols_scn = elf_getscn(e, relocation_shdr32->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_scn, getscn);

    Elf_Data* symbols_data = elf_getdata(symbols_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(symbols_data, getdata);

    int symbol_index;
    
    if(relocation_shdr32->sh_type == SHT_RELA)
    {
        assert(relocation_index * sizeof(Elf32_Rela) < relocation_data->d_size);
        
        Elf32_Rela* relocation = (Elf32_Rela*)relocation_data->d_buf + relocation_index;

        symbol_index = ELF32_R_SYM(relocation->r_info);
        
        addr += relocation->r_addend;
    }
    else
    {
        assert(relocation_index * sizeof(Elf32_Rel) < relocation_data->d_size);
        
        Elf32_Rel* relocation = (Elf32_Rel*)relocation_data->d_buf + relocation_index;

        symbol_index = ELF32_R_SYM(relocation->r_info);
    }
    
    const Elf32_Sym* symbol = (Elf32_Sym*)symbols_data->d_buf + symbol_index;
    
    if(ELF32_ST_TYPE(symbol->st_info) != STT_SECTION)
    {
        printf("Unexpected type of relocation symbol for string(not a section)\n");
        return NULL;
    }
    
    Elf_Scn* strings_scn = elf_getscn(e, symbol->st_shndx);
    CHECK_ELF_FUNCTION_RESULT(strings_scn, getscn);
    
    Elf_Data* strings_data = elf_getdata(strings_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(strings_data, getdata);
    
    str = strdup((char*)strings_data->d_buf + addr);
    
    if(str == NULL)
    {
        printf("Failed to duplicate string.\n");
        return NULL;
    }
    
    return str;
}

static char* resolve_relocated_str64(Elf* e,
    Elf_Scn* relocation_scn,
    int relocation_index,
    Elf64_Addr addr)
{
    char *str;
    
    Elf64_Shdr* relocation_shdr64 = elf64_getshdr(relocation_scn);
    CHECK_ELF_FUNCTION_RESULT(relocation_shdr64, getshdr);
    
    Elf_Data* relocation_data = elf_getdata(relocation_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(relocation_data, getdata);

    Elf_Scn* symbols_scn = elf_getscn(e, relocation_shdr64->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_scn, getscn);

    Elf_Data* symbols_data = elf_getdata(symbols_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(symbols_data, getdata);

    int symbol_index;
    
    if(relocation_shdr64->sh_type == SHT_RELA)
    {
        assert(relocation_index * sizeof(Elf64_Rela) < relocation_data->d_size);
        
        Elf64_Rela* relocation = (Elf64_Rela*)relocation_data->d_buf + relocation_index;

        symbol_index = ELF64_R_SYM(relocation->r_info);
        
        addr += relocation->r_addend;
    }
    else
    {
        assert(relocation_index * sizeof(Elf64_Rel) < relocation_data->d_size);
        
        Elf64_Rel* relocation = (Elf64_Rel*)relocation_data->d_buf + relocation_index;

        symbol_index = ELF64_R_SYM(relocation->r_info);
    }
    
    const Elf64_Sym* symbol = (Elf64_Sym*)symbols_data->d_buf + symbol_index;
    
    if(ELF64_ST_TYPE(symbol->st_info) != STT_SECTION)
    {
        printf("Unexpected type of relocation symbol for string(not a section)\n");
        return NULL;
    }
    
    Elf_Scn* strings_scn = elf_getscn(e, symbol->st_shndx);
    CHECK_ELF_FUNCTION_RESULT(strings_scn, getscn);
    
    Elf_Data* strings_data = elf_getdata(strings_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(strings_data, getdata);
    
    str = strdup((char*)strings_data->d_buf + addr);
    
    if(str == NULL)
    {
        printf("Failed to duplicate string.\n");
        return NULL;
    }
    
    return str;
}


/*
 * Create replacements array according to given replacements descriptions
 * and corresponded relocations.
 */
static struct replacement* replacements_create32(Elf* e_wrapper,
    const struct replacement_desc32* replacements_desc32,
    int n_replacements,
    const struct replacement_desc32* replacements_desc32_rel,
    Elf_Scn* relocation_scn)
{
    struct replacement* replacements = malloc(
        (n_replacements + 1) * sizeof(*replacements));
    if(replacements == NULL)
    {
        printf("Failed to allocate replacements array.\n");
        return NULL;
    }

    int i;
    for(i = 0; i < n_replacements; i++)
    {
        int relocation_index = replacements_desc32_rel[i].function_name;
        
        replacements[i].function_name = resolve_relocated_str32(
            e_wrapper,
            relocation_scn,
            relocation_index,
            replacements_desc32[i].function_name);
        if(replacements[i].function_name == NULL)
        {
            printf("Failed to resolve function name.\n");
            goto err;
        }

        relocation_index = replacements_desc32_rel[i].replacement_name;
        
        replacements[i].replacement_name = resolve_relocated_str32(
            e_wrapper,
            relocation_scn,
            relocation_index,
            replacements_desc32[i].replacement_name);
        if(replacements[i].replacement_name == NULL)
        {
            printf("Failed to resolve replacement name.\n");
            free(replacements[i].function_name);
            goto err;
        }
    }
    
    replacements[n_replacements].function_name = NULL;
    
    return replacements;

err:
    for(--i; i >= 0; i--)
    {
        free(replacements[i].function_name);
        free(replacements[i].replacement_name);
    }
    
    free(replacements);
    
    return NULL;
}

static struct replacement* replacements_create64(Elf* e_wrapper,
    const struct replacement_desc64* replacements_desc64,
    int n_replacements,
    const struct replacement_desc64* replacements_desc64_rel,
    Elf_Scn* relocation_scn)
{
    struct replacement* replacements = malloc(
        (n_replacements + 1) * sizeof(*replacements));
    if(replacements == NULL)
    {
        printf("Failed to allocate replacements array.\n");
        return NULL;
    }

    int i;
    for(i = 0; i < n_replacements; i++)
    {
        int relocation_index = replacements_desc64_rel[i].function_name;
        
        replacements[i].function_name = resolve_relocated_str64(
            e_wrapper,
            relocation_scn,
            relocation_index,
            replacements_desc64[i].function_name);
        if(replacements[i].function_name == NULL)
        {
            printf("Failed to resolve function name.\n");
            goto err;
        }

        relocation_index = replacements_desc64_rel[i].replacement_name;
        
        replacements[i].replacement_name = resolve_relocated_str64(
            e_wrapper,
            relocation_scn,
            relocation_index,
            replacements_desc64[i].replacement_name);
        if(replacements[i].replacement_name == NULL)
        {
            printf("Failed to resolve replacement name.\n");
            free(replacements[i].function_name);
            goto err;
        }
    }
    
    replacements[n_replacements].function_name = NULL;
    
    return replacements;

err:
    for(--i; i >= 0; i--)
    {
        free(replacements[i].function_name);
        free(replacements[i].replacement_name);
    }
    
    free(replacements);
    
    return NULL;
}


/*
 * Return allocated array of replacements.
 * On error, return NULL.
 * 
 * crc field is not filled at this stage.
 */
static struct replacement* get_replacements32(Elf* e_wrapper)
{
    struct replacement* replacements;
    /*
     * Array of replacement descriptions, contained in the file.
     */
    const struct replacement_desc32* replacements_desc32;
    /*
     * Array of structures, contained index of relocation instead of
     * corresponded strings.
     * 
     * This array may be superimpose on array of replacement descriptions
     * for get real pointers to strings.
     * 
     * Usual content of this array will be {{0,1},{2,3},{4,5}...}.
     */
    struct replacement_desc32* replacements_desc32_rel;
    int n_replacements;
    
    Elf_Scn* replacement_scn = get_named_section(e_wrapper, REPLACEMENT_SECTION_NAME);
    if(replacement_scn == NULL)
    {
        printf("Cannot find replacement section in the wrapper module.\n");
        return NULL;
    }
    
    Elf_Data* replacement_data = elf_getdata(replacement_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(replacement_data, getdata);
    
    /*
     * Assume that alignment of replacement_desc structure is same
     * as its size.
     */
    if(replacement_data->d_size % sizeof(*replacements_desc32))
    {
        printf("Unexpected size of replacemen section "
            "(not a multiply of replacement description size).\n");
        return NULL;
    }
    
    replacements_desc32 = replacement_data->d_buf;
    n_replacements = replacement_data->d_size / sizeof(*replacements_desc32);
    
    Elf_Scn* relocation_scn = get_relocation_section(e_wrapper, replacement_scn);
    if(relocation_scn == NULL)
    {
        printf("Cannot find relocation section for one containing replacements.\n");
        return NULL;
    }
    
    Elf32_Shdr* relocation_shdr32 = elf32_getshdr(relocation_scn);
    CHECK_ELF_FUNCTION_RESULT(relocation_shdr32, getshdr);
    
    Elf_Data* relocation_data = elf_getdata(relocation_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(relocation_data, getdata);
    
    // Allocate and fill relocations array for replacements
    replacements_desc32_rel = malloc(n_replacements * sizeof(*replacements_desc32_rel));
    if(replacements_desc32_rel == NULL)
    {
        printf("Failed to allocate array for relocations of replacements.\n");
        return NULL;
    }
    memset(replacements_desc32_rel, -1, n_replacements * sizeof(*replacements_desc32_rel));
    
    if(relocation_shdr32->sh_type == SHT_RELA)
    {
        const Elf32_Rela *relocations = relocation_data->d_buf;
        int n_relocations = relocation_data->d_size / sizeof(*relocations);

        int i;
        for(i = 0; i < n_relocations; i++)
        {
            void* rel_addr = (char*)replacements_desc32_rel + relocations[i].r_offset;
            *((Elf32_Addr*)rel_addr) = i;
        }
    }
    else
    {
        const Elf32_Rel *relocations = relocation_data->d_buf;
        int n_relocations = relocation_data->d_size / sizeof(*relocations);

        int i;
        for(i = 0; i < n_relocations; i++)
        {
            void* rel_addr = (char*)replacements_desc32_rel + relocations[i].r_offset;
            *((Elf32_Addr*)rel_addr) = i;
        }
    }
    // Create replacements array
    replacements = replacements_create32(e_wrapper,
        replacements_desc32,
        n_replacements,
        replacements_desc32_rel,
        relocation_scn);
        
    free(replacements_desc32_rel);
    
    if(replacements == NULL)
    {
        printf("Failed to create replacements array.\n");
        
        return NULL;
    }

    return replacements;
}

static struct replacement* get_replacements64(Elf* e_wrapper)
{
    struct replacement* replacements;
    /*
     * Array of replacement descriptions, contained in the file.
     */
    const struct replacement_desc64* replacements_desc64;
    /*
     * Array of structures, contained index of relocation instead of
     * corresponded strings.
     * 
     * This array may be superimpose on array of replacement descriptions
     * for get real pointers to strings.
     * 
     * Usual content of this array will be {{0,1},{2,3},{4,5}...}.
     */
    struct replacement_desc64* replacements_desc64_rel;
    int n_replacements;
    
    Elf_Scn* replacement_scn = get_named_section(e_wrapper, REPLACEMENT_SECTION_NAME);
    if(replacement_scn == NULL)
    {
        printf("Cannot find replacement section in the wrapper module.\n");
        return NULL;
    }
    
    Elf_Data* replacement_data = elf_getdata(replacement_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(replacement_data, getdata);
    
    /*
     * Assume that alignment of replacement_desc structure is same
     * as its size.
     */
    if(replacement_data->d_size % sizeof(*replacements_desc64))
    {
        printf("Unexpected size of replacemen section "
            "(not a multiply of replacement description size).\n");
        return NULL;
    }
    
    replacements_desc64 = replacement_data->d_buf;
    n_replacements = replacement_data->d_size / sizeof(*replacements_desc64);
    
    Elf_Scn* relocation_scn = get_relocation_section(e_wrapper, replacement_scn);
    if(relocation_scn == NULL)
    {
        printf("Cannot find relocation section for one containing replacements.\n");
        return NULL;
    }
    
    Elf64_Shdr* relocation_shdr64 = elf64_getshdr(relocation_scn);
    CHECK_ELF_FUNCTION_RESULT(relocation_shdr64, getshdr);
    
    Elf_Data* relocation_data = elf_getdata(relocation_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(relocation_data, getdata);
    
    // Allocate and fill relocations array for replacements
    replacements_desc64_rel = malloc(n_replacements * sizeof(*replacements_desc64_rel));
    if(replacements_desc64_rel == NULL)
    {
        printf("Failed to allocate array for relocations of replacements.\n");
        return NULL;
    }
    memset(replacements_desc64_rel, -1, n_replacements * sizeof(*replacements_desc64_rel));
    
    if(relocation_shdr64->sh_type == SHT_RELA)
    {
        const Elf64_Rela *relocations = relocation_data->d_buf;
        int n_relocations = relocation_data->d_size / sizeof(*relocations);

        int i;
        for(i = 0; i < n_relocations; i++)
        {
            void* rel_addr = (char*)replacements_desc64_rel + relocations[i].r_offset;
            *((Elf64_Addr*)rel_addr) = i;
        }
    }
    else
    {
        const Elf64_Rel *relocations = relocation_data->d_buf;
        int n_relocations = relocation_data->d_size / sizeof(*relocations);

        int i;
        for(i = 0; i < n_relocations; i++)
        {
            void* rel_addr = (char*)replacements_desc64_rel + relocations[i].r_offset;
            *((Elf64_Addr*)rel_addr) = i;
        }
    }
    // Create replacements array
    replacements = replacements_create64(e_wrapper,
        replacements_desc64,
        n_replacements,
        replacements_desc64_rel,
        relocation_scn);
        
    free(replacements_desc64_rel);
    
    if(replacements == NULL)
    {
        printf("Failed to create replacements array.\n");
        
        return NULL;
    }

    return replacements;
}

/*
 * Return array of replacements contained in the given wrapper module.
 * 
 * Return NULL on error.
 *
 * If 'replacement_crc' fields is filled, set 'has_versions' to non-zero.
 * 
 */
static struct replacement* get_replacements_from_wrapper(
    const char* wrapper_filename, int* has_versions)
{
    struct replacement* replacements;
    
    int fd;
    
    Elf* e;
    int is_elf32;
    
    int _has_versions;
    
    if (elf_version(EV_CURRENT) == EV_NONE )
        errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));
    if((fd = open(wrapper_filename, O_RDONLY, 0)) < 0)
        err(EX_NOINPUT, "open \"%s\" failed", wrapper_filename);

    e = elf_begin(fd, ELF_C_READ, NULL);
    if(!e)
        errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));
    
    if(elf_kind(e) != ELF_K_ELF)
        errx(EX_DATAERR, "Input file should be in ELF format");
    
    is_elf32 = elf_is32(e);

    replacements = is_elf32 ? get_replacements32(e) : get_replacements64(e);
    if(replacements == NULL)
    {   
        (void)elf_end(e);
        (void)close(fd);
        return NULL;
    }

    struct replacement* repl_first = replacements;
    if(repl_first->function_name == NULL) goto out;// Full array is already filled

    Elf_Scn* scn;
    _has_versions = 0;
    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        repl_first = is_elf32
            ? fill_replacements_crc_from_section32(e, scn, repl_first,
                &_has_versions)
            : fill_replacements_crc_from_section64(e, scn, repl_first,
                &_has_versions);
        
        if(repl_first == NULL)
        {
            errx(EX_DATAERR, "Error occure while extract crc for functions.");
        }
        if(repl_first->function_name == NULL) break;
    }
    
out:    

    (void)elf_end(e);
    (void)close(fd);
    
    if(_has_versions && (repl_first->function_name != NULL))
    {
        printf("Cannot find crc for function %s.", repl_first->replacement_name);
        replacements_destroy(replacements);
        return NULL;
    }

    *has_versions = _has_versions;
    return replacements;
}

static int parse_parameters(int argc, char** argv,
    const char** filename,
    const char** wrapper_filename)
{
    if (argc != 3)
        errx(EX_USAGE, "Usage: %s <module_name> <wrapper_module_name>", argv[0]);
    *filename = argv[1];
    *wrapper_filename = argv[2];
    
    return 0;
}

int
main(int argc, char** argv)
{
    const char* filename;
    const char* wrapper_filename;
    
    struct replacement* replacements;
    
    int has_versions;
    
    int result;

    result = parse_parameters(argc, argv, &filename, &wrapper_filename);
    
    if(result) return result;
    
    replacements = get_replacements_from_wrapper(wrapper_filename, &has_versions);
    
    if(replacements == NULL) return 0;

    replacements_print(replacements);

    result = replace_functions_in_module(filename, replacements, has_versions);
    replacements_destroy(replacements);
    
    if(result) return result;
    
    return 0;
}
