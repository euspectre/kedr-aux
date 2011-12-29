#include "binary_code_template.h"

#include <stdio.h>

#include "binary_instrument_utilities.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <err.h>
#include <sysexits.h>


/*
 * Set symbols of str to value of corresponded bytes(max 16).
 * 
 * NOTE: Terminater null-character is not set by this function.
 */
static void bytes_to_str16(char* str, const char* bytes, size_t len)
{
    assert(len <= 16);
    int i;
    for(i = 0; i < len; i++)
    {
        str[i] = isgraph(bytes[i]) ? bytes[i] : '.';
    }
    for(; i < 16; i++)
    {
        str[i] = ' ';
    }
}

/*
 * Convert integer value(0 <= value < 16)
 * to its hex digit representation.
 */
char int_to_hex(char value)
{
    //printf("Convert value %d to hex.\n", (int)value);
    if(value < 10) return value + '0';
    else return value + 'a' - 10;
}

/*
 * Set symbols of hex str to value of corresponded bytes(max 16):
 * 
 * '******** ******** ******** ********'
 * 
 * NOTE: Spaces between group of digits is not set by this function.
 * Same for terminated null-character.
 */
static void bytes_to_hex16(char hex[35], const char* bytes, size_t len)
{
    assert(len <= 16);

    int i;
    int addend = -1;
    
    for(i = 0; i < len; i++)
    {
        if(i % 4 == 0) addend++;

        hex[i * 2 + addend] = int_to_hex((bytes[i] & 0xf0) >> 4);
        hex[i * 2 + addend + 1] = int_to_hex(bytes[i] & 0xf);
    }
    for(; i < 16; i++)
    {
        if(i % 4 == 0) addend++;

        hex[i * 2 + addend] = ' ';
        hex[i * 2 + addend + 1] = ' ';
    }
}


void ct_print(struct code_template* template)
{
    printf("Section template contains %s-bit code(size - %zu):\n",
        template->is_elf32 ? "32" : "64",
        template->code_size);
    
    char hex[36] = "******** ******** ******** ********";
    char str[17] = "****************";
    
    int i;
    
    int piece_size;
    for(i = 0; i < template->code_size; i+= piece_size)
    {
        piece_size = template->code_size - i;
        if(piece_size > 16) piece_size = 16;
        
        bytes_to_hex16(hex, template->code + i, piece_size);
        bytes_to_str16(str, template->code + i, piece_size);
        
        printf("  %08x %s %s\n", i, hex, str);
    }
    
    if(*template->symbols == NULL) return;
    printf("Next symbols are used by this section:\n");
    char** symbol;
    for(symbol = template->symbols; *symbol != NULL; symbol++)
    {
        printf("  %s\n", *symbol);
    }
    
    if(template->n_relocations == 0) return;
    
    printf("Next relocations are defined for this section:\n");
    if(template->is_elf32)
    {
        if(template->rel_type == SHT_RELA)
        {
            printf("   Offset  Type   Symbol+addend\n");
            for(i = 0; i < template->n_relocations; i++)
            {
                const struct code_template_rel* relocation =
                    &template->relocations[i];
                printf("  %.08x  %.02x  %s + %x\n",
                    (int)relocation->offset,
                    (int)relocation->type,
                    template->symbols[relocation->symbol],
                    (int)relocation->addend);
            }
        }
        else
        {
            printf("   Offset  Type   Symbol\n");
            for(i = 0; i < template->n_relocations; i++)
            {
                const struct code_template_rel* relocation =
                    &template->relocations[i];
                printf("  %.08x  %.02x  %s\n",
                    (int)relocation->offset,
                    (int)relocation->type,
                    template->symbols[relocation->symbol]);
            }
        }
    }
    else
    {
        if(template->rel_type == SHT_RELA)
        {
            printf("       Offset      Type   Symbol+addend\n");
            for(i = 0; i < template->n_relocations; i++)
            {
                const struct code_template_rel* relocation =
                    &template->relocations[i];
                printf("  %.16x  %.02x  %s + %lx\n",
                    (int)relocation->offset,
                    (int)relocation->type,
                    template->symbols[relocation->symbol],
                    (long)relocation->addend);
            }
        }
        else
        {
            printf("       Offset      Type   Symbol\n");
            for(i = 0; i < template->n_relocations; i++)
            {
                const struct code_template_rel* relocation =
                    &template->relocations[i];
                printf("  %.16x %.02x  %s\n",
                    (int)relocation->offset,
                    (int)relocation->type,
                    template->symbols[relocation->symbol]);
            }
        }
    }
}


