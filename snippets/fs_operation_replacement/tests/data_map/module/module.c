#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_AUTHOR("Tsyvarev Andrey");
MODULE_LICENSE("GPL");

#include "data_map.h"



struct data_list
{
    void* key;
    void* data;
};

static struct data_list data_list[] =
{
    {(void*)0x5431, (void*)0xffe1},
    {(void*)0xF, (void*)0x123456},
    {NULL, (void*)0x54312},
    {(void*)0xFFFFFFFF, (void*)0x7543},

};
const int tested_elem = 1;

static int test_simple_common(int n_elems)
{
    int i;
    //Create map
    data_map_t map = data_map_create(n_elems);
    if(map == NULL)
    {
        pr_err("Failed to create map.");
        return -1;
    }
    //Add elements into map
    for(i = 0; i < ARRAY_SIZE(data_list); i++)
    {
        if(data_map_add(map, data_list[i].key, data_list[i].data))
        {
            pr_err("Failed to add {%p, %p} to the map.",
                data_list[i].key, data_list[i].data);
            goto map_err;
        }
    }
    // Verify, that mapping is correct
    for(i = 0; i < ARRAY_SIZE(data_list); i++)
    {
        void* data = data_map_get(map, data_list[i].key);
        if(IS_ERR(data))
        {
            pr_err("data_map_get() return error for existing key.");
            goto map_err;
        }
        if(data != data_list[i].data)
        {
            pr_err("data_map_get() return data different from that was passed to data_map_get().");
            goto map_err;
        }
    }
    //Delete elements from map
    for(i = 0; i < ARRAY_SIZE(data_list); i++)
    {
        void* data = data_map_delete(map, data_list[i].key);
        if(IS_ERR(data))
        {
            pr_err("data_map_delete() return error for existing key.");
            goto map_err;
        }
        if(data != data_list[i].data)
        {
            pr_err("data_map_delete() return data different from that was passed to data_map_get().");
            goto map_err;
        }
    }
    
    data_map_destroy(map);
    return 0;
map_err:
    data_map_delete_all(map, NULL, NULL);
    data_map_destroy(map);
    return -1;
}

static int test_simple(void)
{
    return test_simple_common(1000);
}

static int test_remove_elem(void)
{
    int i;
    //Create map
    data_map_t map = data_map_create(1000);
    if(map == NULL)
    {
        pr_err("Failed to create map.");
        return -1;
    }
    //Add elements into map
    for(i = 0; i < ARRAY_SIZE(data_list); i++)
    {
        if(data_map_add(map, data_list[i].key, data_list[i].data))
        {
            pr_err("Failed to add {%p, %p} to the map.",
                data_list[i].key, data_list[i].data);
            goto map_err;
        }
    }

    BUILD_BUG_ON(tested_elem >= ARRAY_SIZE(data_list));
    //Delete one element from map
    if(IS_ERR(data_map_delete(map, data_list[tested_elem].key)))
    {
        pr_err("Failed to delete element from map.");
        goto map_err;
    }

    // Verify, that mapping remains correct
    for(i = 0; i < ARRAY_SIZE(data_list); i++)
    {
        void* data = data_map_get(map, data_list[i].key);
        if(i == tested_elem)
        {
            if(!IS_ERR(data))
            {
                pr_err("data_map_get() return data for already removed key.");
                goto map_err;
            }
            continue;
        }


        if(IS_ERR(data))
        {
            pr_err("data_map_get() return error for existing key.");
            goto map_err;
        }
        if(data != data_list[i].data)
        {
            pr_err("data_map_get() return data different from that was passed to data_map_get().");
            goto map_err;
        }
    }

    // Delete others elements
    for(i = 0; i < ARRAY_SIZE(data_list); i++)
    {
        void* data = data_map_delete(map, data_list[i].key);
        if(i == tested_elem)
        {
            if(!IS_ERR(data))
            {
                pr_err("Attempt to delete already deleted element was succeed.");
                goto map_err;
            }
            continue;
        }

        if(IS_ERR(data))
        {
            pr_err("data_map_delete() return error for existing key.");
            goto map_err;
        }
        if(data != data_list[i].data)
        {
            pr_err("data_map_delete() return data different from that was passed to data_map_get().");
            goto map_err;
        }
    }
    
    data_map_destroy(map);
    return 0;
map_err:
    data_map_delete_all(map, NULL, NULL);
    data_map_destroy(map);
    return -1;
}

static void data_map_delete_callback(void* data, void* key, void* user_data)
{
    int i;
    for(i = 0; i < ARRAY_SIZE(data_list); i++)
    {
        if(data_list[i].key == key)
        {
            if(data_list[i].data != data)
            {
                pr_err("Incorrect 'data' was passed for callback function of data_map_delete_all().");
                *((int*)user_data) = -1;
            }
            return;
        }
    }
    pr_err("Unexistent 'key' was passed for callback function of data_map_delete_all().");
    *((int*)user_data) = -1;
}

static int test_delete_all(void)
{
    int i;
    int result = 0;
    //Create map
    data_map_t map = data_map_create(1000);
    if(map == NULL)
    {
        pr_err("Failed to create map.");
        return -1;
    }
    //Add elements into map
    for(i = 0; i < ARRAY_SIZE(data_list); i++)
    {
        if(data_map_add(map, data_list[i].key, data_list[i].data))
        {
            pr_err("Failed to add {%p, %p} to the map.",
                data_list[i].key, data_list[i].data);
            goto map_err;
        }
    }
    //Delete elements from map via data_map_delete_all()
    data_map_delete_all(map, data_map_delete_callback, &result);
    data_map_destroy(map);
    return result;
map_err:
    data_map_delete_all(map, NULL, NULL);
    data_map_destroy(map);
    return -1;
}

static int test_simple_1elem(void)
{
    return test_simple_common(1);
}


static int __init
this_module_init(void)
{
    if(test_simple()) return -EINVAL;
    if(test_remove_elem()) return -EINVAL;
    if(test_delete_all()) return -EINVAL;
    if(test_simple_1elem()) return -EINVAL;
    
    return 0;
}

static void __exit
this_module_exit(void)
{
    return;
}

module_init(this_module_init);
module_exit(this_module_exit);