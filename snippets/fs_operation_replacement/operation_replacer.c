#include "operation_replacer.h"

#include <linux/list.h> /* several lists */

#include <linux/slab.h> /* kmalloc */
#include <linux/mutex.h> /*mutex*/

#include "data_map.h"

// Helpers for 'binary' operations on the operations structures.

/* 
 * For different reasons simple char-by-char operations
 * are not always sufficient.
 * 
 * We assume, that operations structures are really structures,
 * and operations pointers - are its fields.
 * So, operations pointers should be correctly aligned inside structures.
 */ 

//Werify, if address is correctly aligned for being pointer to the operation.
// Not used now.
#define IS_ALIGN_OPERATION(addr) ((addr) % __alignof__(void*))

/*
 * Return pointer to the first masked operation.
 * 
 * If mask is empty, return NULL.
 */

static inline void* const*
first_operation_in_mask(const void* mask, size_t ops_size)
{
	void* const* op;
	for(op = (void* const*)(mask); (const char*)op < ((const char*)mask + ops_size); ++op)
		if(*op != NULL) return op;
	return NULL;
}

/*
 * Return pointer to the next masked operation.
 * 
 * If current pointer is the last masked one, return NULL.
 */

static inline void* const*
next_operation_in_mask(const void* mask, size_t ops_size, void* const* op)
{
	for(++op; (const char*)op < ((const char*)mask + ops_size); ++op)
		if(*op != NULL) return op;
	return NULL;
}

/*
 * Iterator for masked operations.
 */
#define FOR_EACH_OPERATION_IN_MASK(op, mask, ops_size) \
for((op) = first_operation_in_mask(mask, ops_size); (op) != NULL; (op) = next_operation_in_mask(mask, ops_size, op))

/*
 * Return pointer to the operation in 'src',
 * corresponded to one in the mask.
 */
static inline void**
corresponded_op(void* src, const void* mask, void* const* mask_op)
{
	return (void**)(src + ((const char*)(mask_op) - (const char*)(mask)));
}

/*
 * Return constant pointer to the operation in 'src',
 * corresponded to one in the mask.
 */
static inline void* const*
corresponded_op_const(const void* src, const void* mask, void* const* mask_op)
{
	return (void* const*)(src + ((const char*)(mask_op) - (const char*)(mask)));
}


/*
 * Structure for storing registered payloads.
 */

struct operation_payload_list
{
	struct list_head list;
	struct list_head list_used;// this list is fixed(!) all time while target loaded
	int is_used;
	struct operation_payload* payload;
};

/*
 * Types of operation replacer.
 * 
 * Determines, how real replacement will be performed.
 */

enum operation_replacer_type
{
	operation_replacer_type_replace_pointer,
	operation_replacer_type_at_place,
};

/*
 * Information about one layer of the operation replacement.
 */

struct operation_replacer_data_layer
{
	// Special payload for this layer.
	struct operation_payload* payload;
	/*
	 *  Mask operations, which is replaced by payload of one
	 * of the previous layers(or by any of non-special payload).
	 */
	char* ops_mask_prev;
	/*
	 *  For each masked operation contains replacement operation
     * which is introduced by payload in the nearest layer before this.
	 */
	char* ops_repl_prev;
};

/*
 *  Contains all information about replacement-original operations
 * interconnections and used for replace operations for particular object.
 */
struct operation_replacer_data
{
	/*
     *  Number of intermediate layers of replacement
	 * Each layer corresponds to one special payload.
     */
	int n_repl_layers;

	/*
     *  This replacement and mask will be used for 
	 * immediate replacement of original operations.
     */
	char* ops_repl_total;
	char* ops_mask_total;
	// Per-layer data.
	struct operation_replacer_data_layer* layers;
};

/*
 * Operation replacer's description.
 */

struct operation_replacer
{
	// List of standard payloads.
    struct list_head payloads;
    // List of special payloads.
	struct list_head payloads_special;
	
    /*
     *  Mask of standard payloads for reject registering payload
     * which mask is intercepted with others.
     */
	char* total_mask;

	/*
	 * Protect from concurrent access to 'payloads', 'payloads_special'
     * and 'total_mask'.
	 */
	struct mutex m;

	/*
	 * Count calls of target_load_callback and target_unload_callback.
	 * 
	 * Note: this counter is correct even in case of concurrent callback calls,
	 * but this usage of replacer is nevetheless forbidden.
	 */
	atomic_t n_uses;

	/*
	 * Prevent self-references while chaining target_load/target_unload callbacks.
	 * 
	 * Note: this protection works only with serializing callbacks calls.
	 * Atomic type of variable will not help in case of concurrent callback calls.
	 */
	int inside_callback;

    // Size of operations structure with which this replacer work.
    size_t ops_size;
    // Type of the replacer.
	int type;
    /*
     *  Payloads which used for replace operations
     * (standard and special ones).
     */
	struct list_head payloads_used;
	struct list_head payloads_special_used;