static Elf_Scn* create_relocations_scn(ElfWriter* ew, Elf_Scn* scn,
    int rel_type)
{
    Elf* e = elf_writer_get_elf(ew);
    int result;
    int elf_is32 = utils_elf_is32(e);

    size_t strndx;
    result = elf_getshdrstrndx(e, &strndx);
    CHECK_ELF_FUNCTION_RESULT(result == 0, elf_getshdrstrndx);
    
    Elf_Scn* symbols_scn = utils_elf_get_symbols_section(e);
    assert(symbols_scn);
    
    size_t section_name_offset;
    if(elf_is32)
    {
        Elf32_Shdr* shdr32 = elf32_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);
        
        section_name_offset = shdr32->sh_name;
    }
    else
    {
        Elf64_Shdr* shdr64 = elf64_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);
        
        section_name_offset = shdr64->sh_name;
    }
    
    const char* section_name = elf_strptr(e, strndx, section_name_offset);
    CHECK_ELF_FUNCTION_RESULT(section_name, elf_strptr);
    
    //printf("Create relocations section for %s.\n", section_name);
    
    static const char prefix_rel[] = ".rel";
    static const char prefix_rela[] = ".rela";
    
    size_t rel_section_name_size = (rel_type == SHT_RELA)
        ? sizeof(prefix_rela) : sizeof(prefix_rel) + strlen(section_name);
    
    Elf_Scn* str_scn = elf_getscn(e, strndx);
    CHECK_ELF_FUNCTION_RESULT(str_scn, elf_getscn);
    
    size_t rel_section_name_offset;
    char* rel_section_name = elf_writer_extend_section(ew, str_scn,
        rel_section_name_size, 1, &rel_section_name_offset);
    assert(rel_section_name);
    
    if(rel_type == SHT_RELA)
    {
        memcpy(rel_section_name, prefix_rela, sizeof(prefix_rela) - 1);
        memcpy(rel_section_name + sizeof(prefix_rela) - 1, section_name,
            rel_section_name_size - sizeof(prefix_rela) + 1);
    }
    else
    {
        memcpy(rel_section_name, prefix_rel, sizeof(prefix_rel) - 1);
        memcpy(rel_section_name + sizeof(prefix_rel) - 1, section_name,
            rel_section_name_size - sizeof(prefix_rel) + 1);
    }
        
    Elf_Scn* relocations_scn = elf_newscn(e);
    CHECK_ELF_FUNCTION_RESULT(relocations_scn, elf_newscn);
        
    if(elf_is32)
    {
        Elf32_Shdr* relocations_shdr32 = elf32_getshdr(relocations_scn);
        CHECK_ELF_FUNCTION_RESULT(relocations_shdr32, elf32_getshdr);
        
        relocations_shdr32->sh_name = rel_section_name_offset;
        relocations_shdr32->sh_type = rel_type;
        relocations_shdr32->sh_flags = 0;
        relocations_shdr32->sh_link = elf_ndxscn(symbols_scn);
        relocations_shdr32->sh_info = elf_ndxscn(scn);
        relocations_shdr32->sh_addralign = 32 / 8;
        relocations_shdr32->sh_entsize = (rel_type == SHT_RELA) 
            ? sizeof(Elf32_Rela) : sizeof(Elf32_Rel);
    }
    else
    {
        Elf64_Shdr* relocations_shdr64 = elf64_getshdr(relocations_scn);
        CHECK_ELF_FUNCTION_RESULT(relocations_shdr64, elf64_getshdr);
        
        relocations_shdr64->sh_name = rel_section_name_offset;
        relocations_shdr64->sh_type = rel_type;
        relocations_shdr64->sh_flags = 0;
        relocations_shdr64->sh_link = elf_ndxscn(symbols_scn);
        relocations_shdr64->sh_info = elf_ndxscn(scn);
        relocations_shdr64->sh_addralign = 64 / 8;
        relocations_shdr64->sh_entsize = (rel_type == SHT_RELA) 
            ? sizeof(Elf64_Rela) : sizeof(Elf64_Rel);
    }
    
    return relocations_scn;
}

