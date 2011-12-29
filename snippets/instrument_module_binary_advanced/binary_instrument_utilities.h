/*
 * Common utilities used for binary instrumentation of elf file.
 */

#ifndef BINARY_INSTRUMENT_UTILITIES_H
#define BINARY_INSTRUMENT_UTILITIES_H

#include <libelf.h>
#include <sysexits.h>
#include <err.h>

#define CHECK_ELF_FUNCTION_RESULT(cond, function) \
    while(!(cond)) {errx(EX_SOFTWARE, "%s:%d: %s() failed: %s.", __FILE__, __LINE__, #function, elf_errmsg(-1)); }

#include "elf_writer.h"

/*
 * Return data at given offset inside section.
 * 
 * Correctly process multiple data blocks in section.
 */
void* utils_elf_get_data_at(Elf_Scn* scn, size_t offset);

/*
 *  Similar to elf_strptr, but work with section of any type,
 * not only SHT_STRTAB.
 */
char* utils_elf_strptr(Elf* e, size_t index, size_t offset);

/*
 * Determine, whether ELF class is 32 or 64.
 * 
 * In first case return non-zero, in the second one - zero.
 */
int utils_elf_is32(Elf* e);

/*
 * Find section with given name.
 * 
 * Return NULL if not found.
 */
Elf_Scn* utils_elf_find_section(Elf* e, const char* section_name);


/*
 * Return section contained symbols.
 * 
 * On error return NULL.
 * 
 * NOTE: Absent of symbol section is treated as error.
 */
Elf_Scn* utils_elf_get_symbols_section(Elf* e);

/*
 * Return identificator for relocation section for given one.
 * 
 * If such section is absent, return NULL.
 */
Elf_Scn* utils_elf_find_relocation_section(Elf* e, int section_index);


/*
 * Return symbol with given name.
 * 
 * If 'index' is not NULL, it will be set to the index of the
 * founded symbol.
 * 
 * Return NULL if symbol doesn't exists.
 */
Elf32_Sym* utils_elf32_find_symbol(Elf* e, const char* symbol_name,
    size_t* index);
Elf64_Sym* utils_elf64_find_symbol(Elf* e, const char* symbol_name,
    size_t* index);
/*
 * Whether crc-hash is provided for exported symbols.
 */
int utils_elf_has_symbol_versions(Elf* e);

/*
 * Return crc-hash for given symbol
 */
int32_t utils_elf32_get_symbol_version(Elf* e, const char* symbol_name);
int64_t utils_elf64_get_symbol_version(Elf* e, const char* symbol_name);

/*
 * Whether crc-hash is required for imported symbols.
 */
int utils_elf_require_symbol_versions(Elf* e);

/*
 * Return crc-hash required for given symbol.
 */
int32_t utils_elf32_get_symbol_required_version(Elf* e, const char* symbol_name);
int64_t utils_elf64_get_symbol_required_version(Elf* e, const char* symbol_name);


/*
 * Resolve relocation and return pointer to the allocated string,
 * pointed by this relocation.
 */
char* resolve_relocated_str32(Elf* e,
    Elf_Scn* relocation_scn,
    int relocation_index,
    Elf32_Addr addr);
char* resolve_relocated_str64(Elf* e,
    Elf_Scn* relocation_scn,
    int relocation_index,
    Elf64_Addr addr);


/*
 * Add string to the given string section.
 * 
 * If 'offset' is not NULL, it will set to the offset of the added
 * string.
 */
int utils_elf_add_string(ElfWriter* ew, Elf_Scn* scn,
    const char* str, size_t* offset);

/*
 * Add symbol to ELF and return it.
 * 
 * On error return NULL.
 * 
 * If 'index' is not NULL, it will be set to the index of the symbol
 * added.
 * 
 * NOTE: in the returned object 'st_name', 'st_size',
 * 'st_value' and 'st_shndx' fields should be filled.
 */
Elf32_Sym* utils_elf32_add_symbol(ElfWriter* ew,
    unsigned char type, unsigned char bind, size_t* index);
Elf64_Sym* utils_elf64_add_symbol(ElfWriter* ew,
    unsigned char type, unsigned char bind, size_t* index);

/*
 * Shortcat for adding imported symbol.
 * All fields of added symbol are set.
 * 
 * NOTE: crc should be added after this call if required.
 */
int utils_elf_add_symbol_imported(ElfWriter* ew, size_t symbol_name,
    size_t* index);

/*
 * Add information about requirement crc for symbol.
 */
int utils_elf32_add_symbol_version(ElfWriter* ew, const char* symbol_name,
    int32_t crc);
int utils_elf64_add_symbol_version(ElfWriter* ew, const char* symbol_name,
    int64_t crc);

/*
 * Copy content of 'src' ELF into 'dest'.
 * After call, src ELF may be closed.
 * 
 * This function is need as a workaround for libelf inability
 * to correctly add sections into existing ELF file.
 * 
 * Using this utility, one can create new ELF file, copy all content
 * from src ELF into it, and work with new ELF as modifiable.
 */
int utils_elf32_copy_elf(ElfWriter* dest, Elf* src);
int utils_elf64_copy_elf(ElfWriter* dest, Elf* src);

#endif /* BINARY_INSTRUMENT_UTILITIES_H */