	// Common data about operations replacements.
	struct operation_replacer_data* data;
  	// Contains information about operations replacements for particuar object.
    data_map_t data_map;

};

/*
 * Return pointer to operations according to key for 'replace_pointer'
 * type of replacer.
 */

static inline const void**
ops_from_key_replace_pointer(void* key)
{
	const void** result = (const void**)key;
	return result;
}


/*
 * Data structure for 'replace_pointer' type of replacer.
 */

struct operation_data_replace_pointer
{
	const char* ops_orig;
	char ops[0];//real size - ops_size
};

/*
 * Size of the data structure of 'replace_pointer' replacer.
 */

static inline size_t
operation_data_replace_pointer_size(size_t ops_size)
{
	return sizeof(struct operation_data_replace_pointer)
		+ ops_size;
}

/*
 * Data structure for the replacer of "at_place" type.
 */

struct operation_data_at_place
{
	char* ops;
	char ops_orig[0];// real size - ops_size
};

/*
 * Size of the data structure for replacer of "at_place" type.
 */

static inline size_t
operation_data_at_place_size(size_t ops_size)
{
	return sizeof(struct operation_data_at_place)
		+ ops_size;
}

/*
 *  Helper, return operation with given offset
 * in the operations structure.
 */

static inline void*
get_op_at_offset(const void* ops, int op_offset)
{
    return *(void* const*)((const char*)ops + op_offset);
}

/*
 * Allocate replacer structure and fill common data of it.
 */

static operation_replacer
operation_replacer_create_common(size_t elem_num, size_t ops_size)
{
	operation_replacer replacer =
		kmalloc(sizeof(*replacer), GFP_KERNEL);
	if(replacer == NULL)
	{
		pr_err("operation_replacer_create_common: Cannot allocate memory for replacer.");
		return NULL;
	}
	
	replacer->data_map = data_map_create(elem_num);
	if(replacer->data_map == NULL)
	{
		pr_err("operation_replacer_create_common: Cannot create data map for replacer.");
		kfree(replacer);
		return NULL;
	}
    
    replacer->total_mask = kmalloc(ops_size, GFP_KERNEL);
    if(replacer->total_mask == NULL)
    {
        pr_err("operation_replacer_create_common: Cannot allocate replacement mask for non-special payloads.");

        data_map_destroy(replacer->data_map);
        kfree(replacer);

        return NULL;
    }
    memset(replacer->total_mask, 0, ops_size);
    
	INIT_LIST_HEAD(&replacer->payloads);
	INIT_LIST_HEAD(&replacer->payloads_special);
	atomic_set(&replacer->n_uses, 0);
	replacer->inside_callback = 0;

    replacer->data = NULL;
	mutex_init(&replacer->m);
	
	replacer->ops_size = ops_size;
	
	return replacer;
}

operation_replacer operation_replacer_create(size_t elem_num,
		size_t ops_size)
{
	operation_replacer replacer =
		operation_replacer_create_common(elem_num, ops_size);
	if(replacer == NULL) return NULL;

	replacer->type = operation_replacer_type_replace_pointer;

	return replacer;
}

operation_replacer operation_replacer_create_at_place(size_t elem_num,
		size_t ops_size)
{
	operation_replacer replacer =
		operation_replacer_create_common(elem_num, ops_size);
	if(replacer == NULL) return NULL;

	replacer->type = operation_replacer_type_at_place;
	
	return replacer;
}


/*
 * Callback for data_map_delete_all for replacer's data.
 */

struct undeleted_key_callback_data
{
	void (*callback)(void* key);
};
static void free_data(void* data, void* key, void* user_data)
{
	struct undeleted_key_callback_data* user_data_real = user_data;
	kfree(data);
	if(user_data_real && user_data_real->callback)
		user_data_real->callback(key);
}

/*
 * Unregister payloads at replacer's destroying stage from given list.
 * May be used both for standard payloads and for special ones.
 */

static void unregister_payloads_at_delete(operation_replacer replacer,
	struct list_head* payloads)
{
	while(!list_empty(payloads))
	{
		struct operation_payload_list* payload_elem =
			list_first_entry(payloads,
				struct operation_payload_list, list);
		pr_warning("unregister_payloads_at_delete: Payload %p of module %s is not unregistered. Unregister it now.",
			payload_elem->payload, module_name(payload_elem->payload->m));
		list_del(&payload_elem->list);
		kfree(payload_elem);
	}
}

void operation_replacer_destroy(operation_replacer replacer)
{
	BUG_ON(atomic_read(&replacer->n_uses) != 0);
	// Unregister payloads if they are
	unregister_payloads_at_delete(replacer, &replacer->payloads);
	unregister_payloads_at_delete(replacer, &replacer->payloads_special);

	kfree(replacer->total_mask);

	mutex_destroy(&replacer->m);
	// Map should be empty at this stage.
	data_map_destroy(replacer->data_map);
	kfree(replacer);
}