/*
 * Instansiate one relocation element.
 */
static void ct_rel_copy_rela32(Elf32_Rela* dst,
    struct code_template_rel* src, int32_t* symbols)
{
    dst->r_offset = src->offset;
    dst->r_info = ELF32_R_INFO(symbols[src->symbol], src->type);
    dst->r_addend = src->addend;
}

static void ct_rel_copy_rel32(Elf32_Rel* dst,
    struct code_template_rel* src, int32_t* symbols)
{
    dst->r_offset = src->offset;
    dst->r_info = ELF32_R_INFO(symbols[src->symbol], src->type);
}

static void ct_rel_copy_rela64(Elf64_Rela* dst,
    struct code_template_rel* src, int32_t* symbols)
{
    dst->r_offset = src->offset;
    dst->r_info = ELF64_R_INFO(symbols[src->symbol], src->type);
    dst->r_addend = src->addend;
}

static void ct_rel_copy_rel64(Elf64_Rel* dst,
    struct code_template_rel* src, int32_t* symbols)
{
    dst->r_offset = src->offset;
    dst->r_info = ELF64_R_INFO(symbols[src->symbol], src->type);
}

/*
 * Add and fill relocations data according to template.
 * 
 * 'symbols' should be array of symbol indices, corresponded to
 * 'symbols' array in the template.
 * 
 * Offsets of all relocations will be corrected as if code moved to
 * offset 'code_offset'.
 */

static void ct_add_relocations_data(struct code_template* template,
    ElfWriter* ew, Elf_Scn* scn, size_t code_offset,
    int32_t* symbols)
{
    if(template->is_elf32)
    {
        Elf32_Shdr* shdr32 = elf32_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr32, elf32_getshdr);
        
        void* buf = elf_writer_extend_section(ew, scn,
            shdr32->sh_entsize * template->n_relocations,
            shdr32->sh_addralign,
            NULL);
        assert(buf);

        int i;
        if(template->rel_type == SHT_RELA)
        {
            for(i = 0; i < template->n_relocations; i++)
            {
                Elf32_Rela* rela32 = (Elf32_Rela*)buf + i;
                ct_rel_copy_rela32(rela32,
                    &template->relocations[i], symbols);
                rela32->r_offset += code_offset;
            }
        }
        else
        {
            for(i = 0; i < template->n_relocations; i++)
            {
                Elf32_Rel* rel32 = (Elf32_Rel*)buf + i;
                ct_rel_copy_rel32(rel32,
                    &template->relocations[i], symbols);
                rel32->r_offset += code_offset;
            }
        }
    }
    else
    {
        Elf64_Shdr* shdr64 = elf64_getshdr(scn);
        CHECK_ELF_FUNCTION_RESULT(shdr64, elf64_getshdr);
        
        void* buf = elf_writer_extend_section(ew, scn,
            shdr64->sh_entsize * template->n_relocations,
            shdr64->sh_addralign,
            NULL);
        assert(buf);

        int i;
        if(template->rel_type == SHT_RELA)
        {
            for(i = 0; i < template->n_relocations; i++)
            {
                Elf64_Rela* rela64 = (Elf64_Rela*)buf + i;
                ct_rel_copy_rela64(rela64,
                    &template->relocations[i], symbols);
                rela64->r_offset += code_offset;
            }
        }
        else
        {
            for(i = 0; i < template->n_relocations; i++)
            {
                Elf64_Rel* rel64 = (Elf64_Rel*)buf + i;
                ct_rel_copy_rel64(rel64,
                    &template->relocations[i], symbols);
                rel64->r_offset += code_offset;
            }
        }
    }
}

