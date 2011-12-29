#ifndef ELF_WRITER_H
#define ELF_WRITER_H

/*
 * Support for convinient modification of ELF.
 */

#include <libelf.h>

/*
 * Object for control writing into ELF.
 */
typedef struct _ElfWriter ElfWriter;

/*
 * Create ElfWriter object for given ELF.
 * 
 * NOTE: 'e' should be opened for read and write.
 */
ElfWriter* elf_writer_create(Elf* e);

/*
 * Return ELF assosiated with given ElfWriter.
 */
Elf* elf_writer_get_elf(ElfWriter* ew);

/*
 * This method should be called just before closing ELF,
 * assosiated with given ElfWriter.
 */
void elf_writer_destroy(ElfWriter* ew);

/*
 * Append data of given size to ELF and return pointer to it.
 * 
 * If not NULL, 'offset' will be set to the offset of the appended data
 * in the section.
 * 
 * Appended data are allocated internally and will be destroyed
 * with ElfWriter itself.
 * 
 * elf_update(ELF_C_NULL) will be called automatically for assosiated
 * ELF.
 * For reflect changings in file, one should call
 * elf_update(ELF_C_WRITE) some time after this method.
 */
void* elf_writer_extend_section(ElfWriter* ew, Elf_Scn* scn,
    size_t additional_size, int align, size_t* offset);


/*
 * For workaround for bugged processing secondary elf_update().
 */
int elf_writer_store_data(ElfWriter* ew, Elf_Data* data);

#endif /* ELF_WRITER_H */