/*
 * Fill 'data' and perform operations replacement for given key.
 * ("replace_pointer" replacer.)
 */

static int operation_replace_replace_pointer(void* data,
	void* key,
	const void* repl,
	const void* mask,
	size_t ops_size)
{
	struct operation_data_replace_pointer* data_real = data;
    void* const* op_mask;
    
	data_real->ops_orig = *ops_from_key_replace_pointer(key);
	
	memcpy(data_real->ops, data_real->ops_orig, ops_size);
    
	FOR_EACH_OPERATION_IN_MASK(op_mask, mask, ops_size)
    {
        *corresponded_op(data_real->ops, mask, op_mask) =
            *corresponded_op_const(repl, mask, op_mask);
    }
	*ops_from_key_replace_pointer(key) = data_real->ops;
	return 0;
}

/*
 * Return pointer to the original operations according to the data.
 * ("replace_pointer" replacer.)
 */

static inline void*
operation_get_orig_replace_pointer(void* data, size_t op_offset,
	size_t ops_size)
{
	struct operation_data_replace_pointer* data_real = data;
    return get_op_at_offset(data_real->ops_orig, op_offset);
}

/*
 * Restore operations for the key according to the data.
 * ("replace_pointer" replacer.)
 */

static int operation_restore_replace_pointer(void* data,
	void* key,
	const void* repl,
	const void* mask,
	size_t ops_size)
{
	struct operation_data_replace_pointer* data_real = data;
	
	if(*ops_from_key_replace_pointer(key) == data_real->ops)
	{
		*ops_from_key_replace_pointer(key) =
			data_real->ops_orig;
	}
	else
	{
		// Operations was changed from ones, set by replacer, to anothers.
		pr_info("operation_restore_replace_pointer: "
			"Pointer to operations for key %p was changed outside of the replacer.",
			key);
	}
	return 0;
}

/*
 * Update operations replacement for the key according to the data.
 * ("replace_pointer" replacer.)
 */

static int operation_replacement_update_replace_pointer(void* data,
	void* key,
	const void* repl,
	const void* mask,
	size_t ops_size)
{
	struct operation_data_replace_pointer* data_real = data;

	if(*ops_from_key_replace_pointer(key) == data_real->ops)
	{
		// Operations are up-to-date
		return 0;
	}
	else if(*ops_from_key_replace_pointer(key) == data_real->ops_orig)
	{
		// Someone set operations to those that was before our replacement.
		*ops_from_key_replace_pointer(key) = data_real->ops;
		return 0;
	}
	else
	{
		// Operations was set to some value.
        // Simply forgot replace operation.
		return operation_replace_replace_pointer(data, key,
			repl, mask, ops_size);
	}
}


/*
 * Fill 'data' and perform operations replacement for given key.
 * ("at_place" replacer.)
 */

static int operation_replace_at_place(void* data,
	void* key,
	const void* repl,
	const void* mask,
	size_t ops_size)
{
	struct operation_data_at_place* data_real = data;
    void* const* op_mask;

	data_real->ops = key;
	memcpy(data_real->ops_orig, data_real->ops, ops_size);
    FOR_EACH_OPERATION_IN_MASK(op_mask, mask, ops_size)
    {
        *corresponded_op(data_real->ops, mask, op_mask) =
            *corresponded_op_const(repl, mask, op_mask);
    }
	
	return 0;
}

/*
 * Return pointer to the original operations according to the data.
 * ("at_place" replacer.)
 */

static inline void*
operation_get_orig_at_place(void* data, size_t op_offset,
	size_t ops_size)
{
	struct operation_data_at_place* data_real = data;
	return get_op_at_offset(data_real->ops_orig, op_offset);
}

/*
 * Restore operations for the key according to the data.
 * ("at_place" replacer.)
 */

static int operation_restore_at_place(void* data,
	void* key,
	const void* repl,
	const void* mask,
	size_t ops_size)
{
	struct operation_data_at_place* data_real = data;
    void* const* mask_op;
    
    FOR_EACH_OPERATION_IN_MASK(mask_op, mask, ops_size)
    {
        void** op = corresponded_op(data_real->ops, mask, mask_op);
        if(*op != *corresponded_op_const(repl, mask, mask_op))
        {
            pr_info("operation_restore_at_place: Operation for key %p "
                "at offset %d was changed outside of replacer.",
                key,
                (const char*)op - (const char*)data_real->ops);
            continue;
        }
        *op = *corresponded_op_const(data_real->ops_orig, mask, mask_op);
    }
	return 0;
}

/*
 * Update operations replacement for the key according to the data.
 * ("at_place" replacer.)
 */

