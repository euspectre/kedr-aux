#include "binary_instrument_utilities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include <err.h>
#include <assert.h>
#include <errno.h>

#define SYMBOLS_SCN_NAME ".symtab"

#define array_element_offset(index, elem_type) ((elem_type*)0 +  (index) - (elem_type*)0)

void* utils_elf_get_data_at(Elf_Scn* scn, size_t offset)
{
    Elf_Data* data;
    for(data = elf_getdata(scn, NULL); data != NULL; data = elf_getdata(scn, data))
    {
        if(offset >= (data->d_off + data->d_size)) continue;
        return (char*)data->d_buf + offset - data->d_off;
    }
    
    printf("Data offset out of section.\n");
    return NULL;
}

char* utils_elf_strptr(Elf* e, size_t index, size_t offset)
{
    Elf_Scn* scn = elf_getscn(e, index);
    CHECK_ELF_FUNCTION_RESULT(scn, elf_getscn);
    
    return utils_elf_get_data_at(scn, offset);
}

int utils_elf_is32(Elf* e)
{
    char* ident;

    assert(elf_kind(e) == ELF_K_ELF);
    
    ident = elf_getident(e, NULL);
    CHECK_ELF_FUNCTION_RESULT(ident, elf_getident);
    
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

Elf_Scn* utils_elf_find_section(Elf* e, const char* section_name)
{
    Elf_Scn* scn;
    int result;
    size_t shdrstrndx;
    
    int is_elf32 = utils_elf_is32(e);
   
    result = elf_getshdrstrndx(e, &shdrstrndx);
    CHECK_ELF_FUNCTION_RESULT(result == 0, elf_getshdrstrndx);

    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        size_t section_name_index;
        
        if(is_elf32)
        {
            Elf32_Shdr* shdr32 = elf32_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);

            section_name_index = shdr32->sh_name;
        }
        else
        {
            Elf64_Shdr* shdr64 = elf64_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);

            section_name_index = shdr64->sh_name;
        }

        const char* _section_name = elf_strptr(e, shdrstrndx,
            section_name_index);
        CHECK_ELF_FUNCTION_RESULT(_section_name, elf_strptr);

        if(strcmp(_section_name, section_name) == 0) return scn;
    }
   
    return NULL;
}

Elf_Scn* utils_elf_get_symbols_section(Elf* e)
{
    Elf_Scn* scn;
    int result;
    size_t shdrstrndx;
    
    int is_elf32 = utils_elf_is32(e);
   
    result = elf_getshdrstrndx(e, &shdrstrndx);
    CHECK_ELF_FUNCTION_RESULT(result == 0, elf_getshdrstrndx);

    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        size_t section_name_index;
        
        if(is_elf32)
        {
            Elf32_Shdr* shdr32 = elf32_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);

            if(shdr32->sh_type != SHT_SYMTAB) continue;
            section_name_index = shdr32->sh_name;
        }
        else
        {
            Elf64_Shdr* shdr64 = elf64_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);

            if(shdr64->sh_type != SHT_SYMTAB) continue;
            section_name_index = shdr64->sh_name;
        }

        char* section_name = elf_strptr(e, shdrstrndx, section_name_index);
        CHECK_ELF_FUNCTION_RESULT(section_name, elf_strptr);
        
        assert(strcmp(section_name, SYMBOLS_SCN_NAME) == 0);
        return scn;
    }
    
    printf("Symbols section is not found.\n");
    
    return NULL;
}

static Elf_Scn* get_versions_section(Elf* e)
{
    return utils_elf_find_section(e, "__versions");
}

