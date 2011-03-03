#ifndef PROCESS_USER_STRING
#define PROCESS_USER_STRING

/*
 * Call caller-defined function for the string,
 * which comes from user-space.
 */

#include <linux/types.h>

typedef void (*user_string_processor)(const char* str, void* data);

/*
 * Return negative error code if 'buf' or 'count' are incorrect.
 * Otherwise return 0.
 */
int process_user_string(const char __user* buf, size_t count,
    user_string_processor usp, void *data);

#endif