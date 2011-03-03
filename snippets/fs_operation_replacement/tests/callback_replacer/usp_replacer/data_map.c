#include "data_map.h"

#include <linux/list.h> /* hash table organization */
#include <linux/hash.h> /* hash function for pointers */
#include <linux/slab.h> /* kmalloc */
#include <linux/spinlock.h> /* spinlock */

/*temporary*/
#ifndef DEBUG
#define DEBUG 
#endif

struct data_map_element
{
	struct hlist_node node;
	void* key;
	void* data;
};

struct data_map
{
	struct hlist_head* hash_table;
	//only 2^n sizes will be used
	int size_bits;
	//protect from concurrent read and writes of the table
	spinlock_t lock;
};

/*
 * Create map which is expected to work with 'elem_number' number of
 * elements.
 * 
 * NOTE: elem_number do not restrict number of elements in the map.
 * It only affect on the perfomance of the lookup functions.
 * 
 * On error return NULL.
 */

data_map_t data_map_create(size_t elem_number)
{
	size_t hash_size;
	int size_bits;
	struct hlist_head* table;
	int i;
	data_map_t map;
	
	//elem_number should be no greater then 70% from hash size
	hash_size = (elem_number * 10 - 1)/ 7 + 1; 
	
	map = kmalloc(sizeof(*map), GFP_KERNEL);
	
	if(map == NULL)
	{
		pr_err("data_map_create: Cannot allocate data map.");
		return NULL;
	}
	for(size_bits = 0, --hash_size; hash_size != 0; size_bits++)
		hash_size >>= 1;

	//size_bits shouldn't be 0(otherwise hash_ptr return incorrect result)
	if(size_bits <= 0)
		size_bits = 1;
		
	table = kmalloc(sizeof(*table) * (1 << size_bits), GFP_KERNEL);
	if(table == NULL)
	{
		pr_err("data_map_create: Cannot allocate table for operation replacer.");
		kfree(map);
		return NULL;
	}
	for(i = 0; i < (1 << size_bits); i++)
		INIT_HLIST_HEAD(&table[i]);

	map->hash_table = table;
	map->size_bits = size_bits;
	spin_lock_init(&map->lock);

	return map;
}

/*
 * Add key->data pair to the map.
 * 
 * Return 0 on success and negative error code on fail.
 */

int data_map_add(data_map_t map, void* key, void* data)
{
	unsigned long flags;
	int error = 0;
	struct data_map_element* new_elem;
	int hash = hash_ptr(key, map->size_bits);
	new_elem = kmalloc(sizeof(*new_elem), GFP_KERNEL);
	if(new_elem == NULL)
	{
		pr_err("data_map_add: Failed to allocate new map element.");
		return -ENOMEM;
	}
	//Base initialization of new replacement element
	INIT_HLIST_NODE(&new_elem->node);
	new_elem->key = key;
	new_elem->data = data;
	//Insert new replacement element into hash table
	spin_lock_irqsave(&map->lock, flags);
#ifdef DEBUG
	{
		struct data_map_element* elem;
		struct hlist_node* tmp;
		hlist_for_each_entry(elem, tmp, &map->hash_table[hash], node)
		{
			if(elem->key == key)
			{
				pr_err("data_map_add: Attempt to add element with already used key.");
				error = -EINVAL;
				kfree(new_elem);
				goto out;
			}
		}
	}
#endif
	hlist_add_head(&new_elem->node, &map->hash_table[hash]);
out:	
	spin_unlock_irqrestore(&map->lock, flags);
	return error;
}

/*
 * Return data corresponding to key.
 * 
 * If key wasn't registered, return ERR_PTR(-EINVAL).
 */

void* data_map_get(data_map_t map, void* key)
{
	unsigned long flags;

	struct data_map_element* elem;
	struct hlist_node* tmp;

	void* result;
	int hash = hash_ptr(key, map->size_bits);

	result = ERR_PTR(-EINVAL);
	
	spin_lock_irqsave(&map->lock, flags);
	hlist_for_each_entry(elem, tmp, &map->hash_table[hash], node)
	{
		if(elem->key == key)
		{
			result = elem->data;
			goto out;
		}
	}
out:
	spin_unlock_irqrestore(&map->lock, flags);
	return result;
}

/*
 * Get data for the key, and remove key from map.
 * 
 * If key wasn't registered, return ERR_PTR(-EINVAL).
 */

void* data_map_delete(data_map_t map, void* key)
{
	unsigned long flags;

	struct data_map_element* elem;
	struct hlist_node* tmp;

	void* result;
	int hash = hash_ptr(key, map->size_bits);

	result = ERR_PTR(-EINVAL);

	spin_lock_irqsave(&map->lock, flags);
	hlist_for_each_entry(elem, tmp, &map->hash_table[hash], node)
	{
		if(elem->key == key)
		{
			result = elem->data;
			hlist_del(&elem->node);
			kfree(elem);
			goto out;
		}
	}
out:
	spin_unlock_irqrestore(&map->lock, flags);
	return result;
}


/* 
 * Remove all keys from map.
 * If free_data is not NULL, call it for every deleting element.
 * 
 * Note: Normally, for operation replacements, every key should be deleted
 * at its time with data_map_delete().
 * So 'free_data', aside from freeing data, may also print warning.
 */
void data_map_delete_all(data_map_t map,
	void (*free_data)(void* data, void* key, void* user_data),
	void* user_data)
{
	//Currently without lock.
	//(Function should be used without concurrency with adding/removing keys).
	int i;
	for(i = 0; i < (1 << map->size_bits); i++)
	{
		struct hlist_head* entry = &map->hash_table[i];
		while(entry->first)
		{
			struct data_map_element* elem = hlist_entry(entry->first,
				struct data_map_element, node);
			hlist_del(&elem->node);
			if(free_data)
				free_data(elem->data, elem->key, user_data);
			kfree(elem);
		}
	}
}

/*
 * Destroy map.
 * 
 * Map should be empty before this operation(see data_map_delete_all()).
 */
void data_map_destroy(data_map_t map)
{
	int i;
	int was_elems = 0;
	for(i = 0; i < (1 << map->size_bits); i++)
	{
		struct hlist_head* entry = &map->hash_table[i];
		while(entry->first)
		{
			struct data_map_element* elem = hlist_entry(entry->first,
				struct data_map_element, node);
			hlist_del(&elem->node);
			kfree(elem);
			was_elems = 1;
		}
	}
	if(was_elems) pr_warning("data_map_destroy: Destroying non-empty map.");
	kfree(map->hash_table);
	kfree(map);
}