Elf_Scn* utils_elf_find_relocation_section(Elf* e, int section_index)
{
    Elf_Scn* scn;

    int is_elf32 = utils_elf_is32(e);

    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        if(is_elf32)
        {
            Elf32_Shdr* shdr32 = elf32_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr32, getshdr);

            if((shdr32->sh_type == SHT_REL) || (shdr32->sh_type == SHT_RELA))
            {
                if(shdr32->sh_info == section_index) return scn;
            }
        }
        else
        {
            Elf64_Shdr* shdr64 = elf64_getshdr(scn);
            CHECK_ELF_FUNCTION_RESULT(shdr64, getshdr);

            if((shdr64->sh_type == SHT_REL) || (shdr64->sh_type == SHT_RELA))
            {
                if(shdr64->sh_info == section_index) return scn;
            }
        }

    }
   
    return NULL;
}

Elf32_Sym* utils_elf32_find_symbol(Elf* e, const char* symbol_name,
    size_t* index)
{
    Elf_Scn* scn = utils_elf_get_symbols_section(e);
    assert(scn);
    
    Elf32_Shdr* shdr32 = elf32_getshdr(scn);
    CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);
    
    size_t names_index = shdr32->sh_link;
    
    Elf_Data* data;
    for(data = elf_getdata(scn, NULL); data != NULL; data = elf_getdata(scn, data))
    {
        Elf32_Sym* sym32;
        Elf32_Sym* sym32_end = data->d_buf + data->d_size;
        
        for(sym32 = data->d_buf; sym32 < sym32_end; sym32++)
        {
            const char* symbol_name_current = elf_strptr(e, names_index, sym32->st_name);
            CHECK_ELF_FUNCTION_RESULT(symbol_name_current, elf_strptr);
            
            if(strcmp(symbol_name_current, symbol_name) == 0)
            {
                if(index)
                {
                    *index = sym32 - (Elf32_Sym*)
                        ((char*)data->d_buf - data->d_off);
                }
                return sym32;
            }
        }
    }
    
    return NULL;
}

Elf64_Sym* utils_elf64_find_symbol(Elf* e, const char* symbol_name,
    size_t* index)
{
    Elf_Scn* scn = utils_elf_get_symbols_section(e);
    assert(scn);
    
    Elf64_Shdr* shdr64 = elf64_getshdr(scn);
    CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);
    
    size_t names_index = shdr64->sh_link;
    
    Elf_Data* data;
    for(data = elf_getdata(scn, NULL); data != NULL; data = elf_getdata(scn, data))
    {
        Elf64_Sym* sym64;
        Elf64_Sym* sym64_end = data->d_buf + data->d_size;
        
        for(sym64 = data->d_buf; sym64 < sym64_end; sym64++)
        {
            const char* symbol_name_current = elf_strptr(e, names_index, sym64->st_shndx);
            CHECK_ELF_FUNCTION_RESULT(symbol_name_current, elf_strptr);
            
            if(strcmp(symbol_name_current, symbol_name) == 0)
            {
                if(index)
                {
                    *index = sym64 - (Elf64_Sym*)
                        ((char*)data->d_buf - data->d_off);
                }
                return sym64;
            }
        }
    }
    
    return NULL;
}


static const char crc_symbol_prefix[] = "__crc_";
#define crc_symbol_prefix_len (sizeof(crc_symbol_prefix) - 1)

int utils_elf_has_symbol_versions(Elf* e)
{
    return utils_elf_find_section(e, "__kcrctab") != NULL;
}