static int operation_replacement_update_at_place(void* data,
	void* key,
	const void* repl,
	const void* mask,
	size_t ops_size)
{
	int result;
	struct operation_data_at_place* data_real = data;
	
	/*
	 * Verification, whether operations are up-to-date,
	 * take similar time as replacement of the operations.
	 * 
	 * Simple restore operations and set them again.
	 */
	result = operation_restore_at_place(data_real, key, repl, mask, ops_size);
	if(result) return result;
	
	return operation_replace_at_place(data_real, key, repl, mask, ops_size);
}


int operation_replace(operation_replacer replacer,
	void* key)
{
	int result;
	size_t data_size;
	void* data;
	
	const void *repl, *mask;
	
	struct operation_replacer_data* replacer_data = replacer->data;
	
    BUG_ON(atomic_read(&replacer->n_uses) == 0);
    
	if(replacer_data == NULL)
	{
		//replacer_data wasn't allocated when target loaded.
		//So, cannot replace operations at all.
		return -ENOMEM;
	}
	
	switch(replacer->type)
	{
	case operation_replacer_type_replace_pointer:
		data_size = operation_data_replace_pointer_size(replacer->ops_size);
		break;
	case operation_replacer_type_at_place:
		data_size = operation_data_at_place_size(replacer->ops_size);
		break;
	default:
		pr_err("operation_replace: Invalid replacer type: %d", replacer->type);
		BUG();
	}
	data = kmalloc(data_size, GFP_KERNEL);
	if(data == NULL)
	{
		pr_err("operation_replace: Cannot allocate data for replace operations.");
		return -ENOMEM;
	}

	repl = replacer_data->ops_repl_total;
	mask = replacer_data->ops_mask_total;

	switch(replacer->type)
	{
	case operation_replacer_type_replace_pointer:
		result = operation_replace_replace_pointer(data, key,
			repl, mask, replacer->ops_size);
		break;
	case operation_replacer_type_at_place:
		result = operation_replace_at_place(data, key,
			repl, mask, replacer->ops_size);
		break;
	default:
		BUG();
	}
	if(result)
	{
		kfree(data);
		pr_err("operation_replace: Fail to perform operations replacement.");
		return result;
	}
	
	result = data_map_add(replacer->data_map, key, data);
	if(result)
	{
		pr_err("operation_replace: Fail to add key for operations replacement.");
		switch(replacer->type)
		{
		case operation_replacer_type_replace_pointer:
			operation_restore_replace_pointer(data, key,
				repl, mask, replacer->ops_size);
			break;
		case operation_replacer_type_at_place:
			operation_restore_at_place(data, key,
				repl, mask, replacer->ops_size);
			break;
		default:
			BUG();
		}
		kfree(data);
		return result;
	}
	return 0;
}

int operation_restore(operation_replacer replacer,
	void* key)
{
	int result;
	
	struct operation_replacer_data* replacer_data = replacer->data;
	const void *repl, *mask;

	void* data;
    
    BUG_ON(atomic_read(&replacer->n_uses) == 0);
	
	if(replacer_data == NULL)
	{
		/*
		 * No replacement may be done in this case, but
		 * error already was reported when target is loaded.
		 * 
		 * Silently return success.
		 */
		return 0;
	}


	data = data_map_get(replacer->data_map, key);
	if(IS_ERR(data))
	{
		/*
		 * It is not an error, that key for deleting replacement is not
		 * exist.
		 * Only warning.
		 */
		pr_info("operation_restore: Attempt to restore operations at unexistent key %p.", key);
		return 1;
	}
	BUG_ON(data == NULL);
	
	repl = replacer_data->ops_repl_total;
	mask = replacer_data->ops_mask_total;
	
	switch(replacer->type)
	{
	case operation_replacer_type_replace_pointer:
		result = operation_restore_replace_pointer(data, key,
			repl, mask, replacer->ops_size);
		break;
	case operation_replacer_type_at_place:
		result = operation_restore_at_place(data, key,
			repl, mask, replacer->ops_size);
		break;
	default:
		BUG();
	}
	if(result) return result;
	
	data_map_delete(replacer->data_map, key);
	kfree(data);
	
	return 0;
}

int operation_replacement_clean(operation_replacer replacer,
	void* key)
{
	void* data;

    BUG_ON(atomic_read(&replacer->n_uses) == 0);

	if(replacer->data == NULL)
	{
		/*
		 * No replacement may be done in this case, but
		 * error already was reported when target is loaded.
		 * 
		 * Silently return success.
		 */
		return 0;
	}

	data = data_map_get(replacer->data_map, key);
	if(IS_ERR(data))
	{
		/*
		 * It is not an error, that key for deleting replacement is not
		 * exist.
		 * Only warning.
		 */
		pr_info("operation_replacement_clean: Attempt to clean operations replacement at unexistent key %p.", key);
		return 1;
	}

	BUG_ON(data == NULL);
	//without restoring the operations
	data_map_delete(replacer->data_map, key);
	kfree(data);
	
	return 0;
}