/*
 * Create array of symbols indices, corresponded to symbol names.
 * 
 * ('symbols_orig' is an array of symbols names, terminated with NULL string).
 * 
 * Array should be freed when no longer used.
 */
static int32_t* translate_symbols(Elf* e,
    char** symbols_orig,
    const struct ct_symbol_correspondence* symbol_correspondences)
{
    size_t size = 0;
    // Count symbols
    char** symbol_orig;
    for(symbol_orig = symbols_orig; *symbol_orig != NULL; symbol_orig++)
        size++;
    
    int32_t* symbols = malloc(size * sizeof(int32_t));
    if(symbols == NULL)
    {
        printf("Failed to allocate symbols array.\n");
        return NULL;
    }
    
    int i;
    for(i = 0; i < size; i++)
    {
        const char* symbol_orig = symbols_orig[i];
        const struct ct_symbol_correspondence* symbol_correspondence;
        for(symbol_correspondence = symbol_correspondences;
            symbol_correspondence->name != NULL;
            symbol_correspondence++)
        {
            if(strcmp(symbol_orig, symbol_correspondence->name) == 0)
                break;
        }
        if(symbol_correspondence->name == NULL)
        {
            printf("Template symbol %s has no corresponded one.\n",
                symbol_orig);
            free(symbols);
            return NULL;
        }
        
        symbols[i] = symbol_correspondence->index;
    }
    return symbols;
}

int ct_instantiate(struct code_template* template,
    ElfWriter* ew, Elf_Scn* scn,
    const struct ct_symbol_correspondence* symbol_correspondences,
    size_t* offset)
{
    Elf* e = elf_writer_get_elf(ew);
    int elf_is32 = utils_elf_is32(e);
    
    int32_t* symbols = translate_symbols(e, template->symbols,
        symbol_correspondences);
    
    if(symbols == NULL) return -EINVAL;
    
    //printf("Extend section...\n");
    void* code = elf_writer_extend_section(ew, scn, template->code_size,
        template->align, offset);
    assert(code);
    //printf("Extend section...done.\n");
    
    memcpy(code, template->code, template->code_size);
    
    Elf_Scn* relocations_scn = utils_elf_find_relocation_section(e,
        elf_ndxscn(scn));

    if(template->is_elf32)
    {
        if(!elf_is32)
        {
            printf("32-bit template cannot be instantiated in 64-bit ELF.");
            free(symbols);
            return -EINVAL;
        }
    }
    else
    {
        if(elf_is32)
        {
            printf("64-bit template cannot be instantiated in 32-bit ELF.");
            free(symbols);
            return -EINVAL;
        }
    }
    
    if(relocations_scn == NULL)
    {
        relocations_scn = create_relocations_scn(ew, scn, template->rel_type);
        assert(relocations_scn);
        //printf("New relocations section has been created.\n");
    }

    if(template->is_elf32)
    {
        Elf32_Shdr* rel_shdr32 = elf32_getshdr(relocations_scn);
        CHECK_ELF_FUNCTION_RESULT(rel_shdr32, elf32_getshdr);
        
        if(rel_shdr32->sh_type != template->rel_type)
        {
            printf("Incompatible type of relocation section.\n");
            free(symbols);
            return -EINVAL;
        }
    }
    else
    {
        Elf64_Shdr* rel_shdr64 = elf64_getshdr(relocations_scn);
        CHECK_ELF_FUNCTION_RESULT(rel_shdr64, elf64_getshdr);
        
        if(rel_shdr64->sh_type != template->rel_type)
        {
            printf("Incompatible type of relocation section.\n");
            free(symbols);
            return -EINVAL;
        }
    }

    ct_add_relocations_data(template, ew, relocations_scn, *offset, symbols);
    
    free(symbols);

    return 0;
}