int32_t utils_elf32_get_symbol_version(Elf* e, const char* symbol_name)
{
    Elf_Scn* scn;
    
    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        Elf32_Shdr* shdr32 = elf32_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);

        if(shdr32->sh_type != SHT_SYMTAB) continue;

        Elf_Data* data = elf_getdata(scn, NULL);
        CHECK_ELF_FUNCTION_RESULT(data, elf_getdata);

        Elf32_Sym *sym32, *sym32_end;
        
        sym32_end = (typeof(sym32_end))((char*)data->d_buf + data->d_size);

        for(sym32 = (typeof(sym32))data->d_buf; sym32 < sym32_end; sym32++)
        {
            const char* _symbol_name;
            
            if(ELF32_ST_BIND(sym32->st_info) != STB_GLOBAL) continue;
            if(sym32->st_shndx != SHN_ABS) continue;
            
            _symbol_name = elf_strptr(e, shdr32->sh_link, sym32->st_name);
            CHECK_ELF_FUNCTION_RESULT(_symbol_name, elf_strptr);
            
            if(strncmp(symbol_name, crc_symbol_prefix, crc_symbol_prefix_len))
                continue;
            
            if(strcmp(_symbol_name + crc_symbol_prefix_len, symbol_name) == 0)
                return sym32->st_value;
        }
    }
    // Shouldn't be reachable
    assert(0);
    
    return 0;
}


int64_t utils_elf64_get_symbol_version(Elf* e, const char* symbol)
{
    Elf_Scn* scn;
    
    for(scn = elf_nextscn(e, NULL); scn != NULL; scn = elf_nextscn(e, scn))
    {
        Elf64_Shdr* shdr64 = elf64_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);

        if(shdr64->sh_type != SHT_SYMTAB) continue;

        Elf_Data* data = elf_getdata(scn, NULL);
        CHECK_ELF_FUNCTION_RESULT(data, elf_getdata);

        Elf64_Sym *sym64, *sym64_end;
        
        sym64_end = (typeof(sym64_end))((char*)data->d_buf + data->d_size);

        for(sym64 = (typeof(sym64))data->d_buf; sym64 < sym64_end; sym64++)
        {
            const char* symbol_name;
            
            if(ELF64_ST_BIND(sym64->st_info) != STB_GLOBAL) continue;
            if(sym64->st_shndx != SHN_ABS) continue;
            
            symbol_name = elf_strptr(e, shdr64->sh_link, sym64->st_name);
            CHECK_ELF_FUNCTION_RESULT(symbol_name, elf_strptr);
            
            if(strncmp(symbol_name, crc_symbol_prefix, crc_symbol_prefix_len))
                continue;
            
            if(strcmp(symbol_name + crc_symbol_prefix_len, symbol) == 0)
                return sym64->st_value;
        }
    }
    // Shouldn't be reachable
    assert(0);
    
    return 0;
}


const char* versions_section_name = "__versions";

/*
 * Record about symbol version in ELF-file.
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
 * Whether crc-hash is required for imported symbols.
 */
int utils_elf_require_symbol_versions(Elf* e)
{
    return utils_elf_find_section(e, versions_section_name) != NULL;
}

int32_t utils_elf32_get_symbol_required_version(Elf* e,
    const char* symbol_name)
{
    Elf_Scn* scn = get_versions_section(e);
    assert(scn);

    Elf_Data* data;
    for(data = elf_getdata(scn, NULL); data != NULL; data = elf_getdata(scn, data))
    {
        struct symVersion32* symVer32;
        struct symVersion32* symVer32_end = (struct symVersion32*)
            ((char*)(data->d_buf) + data->d_size);
        for(symVer32 = data->d_buf;
            symVer32 < symVer32_end;
            symVer32 = symVersion32_next(symVer32))
        {
            if(strcmp(symVer32->name, symbol_name) == 0)
                return symVer32->crc;
        }
    }
    assert(0);
    return 0;
}

int64_t utils_elf64_get_symbol_required_version(Elf* e,
    const char* symbol_name)
{
    Elf_Scn* scn = get_versions_section(e);
    assert(scn);

    Elf_Data* data;
    for(data = elf_getdata(scn, NULL); data != NULL; data = elf_getdata(scn, data))
    {
        struct symVersion64* symVer64;
        struct symVersion64* symVer64_end = (struct symVersion64*)
            ((char*)(data->d_buf) + data->d_size);
        for(symVer64 = data->d_buf;
            symVer64 < symVer64_end;
            symVer64 = symVersion64_next(symVer64))
        {
            if(strcmp(symVer64->name, symbol_name) == 0)
                return symVer64->crc;
        }
    }
    assert(0);
    return 0;
}