int operation_replacement_update(operation_replacer replacer,
	void* key)
{
	int result;
	void* data;
	
	const void *repl, *mask;
	
    BUG_ON(atomic_read(&replacer->n_uses) == 0);
    
	if(replacer->data == NULL)
	{
		//replacer_data wasn't allocated when target load.
		//So, cannot replace operations at all.
		return -ENOMEM;
	}

	data = data_map_get(replacer->data_map, key);
	if(IS_ERR(data))
	{
		//Simply create new replacement
		return operation_replace(replacer, key);
	}

	BUG_ON(data == NULL);

	repl = replacer->data->ops_repl_total;
	mask = replacer->data->ops_mask_total;

	switch(replacer->type)
	{
	case operation_replacer_type_replace_pointer:
		result = operation_replacement_update_replace_pointer(data, key,
			repl, mask, replacer->ops_size);
		break;
	case operation_replacer_type_at_place:
		result = operation_replacement_update_at_place(data, key,
			repl, mask, replacer->ops_size);
		break;
	default:
		pr_err("operation_replace: Invalid replacer type: %d", replacer->type);
		BUG();
	}

	if(result)
	{
		//delete key->data mapping for invalide data
		data_map_delete(replacer->data_map, key);
		kfree(data);
		return result;
	}

	return 0;
}


/*
 * Helpers for fix payload list and release it.
 */

//Should be executed under lock
static int fix_payload(operation_replacer replacer,
	struct operation_payload_list* payload_elem)
{
	BUG_ON(payload_elem->is_used);
	if(payload_elem->payload->m)
		if(try_module_get(payload_elem->payload->m) == 0)
		{
			pr_info("fix_payload: Do not use payload from module which is unloaded now.");
			return -EBUSY;
		}
	payload_elem->is_used = 1;
	return 0;
}

//Should be executed under lock
static void release_payload(operation_replacer replacer,
	struct operation_payload_list* payload_elem)
{
	BUG_ON(!payload_elem->is_used);
	if(payload_elem->payload->m)
		module_put(payload_elem->payload->m);
	payload_elem->is_used = 0;
}


static int
operation_replacer_data_layer_init(
    struct operation_replacer_data_layer* layer,
    size_t ops_size)
{
    layer->ops_mask_prev = kmalloc(ops_size, GFP_KERNEL);
    
    if(layer->ops_mask_prev == NULL)
    {
        pr_err("Cannot allocate 'mask' field for layer data.");
        return -ENOMEM;
    }
    
    layer->ops_repl_prev = kmalloc(ops_size, GFP_KERNEL);
    
    if(layer->ops_repl_prev == NULL)
    {
        pr_err("Cannot allocate 'repl' field for layer data.");
        kfree(layer->ops_mask_prev);
        return -ENOMEM;
    }
    
    return 0;
}

static void operation_replacer_data_layer_destroy(
    struct operation_replacer_data_layer* data_layer)
{
    kfree(data_layer->ops_repl_prev);
    kfree(data_layer->ops_mask_prev);
}

static struct operation_replacer_data*
operation_replacer_data_alloc(size_t ops_size, int n_repl_layers)
{
	struct operation_replacer_data* data;
    int i;

	data = kmalloc(sizeof(*data), GFP_KERNEL);

	if(data == NULL)
	{
		pr_err("operation_replacer_data_alloc: Cannot allocate replacer's cache.");
		return NULL;
	}
    
    data->n_repl_layers = n_repl_layers;
    
    data->ops_repl_total = kmalloc(ops_size, GFP_KERNEL);
    if(data->ops_repl_total == NULL)
    {
        pr_err("operation_replacer_data_alloc: Cannot allocate 'repl' field for replacer's cache.");
        goto err_alloc_repl;
    }

    data->ops_mask_total = kmalloc(ops_size, GFP_KERNEL);
    if(data->ops_mask_total == NULL)
    {
        pr_err("operation_replacer_data_alloc: Cannot allocate 'mask' field for replacer's cache.");
        goto err_alloc_mask;
    }

    data->layers = kmalloc(sizeof(*data->layers) * n_repl_layers, GFP_KERNEL);
    if(data->layers == NULL)
    {
        pr_err("operation_replacer_data_alloc: Cannot allocate 'layers' field for replacer's cache.");
        goto err_alloc_layers;
    }
    
    for(i = 0; i < n_repl_layers; i++)
    {
        int result = operation_replacer_data_layer_init(
                        &data->layers[i],
                        ops_size);
        if(result)
        {
            pr_err("operation_replacer_data_alloc: Cannot allocate layer for replacer's cache.");
            goto err_alloc_layer;
        }
    }

    return data;

err_alloc_layer:
    for(i--; i >= 0; i--)
        operation_replacer_data_layer_destroy(&data->layers[i]);
    kfree(data->layers);
err_alloc_layers:
    kfree(data->ops_mask_total);
err_alloc_mask:
    kfree(data->ops_repl_total);
err_alloc_repl:
    kfree(data);
    
    return NULL;
}

