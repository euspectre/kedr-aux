#include <err.h>
#include <fcntl.h>

#include <gelf.h>
#include <libelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <vis.h>

#include <string.h>

#define CHECK_ELF_FUNCTION_RESULT(cond, function) \
    while(!(cond)) {errx(EX_SOFTWARE, "%s() failed: %s.", #function, elf_errmsg(-1)); }


/*
 * Record about symbol version.
 */
struct symVersion
{
    int32_t crc;
    char name[0];
};

#define align_val(val, alignment) (((val) + ((alignment) - 1)) / (alignment) * (alignment))

/*
 * Return next record about symbol version.
 */
static struct symVersion* symVersion_next(struct symVersion* symVer)
{
    int size = sizeof(symVer->crc) + strlen(symVer->name) + 1;
    
    return (struct symVersion*)((char*)symVer + align_val(size, 64));
}

/*
 * Description of one function for replace.
 */
struct replacement
{
    const char* function_name;
    const char* replacement_name;
    int32_t replacement_crc;
};

// Hardcoded replacements(without crc)
static struct replacement replacements_hardcoded[] =
{
    {"__kmalloc", "__kMalloc", 0},
    {"kfree", "kFree", 0},
    {NULL,}
};

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
static int replace_functions_in_section(Elf* e, Elf_Scn* scn,
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
    
    sym32_end = (typeof(sym32_end))(data->d_buf + data->d_size);
    
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
                symbol_name, repl->replacement_name);
            is_replaced = 1;
        }
    }
    
    if(is_replaced) elf_flagdata(data, ELF_C_SET, ELF_F_DIRTY);
    return is_replaced ? 0 : 1;
}
/*
 * Return identificator for section containing symbol versions in
 * kernel module.
 * 
 * If such section is absent, return NULL.
 */
static Elf_Scn* get_versions_section(Elf* e)
{
    Elf_Scn* scn;
    int result;
    size_t shdrstrndx;
   
    result = elf_getshdrstrndx(e, &shdrstrndx);
    CHECK_ELF_FUNCTION_RESULT(result == 0, getshdrstrndx);

    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        Elf32_Shdr* shdr32 = elf32_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr32, getshdr);

        if(shdr32->sh_type == SHT_PROGBITS)
        {
            const char* section_name = elf_strptr(e, shdrstrndx,
                shdr32->sh_name);
            CHECK_ELF_FUNCTION_RESULT(section_name, strptr);

            if(strcmp(section_name, "__versions") == 0) return scn;
        }
    }
   
    return NULL;
}

/*
 * Look for section with symbol versions and replace needed functions
 * and its versions with corresponded ones.
 * 
 * Return 0 if at least one function was replaced, 1 otherwise.
 */
static int replace_functions_versions(Elf* e, Elf_Scn* scn_versions,
    const struct replacement* replacements)
{
    Elf_Data* data;
    struct symVersion* symVer, *symVer_end;
    int is_replaced = 0;
    
    data = elf_getdata(scn_versions, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, getdata);
    
    symVer_end = (typeof(symVer_end))(data->d_buf + data->d_size);
        
    for(symVer = (typeof(symVer))data->d_buf;
        symVer < symVer_end;
        symVer = symVersion_next(symVer))
    {
        //printf("%.8x %s\n", symVer->crc, symVer->name);
        const struct replacement* repl = replacement_find(
            symVer->name,
            replacements);
        if(repl)
        {
            symVer->crc = repl->replacement_crc;
            strcpy(symVer->name, repl->replacement_name);
            printf("Versioning function %s has been replaced with (%8x,%s).\n",
                repl->function_name, repl->replacement_crc, repl->replacement_name);
            is_replaced = 1;
        }
    }
    
    if(is_replaced) elf_flagdata(data, ELF_C_SET, ELF_F_DIRTY);
    return is_replaced ? 0 : 1;
}