/*
 * Resolve relocation and return pointer to the allocated string,
 * pointed by this relocation.
 */
char* resolve_relocated_str32(Elf* e,
    Elf_Scn* relocation_scn,
    int relocation_index,
    Elf32_Addr addr)
{
    char *str;
    
    Elf32_Shdr* shdr32 = elf32_getshdr(relocation_scn);
    CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);
    
    Elf_Scn* symbols_scn = elf_getscn(e, shdr32->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_scn, elf_getscn);

    int symbol_index;
    
    if(shdr32->sh_type == SHT_RELA)
    {
        Elf32_Rela* relocation = utils_elf_get_data_at(relocation_scn,
            array_element_offset(relocation_index, Elf32_Rela));
        assert(relocation);
        
        symbol_index = ELF32_R_SYM(relocation->r_info);
        addr += relocation->r_addend;
    }
    else
    {
        Elf32_Rel* relocation = utils_elf_get_data_at(relocation_scn,
            array_element_offset(relocation_index, Elf32_Rel));
        assert(relocation);
        
        symbol_index = ELF32_R_SYM(relocation->r_info);
    }
    
    const Elf32_Sym* symbol = utils_elf_get_data_at(symbols_scn,
        array_element_offset(symbol_index, Elf32_Sym));
    
    if(ELF32_ST_TYPE(symbol->st_info) != STT_SECTION)
    {
        printf("Unexpected type of relocation symbol for string(not a section)\n");
        return NULL;
    }
    
    str = strdup(utils_elf_strptr(e, symbol->st_shndx, addr));
    
    if(str == NULL)
    {
        printf("Failed to duplicate string.\n");
        return NULL;
    }
    
    return str;
}

char* resolve_relocated_str64(Elf* e,
    Elf_Scn* relocation_scn,
    int relocation_index,
    Elf64_Addr addr)
{
    char *str;
    
    Elf64_Shdr* shdr64 = elf64_getshdr(relocation_scn);
    CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);
    
    Elf_Scn* symbols_scn = elf_getscn(e, shdr64->sh_link);
    CHECK_ELF_FUNCTION_RESULT(symbols_scn, elf_getscn);

    int symbol_index;
    
    if(shdr64->sh_type == SHT_RELA)
    {
        Elf64_Rela* relocation = utils_elf_get_data_at(relocation_scn,
            array_element_offset(relocation_index, Elf64_Rela));
        assert(relocation);

        symbol_index = ELF64_R_SYM(relocation->r_info);
        addr += relocation->r_addend;
    }
    else
    {
        Elf64_Rel* relocation = utils_elf_get_data_at(relocation_scn,
            array_element_offset(relocation_index, Elf64_Rel));
        assert(relocation);

        symbol_index = ELF64_R_SYM(relocation->r_info);
    }
    
    const Elf64_Sym* symbol = utils_elf_get_data_at(symbols_scn,
        array_element_offset(symbol_index, Elf64_Sym));
    assert(symbol);
    
    if(ELF64_ST_TYPE(symbol->st_info) != STT_SECTION)
    {
        printf("Unexpected type of relocation symbol for string(not a section)\n");
        return NULL;
    }
    
    str = strdup(utils_elf_strptr(e, symbol->st_shndx, addr));
    
    if(str == NULL)
    {
        printf("Failed to duplicate name of the symbol.\n");
        return NULL;
    }
    
    return str;
}

int utils_elf_add_string(ElfWriter* ew, Elf_Scn* scn,
    const char* str, size_t* offset)
{
    size_t size = strlen(str) + 1;
    char* str_allocated = elf_writer_extend_section(ew, scn,
        size, 1, offset);
    if(str_allocated == NULL)
    {
        printf("Failed to add string to the section.\n");
        return -ENOMEM;
    }
    
    memcpy(str_allocated, str, size);
    //printf("String '%s' was added to the ELF.\n", str_allocated);
    
    return 0;
    
}