static void
operation_replacer_data_free(struct operation_replacer_data* data)
{
    int i;
    for(i = 0; i < data->n_repl_layers; i++)
        operation_replacer_data_layer_destroy(&data->layers[i]);
    kfree(data->layers);
    kfree(data->ops_mask_total);
    kfree(data->ops_repl_total);
    kfree(data);
}

//Create replacer's cache
static void operation_replacer_data_create(operation_replacer replacer)
{
	int current_layer = 0;
	struct list_head* iter;
	
	struct operation_replacer_data* data;
	
	struct operation_payload_list* payload_elem;
	
	size_t ops_size = replacer->ops_size;
	
	//count replacement layers
	list_for_each(iter, &replacer->payloads_special_used)
	{
		current_layer++;
	}
	
	data = operation_replacer_data_alloc(replacer->ops_size, current_layer);

	if(data == NULL)
	{
		pr_err("operation_replacer_data_create: Replacement facility will be disabled for this session.");
        replacer->data = NULL;//indicator
        return;
	}
    replacer->data = data;
	memset(data->ops_repl_total, 0, ops_size);
    memset(data->ops_mask_total, 0, ops_size);
	// Simple payloads
	list_for_each_entry(payload_elem, &replacer->payloads_used, list_used)
	{
		const void* add_mask = payload_elem->payload->mask;
        const void* add_repl = payload_elem->payload->repl;
        void* const* op_mask;
        //Do not verify masks' interception. It should be verified at payload's registeration.
		FOR_EACH_OPERATION_IN_MASK(op_mask, add_mask, ops_size)
        {
            // Masks interception shouldn't occure.
            BUG_ON(*corresponded_op(data->ops_mask_total, add_mask, op_mask) != NULL);

            *corresponded_op(data->ops_mask_total, add_mask, op_mask) =
                *op_mask;
            *corresponded_op(data->ops_repl_total, add_mask, op_mask) =
                *corresponded_op_const(add_repl, add_mask, op_mask);
        }
	}
	
	if(data->n_repl_layers == 0) return;
	
	// Special payloads
	current_layer = 0;
	list_for_each_entry(payload_elem, &replacer->payloads_special_used, list_used)
	{
		void* const* op_mask;
        const void* add_mask = payload_elem->payload->mask;
        const void* add_repl = payload_elem->payload->repl;
        struct operation_replacer_data_layer* data_layer = &data->layers[current_layer];
        //debug
        //if(current_layer == 0) {current_layer ++; continue;}
        
        //Set payload for this layer
        data_layer->payload = payload_elem->payload;
        //Copy replacements and mask from 'total' ones
        memcpy(data_layer->ops_repl_prev,
            data->ops_repl_total,
            ops_size);
        memcpy(data_layer->ops_mask_prev,
            data->ops_mask_total,
            ops_size);
        //Update total replacements and mask
        FOR_EACH_OPERATION_IN_MASK(op_mask, add_mask, ops_size)
        {
            *corresponded_op(data->ops_mask_total, add_mask, op_mask) =
                *op_mask;
            *corresponded_op(data->ops_repl_total, add_mask, op_mask) =
                *corresponded_op_const(add_repl, add_mask, op_mask);
        }
		current_layer++;
	}
}

void operation_target_load_callback(operation_replacer replacer,
	struct module *m)
{
	struct operation_payload_list* payload_elem;
	
	if(replacer->inside_callback)
	{
		/*
		 * Callback was (indirectly) called from itself.
		 * Break this call chain.
		 */
		return;
	}
	replacer->inside_callback = 1;
	if(atomic_inc_return(&replacer->n_uses) != 1)
	{
		// Not first using. Ignore it.
		goto out;
	}
	
	/*
	 * First iteration: block payloads.
	 */

	//should be 'killable' in the future
	mutex_lock(&replacer->m);
	
	INIT_LIST_HEAD(&replacer->payloads_used);
	list_for_each_entry(payload_elem, &replacer->payloads, list)
	{
		if(fix_payload(replacer, payload_elem)) continue;

		list_add_tail(&payload_elem->list_used, &replacer->payloads_used);
	}

	INIT_LIST_HEAD(&replacer->payloads_special_used);

	list_for_each_entry(payload_elem, &replacer->payloads_special, list)
	{
		if(fix_payload(replacer, payload_elem)) continue;

		list_add_tail(&payload_elem->list_used, &replacer->payloads_special_used);
	}
	mutex_unlock(&replacer->m);
	
    /*
     * Second iteration: prepare replacer's cache.
     */

    operation_replacer_data_create(replacer);

