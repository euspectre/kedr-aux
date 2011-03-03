#include "callback_replacer.h"

#include "data_map.h"

#include <linux/types.h>
#include <linux/module.h>

#include <linux/mutex.h>

struct callback_replacer
{
    data_map_t callback_map;
    //NULL in this field mean that no payload is registered.
    struct callback_payload* payload;
    //
    int used;
    //Not 0 if payload is registered and succressfully fixed on target load,
    //Otherwise, it is 0.
    int payload_used;
    // Protect payload and payload_used fields from concurrent access.
    struct mutex m;
};

/*struct callback_map_data
{
    void* callback_orig;
};*/
/*
 * Create callback replacer.
 */

callback_replacer callback_replacer_create(size_t elem_num)
{
    callback_replacer replacer = kmalloc(sizeof(*replacer), GFP_KERNEL);
    if(replacer == NULL)
    {
        pr_err("Cannot allocate callback replacer.");
        return NULL;
    }
    replacer->callback_map = data_map_create(elem_num);
    if(replacer->callback_map == NULL)
    {
        pr_err("Cannot create data map for callback replacer.");
        kfree(replacer);
        return NULL;
    }
    
    replacer->payload = NULL;
    replacer->payload_used = 0;
    
    replacer->used = 0;
    mutex_init(&replacer->m);
    return replacer;
}

/*
 * Destroy replacer and free all resources it use.
 */

void callback_replacer_destroy(callback_replacer replacer)
{
    BUG_ON(replacer->used != 0);
    
    if(replacer->payload)
    {
        pr_err("Payload %p for callback replacer %p wasn't unregistered normally. Unregister it now.",
            replacer->payload, replacer);
        replacer->payload = 0;
    }
    mutex_destroy(&m);
    data_map_destroy(replacer->callback_map);
    kfree(replacer);
}

/*
 * Replace callback.
 * 
 * Return pointer to the replacement callback.
 * 
 * Errors return via ERR_PTR().
 */

void* callback_replace(callback_replacer replacer, void* callback,
    void* key)
{
    void* result;
    int result_int;
    
    BUG_ON(replacer->used == 0);
    
    result_int = data_map_add(replacer->callback_map, key, callback);
    if(result_int) return ERR_PTR(result_int);
    
    //if(!replacer->payload_used)
    //    pr_info("callback_replace: no payload is registered.");
    
    result = replacer->payload_used ? replacer->payload->callback : callback;
    
    //pr_info("callback_replace: Replace callback %p with %p.", callback, result);
    
    return result;
}


/*
 * Remove key->callback mapping.
 *
 * If key was not registered, return 1.
 */

int callback_replacement_clean(callback_replacer replacer,
    void* key)
{
    void* data;
    
    BUG_ON(replacer->used == 0);
    
    data = data_map_delete(replacer->callback_map, key);
    return IS_ERR(data) ? PTR_ERR(data) : 0;
}

/*
 * This function should be called by the replacer-user for notify
 * replacer, that its "callback_replace" and "callback_replacement_clean"
 * functions may be used by this user in the future.
 * 
 * After the first call of this function, payloads set become fixed,
 * and try_module_get is called for modules of this payloads.
 */

void callback_target_load_callback(callback_replacer replacer,
    struct module *m)
{
    replacer->used++;
    if(replacer->used != 1) return;
    
    mutex_lock(&replacer->m);
    if((replacer->payload != NULL)
        && ((replacer->payload->m == NULL)
            || (try_module_get(replacer->payload->m) != 0)))
    {
        replacer->payload_used = 1;
        //pr_info("callback_target_load_callback: payload become used.");
    }
    else
    {
        replacer->payload_used = 0;
    }
    mutex_unlock(&replacer->m);
    
    if(replacer->payload_used && replacer->payload->target_load_callback)
    {
        replacer->payload->target_load_callback(m);
    }
}

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
    struct module *m, void undeleted_key(void* key))
{
    replacer->used--;
    if(replacer->used != 0) return;
    
    if(replacer->payload_used && replacer->payload->target_unload_callback)
    {
        replacer->payload->target_unload_callback(m);
    }

    mutex_lock(&replacer->m);
    if(replacer->payload_used)
    {
        if(replacer->payload->m != NULL)
            module_put(replacer->payload->m);
        replacer->payload_used = 0;
    }
    mutex_unlock(&replacer->m);
}

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
    struct callback_payload* payload)
{
    int result = 0;
    mutex_lock(&replacer->m);
    if(replacer->used)
    {
        pr_info("Attempt to register payload while replacer is in using.");
        result = -EBUSY;
        goto out;
    }
    if(replacer->payload)
    {
        pr_info("Attempt to register payload while another payload is registered for the same callback.");
        result = -EBUSY;
        goto out;
    }
    replacer->payload = payload;
out:
    mutex_unlock(&replacer->m);
    return result;
}

/*
 * Deregister payload, registered with callback_payload_register().
 * When replacer is in use, deregistering of the payloads will always fail.
 */

int callback_payload_unregister(callback_replacer replacer,
    struct callback_payload* payload)
{
    int result = 0;
    mutex_lock(&replacer->m);
    if(replacer->payload_used)
    {
        pr_info("Attempt to unregister payload while it is in using.");
        result = -EBUSY;
        goto out;
    }
    if(replacer->payload != payload)
    {
        pr_info("Attempt to unregister payload which wasn't registered.");
        result = -EINVAL;
        goto out;
    }
    replacer->payload = NULL;
out:
    mutex_unlock(&replacer->m);
    return result;
}

/*
 * Return callback wich was replaced with new ones.
 * 
 * Intended to call from callback's replacement.
 */

void* callback_get_orig(callback_replacer replacer,
    void* key)
{
    void* callback_orig;
    
    BUG_ON(replacer->used == 0);
    
    callback_orig = data_map_get(replacer->callback_map, key);
    BUG_ON(callback_orig == NULL);
    
    return callback_orig;
}

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
