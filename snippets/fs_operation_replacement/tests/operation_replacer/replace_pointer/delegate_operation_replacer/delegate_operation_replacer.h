#ifndef DELEGATE_OPERATIONS_REPLACER_H
#define DELEGATE_OPERATIONS_REPLACER_H

#include "operation_replacer.h"
#include "delegate.h"

//int delegate_operations_replacer_init(int n_elems);
//void delegate_operations_replacer_destroy(void);

int delegate_operations_payload_register(struct operation_payload* payload);
int delegate_operations_payload_unregister(struct operation_payload* payload);

int delegate_operations_payload_register_special(struct operation_payload* payload);
int delegate_operations_payload_unregister_special(struct operation_payload* payload);

void delegate_operations_target_load_callback(struct module* m);
void delegate_operations_target_unload_callback(struct module* m);

int delegate_operations_replace(struct delegate_object* obj);
int delegate_operations_restore(struct delegate_object* obj);

int delegate_operations_replacement_update(struct delegate_object* obj);

void* delegate_operations_get_orig_f(int op_offset, struct delegate_object* obj);
#define delegate_operations_get_orig(op, obj) \
    ((typeof(((struct delegate_operations*)0)->op)) \
        delegate_operations_get_orig_f(offsetof(struct delegate_operations, op), obj))

void* delegate_operations_get_orig_special_f(int op_offset, struct delegate_object* obj,
    struct operation_payload* payload);

#define delegate_operations_get_orig_special(op, obj, payload) \
    ((typeof(((struct delegate_operations*)0)->op)) \
        delegate_operations_get_orig_special_f(offsetof(struct delegate_operations, op), obj, payload))


#endif