	/*
	 * Third iteration: call target_load_callback's of all used payloads.
	 * 
	 * It may be done without lock with using 'list_used' lists.
	 * 
	 * Note, that even we drop mutex, there is no possibility to
	 * using replacements or to unload this replacer -
	 * this should be enforced by the caller.
	 */
	list_for_each_entry(payload_elem, &replacer->payloads_used, list_used)
	{
		if(payload_elem->payload->target_load_callback)
			payload_elem->payload->target_load_callback(m);
	}
	list_for_each_entry(payload_elem, &replacer->payloads_special_used, list_used)
	{
		if(payload_elem->payload->target_load_callback)
			payload_elem->payload->target_load_callback(m);
	}
	
out:
	replacer->inside_callback = 0;
	
}

void operation_target_unload_callback(operation_replacer replacer,
	struct module *m, void undeleted_key(void* key))
{
	struct operation_payload_list* payload_elem;
	struct undeleted_key_callback_data undeleted_key_data;

	if(replacer->inside_callback)
	{
		/*
		 * Callback was (indirectly) called from itself.
		 * Break this call chain.
		 */
		return;
	}
	replacer->inside_callback = 1;

	if(atomic_dec_return(&replacer->n_uses) != 0)
	{
		// Not last using. Ignore it.
		goto out;
	}

	// For case if some replacement wasn't been restored.
	undeleted_key_data.callback = undeleted_key;
	data_map_delete_all(replacer->data_map, free_data, &undeleted_key_data);


	/*
	 * First iteration: call target_unload_callback's of all used payloads.
	 * 
	 * It may be done without lock with using 'list_used' lists.
	 * 
	 * Note, that even we drop mutex, there is no possibility to
	 * load this replacer again - this should be enforced by the caller.
	 */

	list_for_each_entry_reverse(payload_elem, &replacer->payloads_used, list_used)
	{
		if(payload_elem->payload->target_unload_callback)
			payload_elem->payload->target_unload_callback(m);
	}
	list_for_each_entry_reverse(payload_elem, &replacer->payloads_special_used, list_used)
	{
		if(payload_elem->payload->target_unload_callback)
			payload_elem->payload->target_unload_callback(m);
	}

    /*
     * Second: free cache.
     */

    if(replacer->data)
        operation_replacer_data_free(replacer->data);

	/*
	 * Third iteration: clear undeleted replacements
	 * and unblock payloads.
	 */
	
	//should be 'killable' in the future
	mutex_lock(&replacer->m);
	
	list_for_each_entry(payload_elem, &replacer->payloads_used, list_used)
	{
		release_payload(replacer, payload_elem);
	}
	
	list_for_each_entry(payload_elem, &replacer->payloads_special_used, list_used)
	{
		release_payload(replacer, payload_elem);
	}
	mutex_unlock(&replacer->m);
out:
	replacer->inside_callback = 0;
}

int operation_payload_register(operation_replacer replacer,
	struct operation_payload* payload)
{
	void* const* op_mask;
    const void* add_mask = payload->mask;
    struct operation_payload_list* payload_elem;
	int result = 0;

	if(atomic_read(&replacer->n_uses))
	{
		/*
		 * Really, implementation doesn't need to enforce this restriction
		 * to work correctly, but this is useful for do not mislead user.
		 * 
		 * (also, it is a 'weak' verification).
		 */
		pr_err("operation_payload_register: Attempt to register payload while target is being loaded.");
		return -EBUSY;
	}

	if(mutex_lock_killable(&replacer->m))
	{
		return -EINTR;
	}

    // Verify, that no operation will intercept twice.
    FOR_EACH_OPERATION_IN_MASK(op_mask, add_mask, replacer->ops_size)
    {
        if(*corresponded_op(replacer->total_mask, add_mask, op_mask) != NULL)
        {
            pr_err("operation_payload_register: Cannot register "
                "non-special payload which replace operation "
                "which already has replaced by other payload.");
            result = -EINVAL;
            goto out;
        }

    }

	payload_elem = kmalloc(sizeof(*payload_elem), GFP_KERNEL);
	if(payload_elem == NULL)
	{
		pr_err("operation_payload_register: Fail to allocate new payload elem.");
		result = -ENOMEM;
		goto out;
	}
	payload_elem->payload = payload;
	payload_elem->is_used = 0;
	
	list_add(&payload_elem->list, &replacer->payloads);
    FOR_EACH_OPERATION_IN_MASK(op_mask, add_mask, replacer->ops_size)
    {
        *corresponded_op(replacer->total_mask, add_mask, op_mask) =
            *op_mask;
    }

out:
	mutex_unlock(&replacer->m);
	return result;
}