Elf32_Sym* utils_elf32_add_symbol(ElfWriter* ew,
    unsigned char type, unsigned char bind, size_t* index)
{
    Elf* e = elf_writer_get_elf(ew);
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    if(symbols_scn == NULL) return NULL;
    
    Elf32_Shdr* symbols_shdr32 = elf32_getshdr(symbols_scn);
    CHECK_ELF_FUNCTION_RESULT(symbols_shdr32, elf32_getshdr);
    
    if(bind != STB_GLOBAL)
    {
        printf("Only adding of global symbols is currently supported.\n");
        return NULL;
    }

    size_t offset;
    Elf32_Sym* sym32 = elf_writer_extend_section(ew, symbols_scn,
        sizeof(*sym32), 1, &offset);
    
    if(sym32 == NULL)
    {
        printf("Failed to add symbol descriptor to the symbols section.\n");
        return NULL;
    }
    
    //Should be set after call: sym32->st_name
    //Should be set after call: sym32->st_value
    //Should be set after call: sym32->st_size
    sym32->st_info = ELF32_ST_INFO(bind, type);
    sym32->st_other = STV_DEFAULT;
    //Should be set after call: sym32->st_shndx
    
    if(index)
    {
        *index = offset / sizeof(*sym32);
        //printf("Index of added symbol is %zu.\n", *index);
    }
    
    //printf("Symbol was added to the ELF.\n");
    return sym32;
}


Elf64_Sym* utils_elf64_add_symbol(ElfWriter* ew,
    unsigned char type, unsigned char bind, size_t* index)
{
    Elf* e = elf_writer_get_elf(ew);
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    if(symbols_scn == NULL) return NULL;
    
    Elf64_Shdr* symbols_shdr64 = elf64_getshdr(symbols_scn);
    CHECK_ELF_FUNCTION_RESULT(symbols_shdr64, elf64_getshdr);
    
    if(bind != STB_GLOBAL)
    {
        printf("Only adding of global symbols is currently supported.\n");
        return NULL;
    }

    size_t offset;
    Elf64_Sym* sym64 = elf_writer_extend_section(ew, symbols_scn,
        sizeof(*sym64), 1, &offset);
    
    if(sym64 == NULL)
    {
        printf("Failed to add symbol descriptor to the symbols section.\n");
        return NULL;
    }
    
    //Should be set after call: sym64->st_name
    //Should be set after call: sym64->st_value
    //Should be set after call: sym64->st_size
    sym64->st_info = ELF64_ST_INFO(bind, type);
    sym64->st_other = STV_DEFAULT;
    //Should be set after call: sym64->st_shndx
    
    if(index)
    {
        *index = offset / sizeof(*sym64);
    }
    return sym64;
}


int utils_elf_add_symbol_imported(ElfWriter* ew, size_t symbol_name,
    size_t* index)
{
    Elf* e = elf_writer_get_elf(ew);
    
    if(utils_elf_is32(e))
    {
        Elf32_Sym* sym32 = utils_elf32_add_symbol(ew,
            STT_NOTYPE, STB_GLOBAL, index);
        if(sym32 == NULL) return -ENOMEM;
        
        sym32->st_name = symbol_name;
        sym32->st_value = 0;
        sym32->st_size = 0;
        sym32->st_shndx = SHT_NULL;
    }
    else
    {
        Elf64_Sym* sym64 = utils_elf64_add_symbol(ew,
            STT_NOTYPE, STB_GLOBAL, index);
        if(sym64 == NULL) return -ENOMEM;
        
        sym64->st_name = symbol_name;
        sym64->st_value = 0;
        sym64->st_size = 0;
        sym64->st_shndx = SHT_NULL;
    }
    
    return 0;
}

