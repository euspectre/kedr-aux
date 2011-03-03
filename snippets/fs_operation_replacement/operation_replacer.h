#ifndef OPERATION_REPLACER_H
#define OPERATION_REPLACER_H

#include <linux/types.h>
#include <linux/module.h>

/*
 * Operations replacer is an object which represent interfaces
 * for replace particular type of operations for objects.
 * 
 * Using such replacer one can intercept call of such operations for
 * every object, for which replacer is registered.
 * 
 * Standard steps for implement replacer for particular type of operations
 * of some objects:
 * 
 * 1. Determine object and operations for these objects, which one want
 * to intercept. Determine type of replacer which should be used for
 * these purpose:
 * -If object contain pointer to structure, which contain operations
 * for these object, then one need to use standard replacer, which
 * will replace pointer for replace operations.
 * 		operation_replacer_create()
 * -If object containt operations itself, then one need to use replacer
 * "at place", which will replace operations in the object.
 * 		operation_replacer_create_at_place()
 * 
 * 2. Determine, when replacer should perform operations replacement for
 * the object. Usually, it should be performed after object is created
 * and before its operations may be used externally. 
 * Then one need to insert
 * 		operation_replace()
 * call to this point.
 * Also determine, when replacer should restore operations for the object.
 * Usually it should be performed after object's operations become unavailable
 * for use externally and before object is destroyed or may be reused.
 * Then insert
 * 		operation_restore()
 * call to this point.
 * 
 * Usually, for insertion of this calls one need to intercept either global
 * function via registering kedr_payload or operation of the another object.
 * In both cases on should create payload, which should contain funcions/
 * operations needed to intercepted and callbacks 'target_load_callback'
 * and 'target_unload_callback', which should call replacer's
 * 		operation_replacer_target_load_callback()
 * 		operation_replacer_target_unload_callback()
 * (Creation of the payload for intercept operations will be described lately.)
 * 
 * 3. Determine, which operations should be replaced, and with which ones.
 * Create operation_payload, and set next field in it:
 * 	.m - module, which contain code of the new operations
 * 	.repl - pointer to the operation structure, which contain new operations,
 *  .mask - pointer to the operations structure, which contain REPLACEMENT_MASK
 * for each operation, which should be replaced. Other fields should be zeroed.
 * Optionally, fields
 * 	.target_load_callback and
 *  .target_unload_callback
 * may be set for get notifications, when target module is loaded and unloaded.
 * 
 * 4. New operations, for call original ones, should use
 * 		operation_get_orig()
 * 
 *   	
 */

struct operation_payload
{
	struct module *m;
	void* repl;
	void* mask;
	void (*target_load_callback)(struct module* m);
	void (*target_unload_callback)(struct module* m);
};

// For use in mask.
#define REPLACEMENT_MASK ((void*)-1)

typedef struct operation_replacer* operation_replacer;


/*
 * Create operations replacer, which replace operations by pointer.
 */

operation_replacer operation_replacer_create(size_t elem_num,
		size_t ops_size);

/*
 * Create operations replacer, which replace operations at place.
 */

operation_replacer operation_replacer_create_at_place(size_t elem_num,
		size_t ops_size);

/*
 * Destroy replacer and free all resources it use.
 */

void operation_replacer_destroy(operation_replacer replacer);

/*
 * Replace operations.
 * For replacer by pointer key should be pointer to the pointer to the operations,
 * for replacer at place key should be pointer to operations.
 */

int operation_replace(operation_replacer replacer,
	void* key);

/*
 * Remove key->operations mapping
 * and revert replaced opertaions.
 *
 * If key was not registered, return 1.
 */

int operation_restore(operation_replacer replacer,
	void* key);


/*
 * Remove key->operations mapping
 * but without reverting replaced operations.
 * 
 * This function may be usefull for cases when it is possible
 * that key already freed at this stage, so reverting replaced operations
 * may cause SIGFAULT.
 * 
 * Note, that something similar to this functions is automatically
 * called for all registered keys when target module is unloaded.
 * But in that case warnings may appear in system log to indicate
 * that such situations are not normal.
 *
 * Using this function directly do not generate any warnings.
 * 
 * If key was not registered, return 1.
 */

int operation_replacement_clean(operation_replacer replacer,
	void* key);

/*
 * If key hasn't registered, replace operations for it.
 * 
 * If key already registered, compare current operations with
 * thouse which was set.
 * 
 * If operations changed, delete old replacement and create new.
 * Otherwise do nothing.
 */

int operation_replacement_update(operation_replacer replacer,
	void* key);


/*
 * This function should be called by the replacer-user for notify
 * replacer, that its "operation_replace" and "operations_restore"
 * functions may be used by this user in the future.
 * 
 * After the first call of this function, payloads set become fixed,
 * and try_module_get is called for modules of this payloads.
 */

void operation_target_load_callback(operation_replacer replacer,
	struct module *m);

/*
 * This function should be called by the replacer-user for notify
 * replacer, that its "operation_replace" and "operations_restore"
 * functions may not be used by him in the future.
 * 
 * After the last call of this function, payloads set again become changable,
 * and module_put is called for modules of all payload which was
 * used for replacements.
 * 
 * Also, at the last call of this functions all mappings key->operations
 * will be deleted, and caller will be notify about every such key
 * via 'undeleted_key' callback (if it is not NULL).
 */

void operation_target_unload_callback(operation_replacer replacer,
	struct module *m, void undeleted_key(void* key));

/*
 * Register new operation payload which declare new operations
 * for replace.
 *
 * It is acceptable that these new operations doesn't call original ones.
 * 
 * But set of operations to replace should not intercept with one of EVERY
 * other payload registered.
 * 
 * For get original operations inside replacing one, use
 * operation_get_orig().
 * 
 * When replacer is in use, registering of new payloads will always fail.
 */
int operation_payload_register(operation_replacer replacer,
	struct operation_payload* payload);

/*
 * Deregister payload, registered with operation_payload_register().
 * When replacer is in use, deregistering of the payloads will always fail.
 */

int operation_payload_unregister(operation_replacer replacer,
	struct operation_payload* payload);

/*
 * Return operation wich was replaced with new one.
 * 
 * Intended to call from the replacement operation.
 */

void* operation_get_orig(operation_replacer replacer,
	int operation_offset,
	void* key);

/*
 * Register new operation payload which declare new operations
 * for replace.
 *
 * For this type of registering, new operations should call original ones.
 * 
 * But available set of operations to replace is do not depend from
 * other payload registered - payload may replace any operations.
 * 
 * For get original operation inside replacing one, use
 * operation_get_orig_special().
 * 
 * When replacer is in use, registering of new payloads will always fail.
 * 
 * This type of payloads registration is suitable for implementation of
 * interconnections between replacers.
 */

int operation_payload_register_special(operation_replacer replacer,
	struct operation_payload* payload);

/*
 * Deregister payload, registered with operation_payload_register_special().
 * When replacer is in use, deregistering of the payloads will always fail.
 */

int operation_payload_unregister_special(operation_replacer replacer,
	struct operation_payload* payload);

/*
 * Return operation wich was replaced with new one.
 * 
 * Intended to call from the replacement operation.
 */

void* operation_get_orig_special(operation_replacer replacer,
	int operation_offset,
	void* key,
	struct operation_payload* payload);

#endif