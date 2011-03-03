#ifndef DATA_MAP_H
#define DATA_MAP_H

#include <linux/types.h>

/*
 * Define map between keys and values, both of type void*.
 */

typedef struct data_map* data_map_t;

/*
 * Create map which is expected to work with 'elem_number' number of
 * elements.
 * 
 * NOTE: elem_number do not restrict number of elements in the map.
 * It only affect on the perfomance of the lookup functions.
 * 
 * On error return NULL.
 */

data_map_t data_map_create(size_t elem_number);

/*
 * Add key->data pair to the map.
 * 
 * Return 0 on success and negative error code on fail.
 */
int data_map_add(data_map_t map, void* key, void* data);

/*
 * Return data corresponding to key.
 * 
 * If key wasn't registered, return ERR_PTR(-EINVAL).
 */
void* data_map_get(data_map_t map, void* key);

/*
 * Get data for the key, and remove key from map.
 * 
 * If key wasn't registered, return ERR_PTR(-EINVAL).
 */
void* data_map_delete(data_map_t map, void* key);

/* 
 * Remove all keys from map.
 * If free_data is not NULL, call it for every deleting element.
 * 
 * Note: Normally, for operation replacements, every key should be deleted
 * at its time with data_map_delete().
 * So 'free_data', beside freeing data, may also print warning.
 */
void data_map_delete_all(data_map_t map,
	void (*free_data)(void* data, void* key, void* user_data),
	void* user_data);

/*
 * Destroy map.
 * 
 * Map should be empty before this operation(see data_map_delete_all()).
 */
void data_map_destroy(data_map_t map);

#endif
