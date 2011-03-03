#ifndef CALLBACK_REPLACER_H
#define CALLBACK_REPLACER_H

#include <linux/types.h>
#include <linux/module.h>


struct callback_payload
{
    struct module *m;
    void* callback;
    void (*target_load_callback)(struct module* m);
    void (*target_unload_callback)(struct module* m);
};

typedef struct callback_replacer* callback_replacer;


/*
 * Create callback replacer.
 */

callback_replacer callback_replacer_create(size_t elem_num);

/*
 * Destroy replacer and free all resources it use.
 */

void callback_replacer_destroy(callback_replacer replacer);

/*
 * Replace callback.
 * 
 * Return pointer to the replacement callback.
 * 
 * Errors return via ERR_PTR().
 */

void* callback_replace(callback_replacer replacer, void* callback,
    void* key);


/*
 * Remove key->callback mapping.
 *
 * If key was not registered, return 1.
 */

int callback_replacement_clean(callback_replacer replacer,
    void* key);

/*
 * This function should be called by the replacer-user for notify
 * replacer, that its "callback_replace" and "callback_replacement_clean"
 * functions may be used by this user in the future.
 * 
 * After the first call of this function, payloads set become fixed,
 * and try_module_get is called for modules of this payloads.
 */

void callback_target_load_callback(callback_replacer replacer,
    struct module *m);

/*
 * This function should be called by the replacer-user for notify
 * replacer, that its "callback_replace" and "callback_replacement_clean"
 * functions may not be used by him in the future.
 * 
 * After the last call of this function, payloads set again become changable,
 * and module_put is called for modules of all payload which was
 * used for replacements.
 * 
 * Also, at the last call of this functions all mappings key->callback
 * will be deleted, and caller will be notify about every such key
 * via 'undeleted_key' callback (if it is not NULL).
 */

void callback_target_unload_callback(callback_replacer replacer,
    struct module *m, void undeleted_key(void* key));

/*
 * Register payload which declare replacement for callback.
 *
 * It is acceptable for this callback to not call original one.
 * 
 * But at most one such callback may be registered.
 * 
 * For get original callback inside replacing one, use
 * callback_get_orig().
 * 
 * When replacer is in use, registering of new payloads will always fail.
 */
int callback_payload_register(callback_replacer replacer,
    struct callback_payload* payload);

/*
 * Deregister payload, registered with callback_payload_register().
 * When replacer is in use, deregistering of the payloads will always fail.
 */

int callback_payload_unregister(callback_replacer replacer,
    struct callback_payload* payload);

/*
 * Return callback wich was replaced with new ones.
 * 
 * Intended to call from callback's replacement.
 */

void* callback_get_orig(callback_replacer replacer,
    void* key);

/*
 * NOTE: next 3 functions for special payloads are not implemented yet.
 */

/*
 * Register new payload which declare new callback
 * for replace.
 *
 * For this type of registering, new callback should call original one.
 * 
 * But number of such payloads is not limited.
 * 
 * For get original callback inside replacing one, use
 * callback_get_orig_special().
 * 
 * When replacer is in use, registering of new payloads will always fail.
 * 
 * This type of payloads registration is suitable for implementation of
 * interconnections between replacers.
 */

int callback_payload_register_special(callback_replacer replacer,
    struct callback_payload* payload);

/*
 * Deregister payload, registered with callback_payload_register_special().
 * When replacer is in use, deregistering of the payloads will always fail.
 */

int callback_payload_unregister_special(callback_replacer replacer,
    struct callback_payload* payload);

/*
 * Return callback with was replaced with new one.
 * 
 * Intended to call from callback's replacements.
 */

void* callback_get_orig_special(callback_replacer replacer,
    void* key,
    struct callback_payload* payload);

#endif /* CALLBACK_REPLACER_H */