int utils_elf32_add_symbol_version(ElfWriter* ew, const char* symbol_name,
    int32_t crc)
{
    Elf* e = elf_writer_get_elf(ew);
    
    Elf_Scn* versions_scn = get_versions_section(e);
    assert(versions_scn);
    
    struct symVersion32* symVer32;
    
    size_t record_len = align_val(sizeof(*symVer32)
        + strlen(symbol_name) + 1, 64);
    
    symVer32 = elf_writer_extend_section(ew, versions_scn, record_len,
        64, NULL);
    
    if(symVer32 == NULL)
    {
        printf("Failed to add record with version of imported symbol.\n");
        return -ENOMEM;
    }
    
    symVer32->crc = crc;
    strcpy(symVer32->name, symbol_name);
    
    memset(symVer32->name + strlen(symbol_name), 0,
        record_len - strlen(symbol_name) - sizeof(*symVer32));
    
    return 0;
}

int utils_elf64_add_symbol_version(ElfWriter* ew, const char* symbol_name,
    int64_t crc)
{
    Elf* e = elf_writer_get_elf(ew);
    
    Elf_Scn* versions_scn = get_versions_section(e);
    assert(versions_scn);
    
    struct symVersion64* symVer64;
    
    size_t record_len = align_val(sizeof(*symVer64)
        + strlen(symbol_name) + 1, 64);
    
    symVer64 = elf_writer_extend_section(ew, versions_scn, record_len,
        64, NULL);
    
    if(symVer64 == NULL)
    {
        printf("Failed to add record with version of imported symbol.\n");
        return -ENOMEM;
    }
    
    symVer64->crc = crc;
    strcpy(symVer64->name, symbol_name);
    
    memset(symVer64->name + strlen(symbol_name), 0,
        record_len - strlen(symbol_name) - sizeof(*symVer64));
    
    return 0;
}


