#ifndef USP_REPLACER_H
#define USP_REPLACER_H

#include "process_user_string.h"
#include "callback_replacer.h"

void usp_target_load_callback(struct module* m);
void usp_target_unload_callback(struct module* m);

int usp_payload_register(struct callback_payload* payload);
int usp_payload_unregister(struct callback_payload* payload);

user_string_processor
usp_replace(user_string_processor callback, void* data);

int usp_replacement_clean(void* data);

user_string_processor
usp_get_orig(void* data);

#endif