int operation_payload_unregister(operation_replacer replacer,
	struct operation_payload* payload)
{
	struct operation_payload_list* payload_elem;
	int result = 0;
	if(mutex_lock_killable(&replacer->m))
	{
		return -EINTR;
	}
	list_for_each_entry(payload_elem, &replacer->payloads, list)
	{
		if(payload_elem->payload == payload)
		{
			void* const* op_mask;
            const void* rem_mask = payload->mask;
            if(payload_elem->is_used)
			{
				pr_err("operation_payload_unregister: Attempt to unregister payload while it in use.");
				result = -EBUSY;
				goto out;
			}
			list_del(&payload_elem->list);
            FOR_EACH_OPERATION_IN_MASK(op_mask, rem_mask, replacer->ops_size)
            {
                void** op = corresponded_op(replacer->total_mask,
                    rem_mask, op_mask);
                BUG_ON(*op == NULL);
                *op = NULL;
            }
			kfree(payload_elem);
			goto out;
		}
	}
	pr_err("operation_payload_unregister: Attempt to unregister payload which wasn't be registered.");
	result = -EINVAL;

out:
	mutex_unlock(&replacer->m);
	return result;
}

void* operation_get_orig(operation_replacer replacer, int operation_offset,
	void* key)
{
	void* result;
	void* data;
    
    BUG_ON(atomic_read(&replacer->n_uses) == 0);
    BUG_ON(replacer->data == NULL);
    BUG_ON(get_op_at_offset(replacer->data->ops_mask_total,
        operation_offset) == NULL);

    data = data_map_get(replacer->data_map, key);
	
	/*
	 * Unexistent key - error or warning?
	 * Now error.
	 */
	BUG_ON(IS_ERR(data));
	BUG_ON(data == NULL);
	
	switch(replacer->type)
	{
	case operation_replacer_type_replace_pointer:
		result = operation_get_orig_replace_pointer(data,
            operation_offset,
			replacer->ops_size);
	break;
	case operation_replacer_type_at_place:
		result = operation_get_orig_at_place(data,
            operation_offset,
			replacer->ops_size);
	break;
	default:
		BUG();
	}
	return result;
}


int operation_payload_register_special(operation_replacer replacer,
	struct operation_payload* payload)
{
	struct operation_payload_list* payload_elem;
	int result = 0;

	if(atomic_read(&replacer->n_uses))
	{
		/*
		 * Really, implementation doesn't need to enforce this restriction
		 * to work correctly, but this is useful for do not mislead user.
		 * 
		 * (also, it is a 'weak' verification).
		 */
		pr_err("operation_payload_register_special: Attempt to register payload while target is being loaded.");
		return -EBUSY;
	}

	if(mutex_lock_killable(&replacer->m))
	{
		return -EINTR;
	}

	payload_elem = kmalloc(sizeof(*payload_elem), GFP_KERNEL);
	if(payload_elem == NULL)
	{
		pr_err("operation_payload_register_special: Fail to allocate new payload elem.");
		result = -ENOMEM;
		goto out;
	}
    //debug
    //if(!list_empty(&replacer->payloads_special))
    //{
        //pr_err("(test)No more than 1 special payload is allowed.");
        //result = -EINVAL;
        //goto out;
    //}
    
	payload_elem->payload = payload;
	payload_elem->is_used = 0;
	
	list_add(&payload_elem->list, &replacer->payloads_special);
out:
	mutex_unlock(&replacer->m);
	return result;
}

int operation_payload_unregister_special(operation_replacer replacer,
	struct operation_payload* payload)
{
	struct operation_payload_list* payload_elem;
	int result = 0;
	if(mutex_lock_killable(&replacer->m))
	{
		return -EINTR;
	}
	list_for_each_entry(payload_elem, &replacer->payloads_special, list)
	{
		if(payload_elem->payload == payload)
		{
			if(payload_elem->is_used)
			{
				pr_err("operation_payload_unregister_special: Attempt to unregister payload while it in use.");
				result = -EBUSY;
				goto out;
			}
			list_del(&payload_elem->list);
			kfree(payload_elem);
			goto out;
		}
	}
	pr_err("operation_payload_unregister_special: Attempt to unregister payload which wasn't be registered.");
	result = -EINVAL;

out:
	mutex_unlock(&replacer->m);
	return result;
}

void* operation_get_orig_special(operation_replacer replacer,
	int operation_offset,
	void* key,
	struct operation_payload* payload)
{
    struct operation_replacer_data* replacer_data = replacer->data;
    struct operation_replacer_data_layer* layers;
    int i;
    
    BUG_ON(atomic_read(&replacer->n_uses) == 0);
    BUG_ON(replacer->data == NULL);
    BUG_ON(get_op_at_offset(payload->mask, operation_offset) == NULL);


    layers = replacer_data->layers;
    for(i = 0; i < replacer_data->n_repl_layers; i++)
    {
        if(layers[i].payload == payload) break;
    }
    BUG_ON(i >= replacer_data->n_repl_layers);
    
    if(get_op_at_offset(layers[i].ops_mask_prev, operation_offset) != NULL)
    {
        //Result - replacement operation of other payload
        return get_op_at_offset(layers[i].ops_repl_prev, operation_offset);
    }
    else
    {
        //Result - original operation
        return operation_get_orig(replacer, operation_offset, key);
    }
}
