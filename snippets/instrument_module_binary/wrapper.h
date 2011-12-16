/*
 * Definitions fo wrapper module, which are used for instrumentation of
 * the target.
 */

#ifndef WRAPPER_H
#define WRAPPER_H

#define REPLACEMENT_SECTION_NAME ".kedr.replacements"

#ifdef __KERNEL__

struct replacement_desc
{
    const char* function_name;
    const char* replacement_name;
};

#define declare_replacement(_function, _replacement) \
extern typeof(_function) _function; \
extern typeof(_replacement) _replacement; \
struct replacement_desc _function##_replacement_desc \
__attribute__((section(REPLACEMENT_SECTION_NAME))) = \
{.function_name = #_function, .replacement_name = #_replacement}

#else /* __KERNEL__ */

#include <libelf.h>

struct replacement_desc32
{
    Elf32_Addr function_name;
    Elf32_Addr replacement_name;
};

struct replacement_desc64
{
    Elf64_Addr function_name;
    Elf64_Addr replacement_name;
};

#endif /* __KERNEL__ */

#endif /* WRAPPER_H */