static int ct_rel_new(struct code_template* template)
{
    template->relocations = NULL;
    template->n_relocations = 0;
    
    return 0;
}

static void ct_rel_destroy(struct code_template* template)
{
    free(template->relocations);
}

// Return newly allocated relocation
static struct code_template_rel* ct_rel_add(
    struct code_template* template)
{
    int n_relocations_new = template->n_relocations + 1;
    struct code_template_rel* new_relocations = realloc(
        template->relocations, n_relocations_new * sizeof(*new_relocations));
    if(new_relocations == NULL)
    {
        printf("Failed to reallocate array for relocations for template.\n");
        return NULL;
    }
    
    template->relocations = new_relocations;
    template->n_relocations = n_relocations_new;
    
    return new_relocations + n_relocations_new - 1;
}

static char** string_array_new(void)
{
    char** string_array = malloc(sizeof(*string_array));
    if(string_array == NULL)
    {
        printf("Failed to allocate array of strings.\n");
        return NULL;
    }
    
    *string_array = NULL;
    return string_array;
}
static void string_array_destroy(char** string_array)
{
    char* const* string_elem;
    for(string_elem = string_array; *string_elem != NULL; string_elem++)
        free(*string_elem);
    
    free(string_array);
}

/*
 * Return index of string in the array.
 * 
 * In fail return negative error code.
 */
static size_t string_array_add_string_unique(char*** string_array,
    const char* str)
{
    char** string_elem;
    for(string_elem = *string_array; *string_elem != NULL; string_elem++)
    {
        if(strcmp(*string_elem, str) == 0)
        {
            return string_elem - *string_array;
        }
    }
    
    char* new_str = strdup(str);
    if(new_str == NULL)
    {
        printf("Failed to allocate string.\n");
        return -ENOMEM;
    }
    
    size_t new_size = string_elem - *string_array + 1;
    char** new_string_array = realloc(*string_array,
        (new_size + 1) * sizeof(*new_string_array));
    
    if(new_string_array == NULL)
    {
        printf("Failed to reallocate array of strings.\n");
        free(new_str);
        return -ENOMEM;
    }
    new_string_array[new_size - 1] = new_str;
    new_string_array[new_size] = NULL;
    *string_array = new_string_array;
    
    return new_size - 1;
}