static int replace_functions_in_module(const char* module_filename,
    const struct replacement* replacements)
{
    int fd;
    
    Elf* e;
    char* ident;
    
    Elf_Scn* scn;
    Elf_Scn* scn_versions;
    
    int is_elf32;

    Elf32_Ehdr* ehdr32;

    if (elf_version(EV_CURRENT) == EV_NONE )
        errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));
    if((fd = open(module_filename, O_RDWR, 0)) < 0)
        err(EX_NOINPUT, "open \"%s\" failed", module_filename);

    e = elf_begin(fd, ELF_C_RDWR, NULL);
    if(!e)
        errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));
    
    if(elf_kind(e) != ELF_K_ELF)
        errx(EX_DATAERR, "Input file should be in ELF format");
    
    ident = elf_getident(e, NULL);
    CHECK_ELF_FUNCTION_RESULT(ident, getident);
    
    switch(ident[EI_CLASS])
    {
    case ELFCLASS32:
        is_elf32 = 1;
    break;
    case ELFCLASS64:
        is_elf32 = 0;
    break;
    default:
        errx(EX_SOFTWARE, "Invalid Elf class.");
    }
    
    if(!is_elf32)
        errx(EX_DATAERR, "Only 32-bit ELF format is currently supported.");

    ehdr32 = elf32_getehdr(e);
    CHECK_ELF_FUNCTION_RESULT(ehdr32, getehdr);
    
    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        if(replace_functions_in_section(e, scn, replacements) == 0)
        {
            elf_update(e, ELF_C_WRITE);
            printf("Module content has been updated.\n");
        }
    }
    
    
    scn_versions = get_versions_section(e);
    if(scn_versions)
    {
        if(replace_functions_versions(e, scn_versions, replacements) == 0)
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
 */
static struct replacement*
fill_replacements_crc_from_section(Elf* e,
    Elf_Scn* scn, struct replacement* replacements)
{
    struct replacement* repl_first = replacements;

    Elf_Data* data;
    
    Elf32_Sym *sym32, *sym32_end;
    Elf32_Shdr* shdr32 = elf32_getshdr(scn);
    CHECK_ELF_FUNCTION_RESULT(shdr32, getshdr);
    
    if(shdr32->sh_type != SHT_SYMTAB) return repl_first;
    
    data = elf_getdata(scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, getdata);
    
    sym32_end = (typeof(sym32_end))(data->d_buf + data->d_size);

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

/*
 * Fill 'replacement_crc' fields in array of replacements.
 */
static int fill_replacements_crc_from_module(const char* wrapper_filename,
    struct replacement* replacements)
{
    int fd;
    
    Elf* e;
    char* ident;
    
    Elf_Scn* scn;
    Elf_Scn* scn_versions;
    
    int is_elf32;

    Elf32_Ehdr* ehdr32;
    
    struct replacement* repl_first = replacements;
    if(repl_first->function_name == NULL) return 0;// Full array is already filled

    if (elf_version(EV_CURRENT) == EV_NONE )
        errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));
    if((fd = open(wrapper_filename, O_RDONLY, 0)) < 0)
        err(EX_NOINPUT, "open \"%s\" failed", wrapper_filename);

    e = elf_begin(fd, ELF_C_READ, NULL);
    if(!e)
        errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));
    
    if(elf_kind(e) != ELF_K_ELF)
        errx(EX_DATAERR, "Input file should be in ELF format");
    
    ident = elf_getident(e, NULL);
    CHECK_ELF_FUNCTION_RESULT(ident, getident);
    
    switch(ident[EI_CLASS])
    {
    case ELFCLASS32:
        is_elf32 = 1;
    break;
    case ELFCLASS64:
        is_elf32 = 0;
    break;
    default:
        errx(EX_SOFTWARE, "Invalid Elf class.");
    }
    
    if(!is_elf32)
        errx(EX_DATAERR, "Only 32-bit ELF format is currently supported.");

    ehdr32 = elf32_getehdr(e);
    CHECK_ELF_FUNCTION_RESULT(ehdr32, getehdr);
    
    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        struct replacement* repl;
        
        repl_first = fill_replacements_crc_from_section(e, scn,
            repl_first);
        
        if(repl_first == NULL)
        {
            errx(EX_DATAERR, "Error occure while extract crc for functions.");
        }
        if(repl_first->function_name == NULL) break;
    }
    
    
    (void)elf_end(e);
    (void)close(fd);
    
    if(repl_first->function_name != NULL)
    {
        errx(EX_DATAERR, "Cannot find crc for function %s.",
            repl_first->replacement_name);
    }

    return 0;
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
    
    struct replacement* replacements = replacements_hardcoded;
    
    int result;

    result = parse_parameters(argc, argv, &filename, &wrapper_filename);
    
    if(result) return result;
    
    result = fill_replacements_crc_from_module(wrapper_filename, replacements);
    
    if(result) return result;

    result = replace_functions_in_module(filename, replacements);
    
    if(result) return result;
    
    return 0;
}