int utils_elf32_copy_elf(ElfWriter* dest, Elf* src)
{
    int result;
    
    Elf* dest_e = elf_writer_get_elf(dest);
    
    Elf32_Ehdr* src_ehdr32 = elf32_getehdr(src);
    CHECK_ELF_FUNCTION_RESULT(src_ehdr32, elf32_getehdr);
    
    Elf32_Ehdr* dest_ehdr32 = elf32_newehdr(dest_e);
    CHECK_ELF_FUNCTION_RESULT(dest_ehdr32, elf32_newehdr);
    
#define COPY_VAR(var) dest_##var = src_##var;

    COPY_VAR(ehdr32->e_ident[EI_DATA]);
    COPY_VAR(ehdr32->e_ident[EI_VERSION]);
    COPY_VAR(ehdr32->e_machine);
    COPY_VAR(ehdr32->e_type);
    COPY_VAR(ehdr32->e_version);
    COPY_VAR(ehdr32->e_shstrndx);
    
    dest_ehdr32->e_entry = 0;
    
    dest_ehdr32->e_ehsize = 0;
    
    dest_ehdr32->e_phoff = 0LL;
    dest_ehdr32->e_shoff = 0LL;

    Elf_Scn* src_scn;
    for(src_scn = elf_nextscn(src, NULL);
        src_scn != NULL;
        src_scn = elf_nextscn(src, src_scn))
    {
        // Zeroth section is created automatically by libelf.
        if(elf_ndxscn(src_scn) == 0) continue;
        
        //printf("Add section %zu...\n", elf_ndxscn(src_scn));
        
        Elf_Scn* dest_scn = elf_newscn(dest_e);
        CHECK_ELF_FUNCTION_RESULT(dest_scn, elf_newscn);
        
        Elf32_Shdr* src_shdr32 = elf32_getshdr(src_scn);
        CHECK_ELF_FUNCTION_RESULT(src_shdr32, elf32_getshdr);

        Elf32_Shdr* dest_shdr32 = elf32_getshdr(dest_scn);
        CHECK_ELF_FUNCTION_RESULT(dest_shdr32, elf32_getshdr);

        COPY_VAR(shdr32->sh_name);
        COPY_VAR(shdr32->sh_type);
        COPY_VAR(shdr32->sh_flags);
        COPY_VAR(shdr32->sh_link);
        COPY_VAR(shdr32->sh_info);
        COPY_VAR(shdr32->sh_name);
        COPY_VAR(shdr32->sh_addralign);
        COPY_VAR(shdr32->sh_entsize);
        
        //printf("Add section %zu... done.\n", elf_ndxscn(src_scn));
        
        Elf_Data* src_data;
        
        if(src_shdr32->sh_type == SHT_NOBITS)
        {
            // No data for copy
            continue;
        }
        
        //printf("Add data for section %zu...\n", elf_ndxscn(src_scn));
        
        for(src_data = elf_getdata(src_scn, NULL);
            src_data != NULL;
            src_data = elf_getdata(src_scn, src_data))
        {
            Elf_Data* dest_data = elf_newdata(dest_scn);
            CHECK_ELF_FUNCTION_RESULT(dest_data, elf_newdata);
            
            COPY_VAR(data->d_buf);
            COPY_VAR(data->d_size);
            COPY_VAR(data->d_align);
            COPY_VAR(data->d_version);
            
            dest_data->d_off = 0LL;
        }
        
        //printf("Add data for section %zu... done.\n", elf_ndxscn(src_scn));
    }
    //First update
    result = elf_update(dest_e, ELF_C_NULL);
    CHECK_ELF_FUNCTION_RESULT(result >= 0, elf_update);

    // workaround bug in ELF, when it checks data from NOBITS section.
    Elf_Scn* scn;
    for(scn = elf_nextscn(dest_e, NULL);
        scn != NULL;
        scn = elf_nextscn(dest_e, scn))
    {
        Elf32_Shdr* shdr32 = elf32_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);
        
        if(shdr32->sh_type == SHT_NOBITS)
        {
            Elf_Data* data = elf_getdata(scn, NULL);
            if(data != NULL)
            {
                data->d_version = EV_CURRENT;
            }
        }
    }
    // Futhers elf_update(ELF_C_NULL) will work correctly.
    
    // Store data in ElfWriter
    for(scn = elf_nextscn(dest_e, NULL);
        scn != NULL;
        scn = elf_nextscn(dest_e, scn))
    {
        Elf32_Shdr* shdr32 = elf32_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);
        
        if(shdr32->sh_type == SHT_NOBITS) continue;
        
        Elf_Data* data;
        for(data = elf_getdata(scn, NULL);
            data != NULL;
            data = elf_getdata(scn, data))
        {
            result = elf_writer_store_data(dest, data);
            assert(result == 0);
        }
    }
    return 0;

#undef COPY_VAR
}