// Return index of the added symbol.
static size_t ct_add_sym32(struct code_template* template,
    Elf* e, size_t symbol_index, size_t symbol_section_index)
{
    Elf_Scn* symbol_scn = elf_getscn(e, symbol_section_index);
    CHECK_ELF_FUNCTION_RESULT(symbol_scn, elf_getscn);
    
    Elf32_Shdr* symbol_shdr32 = elf32_getshdr(symbol_scn);
    CHECK_ELF_FUNCTION_RESULT(symbol_shdr32, elf32_getshdr);
    
    assert(symbol_shdr32->sh_type == SHT_SYMTAB);
    
    Elf_Data* symbol_data = elf_getdata(symbol_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(symbol_data, elf_getdata);
    
    Elf32_Sym* sym32 = (Elf32_Sym*)symbol_data->d_buf + symbol_index;
    assert((char*)sym32 - (char*)symbol_data->d_buf < symbol_data->d_size);
    
    if(ELF32_ST_TYPE(sym32->st_info) == STT_SECTION)
    {
        printf("Cannot process relocations with section-type symbols.\n");
        return -EINVAL;
    }
    
    const char* symbol_name = elf_strptr(e, symbol_shdr32->sh_link,
        sym32->st_name);
    CHECK_ELF_FUNCTION_RESULT(symbol_name, elf_strptr);
    
    return string_array_add_string_unique(&template->symbols, symbol_name);
}

static size_t ct_add_sym64(struct code_template* template,
    Elf* e, size_t symbol_index, size_t symbol_section_index)
{
    Elf_Scn* symbol_scn = elf_getscn(e, symbol_section_index);
    CHECK_ELF_FUNCTION_RESULT(symbol_scn, elf_getscn);
    
    Elf64_Shdr* symbol_shdr64 = elf64_getshdr(symbol_scn);
    CHECK_ELF_FUNCTION_RESULT(symbol_shdr64, elf64_getshdr);
    
    assert(symbol_shdr64->sh_type == SHT_SYMTAB);
    
    Elf_Data* symbol_data = elf_getdata(symbol_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(symbol_data, elf_getdata);
    
    Elf64_Sym* sym64 = (Elf64_Sym*)symbol_data->d_buf + symbol_index;
    assert((char*)sym64 - (char*)symbol_data->d_buf < symbol_data->d_size);
    
    if(ELF64_ST_TYPE(sym64->st_info) == STT_SECTION)
    {
        printf("Cannot process relocations with section-type symbols.\n");
        return -EINVAL;
    }
    
    const char* symbol_name = elf_strptr(e, symbol_shdr64->sh_link,
        sym64->st_name);
    CHECK_ELF_FUNCTION_RESULT(symbol_name, elf_strptr);
    
    return string_array_add_string_unique(&template->symbols, symbol_name);
}

// Add relocation.
static int ct_add_rel32(struct code_template* template,
    Elf* e, Elf32_Addr offset, Elf32_Word info, Elf32_Xword addend,
    Elf32_Word symbol_section_index)
{
    int symbol_index = ct_add_sym32(template, e, ELF32_R_SYM(info),
        symbol_section_index);
    if(symbol_index < 0) return symbol_index;
    
    struct code_template_rel* relocation = ct_rel_add(template);
    
    if(relocation == NULL) return -ENOMEM;
    
    relocation->offset = offset;
    relocation->type = ELF32_R_TYPE(info);
    relocation->symbol = symbol_index;
    relocation->addend = addend;
    
    assert((addend == 0) || (template->rel_type == SHT_RELA));
    
    return 0;
}

static int ct_add_rel64(struct code_template* template,
    Elf* e, Elf64_Addr offset, Elf64_Xword info, Elf64_Xword addend,
    Elf64_Word symbol_section_index)
{
    size_t symbol_index = ct_add_sym64(template, e, ELF64_R_SYM(info),
        symbol_section_index);
    if(symbol_index < 0) return symbol_index;
    
    struct code_template_rel* relocation = ct_rel_add(template);
    
    if(relocation == NULL) return -ENOMEM;
    
    relocation->offset = offset;
    relocation->type = ELF64_R_TYPE(info);
    relocation->symbol = symbol_index;
    relocation->addend = addend;
    
    assert((addend == 0) || (template->rel_type == SHT_RELA));
    
    return 0;
}

struct code_template* ct_load(Elf* e, Elf_Scn* scn, size_t offset,
    size_t size, size_t align)
{
    struct code_template* template = malloc(sizeof(*template));
    if(template == NULL)
    {
        printf("Failed to allocate template structure.\n");
        return NULL;
    }
    
    template->is_elf32 = utils_elf_is32(e);
    
    Elf_Data* data = elf_getdata(scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(data, elf_getdata);
    
    assert(offset < data->d_size);
    assert(offset + size <= data->d_size);
    
    template->code_size = size;
    template->align = align;
    
    template->code = malloc(template->code_size);
    if(template->code == NULL)
    {
        printf("Failed to allocate code for template.\n");
        goto err_code;
    }
    
    template->symbols = string_array_new();
    if(template->symbols == NULL) goto err_symbols;

    if(ct_rel_new(template)) goto err_relocations;

    memcpy(template->code, data->d_buf + offset, template->code_size);
    
    Elf_Scn* relocations_scn = utils_elf_find_relocation_section(e,
        elf_ndxscn(scn));
    if(relocations_scn == NULL) goto out; //everything is done
    
    Elf_Data* relocations_data = elf_getdata(relocations_scn, NULL);
    CHECK_ELF_FUNCTION_RESULT(relocations_data, elf_getdata);
    
    if(template->is_elf32)
    {
        Elf32_Shdr* relocations_shdr32 = elf32_getshdr(relocations_scn);
        CHECK_ELF_FUNCTION_RESULT(relocations_shdr32, elf32_getshdr);
        
        int symbol_section_index = relocations_shdr32->sh_link;
        template->rel_type = relocations_shdr32->sh_type;
        
        if(template->rel_type == SHT_RELA)
        {
            Elf32_Rela *rela32, *rela32_end;
            rela32_end = (typeof(rela32_end))
                ((char*)relocations_data->d_buf + relocations_data->d_size);
            
            for(rela32 = relocations_data->d_buf; rela32 < rela32_end; rela32++)
            {
                if((rela32->r_offset < offset)
                    || (rela32->r_offset >= offset + size))
                    continue;

                int result = ct_add_rel32(template, e,
                    rela32->r_offset - offset,
                    rela32->r_info, rela32->r_addend,
                    symbol_section_index);
                if(result) goto err_relocation;
            }
        }
        else
        {
            Elf32_Rel *rel32, *rel32_end;
            rel32_end = (typeof(rel32_end))
                ((char*)relocations_data->d_buf + relocations_data->d_size);
            
            for(rel32 = relocations_data->d_buf; rel32 < rel32_end; rel32++)
            {
                if((rel32->r_offset < offset)
                    || (rel32->r_offset >= offset + size))
                    continue;

                int result = ct_add_rel32(template, e,
                    rel32->r_offset - offset,
                    rel32->r_info, 0,
                    symbol_section_index);
                if(result) goto err_relocation;
            }
        }
    }
    else
    {
        Elf64_Shdr* relocations_shdr64 = elf64_getshdr(relocations_scn);
        CHECK_ELF_FUNCTION_RESULT(relocations_shdr64, elf64_getshdr);
        
        int symbol_section_index = relocations_shdr64->sh_link;
        template->rel_type = relocations_shdr64->sh_type;
        
        if(template->rel_type == SHT_RELA)
        {
            Elf64_Rela *rela64, *rela64_end;
            rela64_end = (typeof(rela64_end))
                ((char*)relocations_data->d_buf + relocations_data->d_size);
            
            for(rela64 = relocations_data->d_buf; rela64 < rela64_end; rela64++)
            {
                if((rela64->r_offset < offset)
                    || (rela64->r_offset >= offset + size))
                    continue;

                int result = ct_add_rel64(template, e,
                    rela64->r_offset - offset,
                    rela64->r_info, rela64->r_addend,
                    symbol_section_index);
                if(result) goto err_relocation;
            }
        }
        else
        {
            Elf64_Rel *rel64, *rel64_end;
            rel64_end = (typeof(rel64_end))
                ((char*)relocations_data->d_buf + relocations_data->d_size);
            
            for(rel64 = relocations_data->d_buf; rel64 < rel64_end; rel64++)
            {
                if((rel64->r_offset < offset)
                    || (rel64->r_offset >= offset + size))
                    continue;

                int result = ct_add_rel64(template, e,
                    rel64->r_offset - offset,
                    rel64->r_info, 0,
                    symbol_section_index);
                if(result) goto err_relocation;
            }
        }
    }


out:
    return template;

err_relocation:
    ct_rel_destroy(template);
err_relocations:
    string_array_destroy(template->symbols);
err_symbols:
    free(template->code);
err_code:
    free(template);
    
    return NULL;
}

void ct_destroy(struct code_template* template)
{
    ct_rel_destroy(template);
    string_array_destroy(template->symbols);
    free(template->code);
    free(template);
}