int utils_elf64_copy_elf(ElfWriter* dest, Elf* src)
{
    int result;
    
    Elf* dest_e = elf_writer_get_elf(dest);
    
    Elf64_Ehdr* src_ehdr64 = elf64_getehdr(src);
    CHECK_ELF_FUNCTION_RESULT(src_ehdr64, elf64_getehdr);
    
    Elf64_Ehdr* dest_ehdr64 = elf64_newehdr(dest_e);
    CHECK_ELF_FUNCTION_RESULT(dest_ehdr64, elf64_newehdr);
    
#define COPY_VAR(var) dest_##var = src_##var;

    COPY_VAR(ehdr64->e_ident[EI_DATA]);
    COPY_VAR(ehdr64->e_ident[EI_VERSION]);
    COPY_VAR(ehdr64->e_machine);
    COPY_VAR(ehdr64->e_type);
    COPY_VAR(ehdr64->e_version);
    COPY_VAR(ehdr64->e_shstrndx);
    
    dest_ehdr64->e_entry = 0;
    
    dest_ehdr64->e_ehsize = 0;
    
    dest_ehdr64->e_phoff = 0LL;
    dest_ehdr64->e_shoff = 0LL;

    Elf_Scn* src_scn;
    for(src_scn = elf_nextscn(src, NULL);
        src_scn != NULL;
        src_scn = elf_nextscn(src, src_scn))
    {
        // Zeroth section is created automatically by libelf.
        if(elf_ndxscn(src_scn) == 0) continue;
        
        //printf("Add section %zu...\n", elf_ndxscn(src_scn));
        
        Elf_Scn* dest_scn = elf_newscn(dest_e);
        CHECK_ELF_FUNCTION_RESULT(dest_scn, elf_newscn);
        
        Elf64_Shdr* src_shdr64 = elf64_getshdr(src_scn);
        CHECK_ELF_FUNCTION_RESULT(src_shdr64, elf64_getshdr);

        Elf64_Shdr* dest_shdr64 = elf64_getshdr(dest_scn);
        CHECK_ELF_FUNCTION_RESULT(dest_shdr64, elf64_getshdr);

        COPY_VAR(shdr64->sh_name);
        COPY_VAR(shdr64->sh_type);
        COPY_VAR(shdr64->sh_flags);
        COPY_VAR(shdr64->sh_link);
        COPY_VAR(shdr64->sh_info);
        COPY_VAR(shdr64->sh_name);
        COPY_VAR(shdr64->sh_addralign);
        COPY_VAR(shdr64->sh_entsize);
        
        //printf("Add section %zu... done.\n", elf_ndxscn(src_scn));
        
        Elf_Data* src_data;
        
        if(src_shdr64->sh_type == SHT_NOBITS)
        {
            // No data for copy
            continue;
        }
        
        //printf("Add data for section %zu...\n", elf_ndxscn(src_scn));
        
        for(src_data = elf_getdata(src_scn, NULL);
            src_data != NULL;
            src_data = elf_getdata(src_scn, src_data))
        {
            Elf_Data* dest_data = elf_newdata(dest_scn);
            CHECK_ELF_FUNCTION_RESULT(dest_data, elf_newdata);
            
            COPY_VAR(data->d_buf);
            COPY_VAR(data->d_size);
            COPY_VAR(data->d_align);
            COPY_VAR(data->d_version);
            
            dest_data->d_off = 0LL;
        }
        
        //printf("Add data for section %zu... done.\n", elf_ndxscn(src_scn));
    }
    //First update
    result = elf_update(dest_e, ELF_C_NULL);
    CHECK_ELF_FUNCTION_RESULT(result >= 0, elf_update);

    // workaround bug in ELF, when it checks data from NOBITS section.
    Elf_Scn* scn;
    for(scn = elf_nextscn(dest_e, NULL);
        scn != NULL;
        scn = elf_nextscn(dest_e, scn))
    {
        Elf64_Shdr* shdr64 = elf64_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);
        
        if(shdr64->sh_type == SHT_NOBITS)
        {
            Elf_Data* data = elf_getdata(scn, NULL);
            if(data != NULL)
            {
                data->d_version = EV_CURRENT;
            }
        }
    }
    // Futhers elf_update(ELF_C_NULL) will work correctly.
    
    // Store data in ElfWriter
    for(scn = elf_nextscn(dest_e, NULL);
        scn != NULL;
        scn = elf_nextscn(dest_e, scn))
    {
        Elf64_Shdr* shdr64 = elf64_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);
        
        if(shdr64->sh_type == SHT_NOBITS) continue;
        
        Elf_Data* data;
        for(data = elf_getdata(scn, NULL);
            data != NULL;
            data = elf_getdata(scn, data))
        {
            result = elf_writer_store_data(dest, data);
            assert(result == 0);
        }
    }
    return 0;

#undef COPY_VAR
}
