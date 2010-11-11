/*********************************************************************
 * Indicator: <$indicator.name$>
 *********************************************************************/
#include <linux/module.h>
#include <linux/init.h>

/*
 * Maximum number of different addresses, information about which is stored.
 *
 * That is, 'local_times' counter is valid only for that number of addresses.
 * For every next address, which is differ from these ones, 'local_times' is 1.
 *
 * But every such address miss is fixed and shown in debugfs.
 */
#ifndef ADDRESSES_MAX_NUMBER
#define ADDRESSES_MAX_NUMBER 100
#endif

MODULE_AUTHOR("<$module.author$>");
MODULE_LICENSE("<$module.license$>");
/*********************************************************************/

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */

#include <linux/debugfs.h>

#include <linux/mutex.h>

#include <linux/spinlock.h>

#include <kedr/fault_simulation/fault_simulation.h>
#include <kedr/fault_simulation/calculator.h>
#include <kedr/control_file/control_file.h>

#include <kedr/base/common.h> /* in_init */
#include <linux/sched.h> /* task_pid */
#include <linux/random.h> /* random32() */

// Macros for unify output information to the kernel log file
#define debug(str, ...) pr_debug("%s: " str, __func__, __VA_ARGS__)
#define debug0(str) debug("%s", str)

#define print_error(str, ...) pr_err("%s: " str, __func__, __VA_ARGS__)
#define print_error0(str) print_error("%s", str)

<$global$>

// Constants in the expression
<$expressionConstDeclarations$>

// Variables in the expression
static const char* var_names[]= {
    "times",//local variable of indicator state
    "caller",
    "local_times",
<$expressionVarNames$>
};
// Runtime variables in the expression
static kedr_calc_int_t in_init_weak_var_compute(void)
{
    return kedr_target_module_in_init();
}

static kedr_calc_int_t rnd100_weak_var_compute(void)
{
    return random32() % 100;
}

static kedr_calc_int_t rnd10000_weak_var_compute(void)
{
    return random32() % 10000;
}

<$expressionRvarFunctions$>

static const struct kedr_calc_weak_var weak_vars[] = {
    { .name = "in_init", .compute = in_init_weak_var_compute },
    { .name = "rnd100", .compute = rnd100_weak_var_compute },
    { .name = "rnd10000", .compute = rnd10000_weak_var_compute },
<$expressionRvarDeclarations$>
};

// Indicator parameters
<$pointDataType$>

// Data for particular caller address
struct address_local_data
{
	void* address;
	atomic_t times;
};

struct address_local_state
{
	//array with fixed capacity, but with floating size
	struct address_local_data* data;
	//
	int array_capacity;
	int array_size;
	// number of failed attemps to get local data
	int misses;
	//protect 'size' field and content of 'data' field.
	spinlock_t lock;
};

/*
 * If 'state' contains data for this address, return these data;
 * else if data may be added, add data and return these data;
 * else return NULL.
 */

static struct address_local_data*
get_address_local_data(struct address_local_state* state, void* address);

static struct address_local_state*
address_local_state_create(int capacity);

static void
address_local_state_destroy(struct address_local_state* state);

struct indicator_real_state
{
    kedr_calc_t* calc;
    char* expression;
    atomic_t times;
    //if not 0, we shouldn't make fail processes which is not derived from process of this pid.
    //should have java volatile-like behaviour
    atomic_t pid;
    //
	struct address_local_state* addresses_state;
    //
    struct dentry* expression_file;
    struct dentry* pid_file;
    // statistic files for addresses state
    struct dentry* addresses_max_number_file;
    struct dentry* addresses_current_number_file;
    struct dentry* addresses_misses_file;
};
//Protect from concurrent access all except read from 'struct indicator_real_state'.'calc'.
// That read is protected by rcu.
DEFINE_MUTEX(indicator_mutex);

////////////////Auxiliary functions///////////////////////////

// Create indicator state
static struct indicator_real_state*
indicator_state_create(const char* expression,
	int addresses_max_number,
	struct dentry* control_directory);
// Destroy indicator state
static void
indicator_state_destroy(struct indicator_real_state* state);

// Change expression of the indicator. On fail state is not changed.
// Should be executed with mutex taken.
static int
indicator_state_expression_set_internal(
	struct indicator_real_state* state,
	const char* expression);
//Determine according to the value of 'pid' field of the indicator state,
//whether we may make fail this process or not.
static int process_may_fail(pid_t filter_pid);
// Change pid-constraint of the indicator.
// Should be executed with mutex taken.
static int
indicator_state_pid_set_internal(
	struct indicator_real_state* state,
	pid_t pid);
//////////////Indicator's functions declaration////////////////////////////

static int indicator_simulate(void* indicator_state, void* user_data);
static void indicator_instance_destroy(void* indicator_state);
static int indicator_instance_init(void** indicator_state,
    const char* params, struct dentry* control_directory);

struct kedr_simulation_indicator* indicator;

static int __init
indicator_init(void)
{
    indicator = kedr_fsim_indicator_register("<$indicator.name$>",
        indicator_simulate, "<$indicatorFormatString$>",
        indicator_instance_init,
        indicator_instance_destroy);
    if(indicator == NULL)
    {
        printk(KERN_ERR "Cannot register indicator.\n");
        return -1;
    }

    return 0;
}

static void
indicator_exit(void)
{
	kedr_fsim_indicator_unregister(indicator);
	return;
}

module_init(indicator_init);
module_exit(indicator_exit);

////////////Implementation of indicator's functions////////////////////

int indicator_simulate(void* indicator_state, void* user_data)
{
    int result = 0;

    struct indicator_real_state* indicator_real_state =
        (struct indicator_real_state*)indicator_state;
    
    smp_rmb();//volatile semantic of 'pid' field
    if(process_may_fail((pid_t)atomic_read(&indicator_real_state->pid)))
    {

        kedr_calc_int_t vars[ARRAY_SIZE(var_names)];
        struct address_local_data* local_data;
		void* caller_address =
            ((struct point_data*)user_data)->caller_address;
		
		local_data = get_address_local_data(indicator_real_state->addresses_state,
			caller_address);

        vars[0] = atomic_inc_return(&indicator_real_state->times);
        vars[1] = (unsigned long)caller_address;
        vars[2] = (local_data != NULL) ?
            atomic_inc_return(&local_data->times) : 1;

<$pointDataUse$>

<$expressionVarsSet$>

<$pointDataUnuse$>

        rcu_read_lock();
        result = kedr_calc_evaluate(rcu_dereference(indicator_real_state->calc), vars);
        rcu_read_unlock();
    }
    return result;
}

void indicator_instance_destroy(void* indicator_state)
{
    struct indicator_real_state* indicator_real_state =
        (struct indicator_real_state*)indicator_state;
    
    indicator_state_destroy(indicator_real_state);
}

int indicator_instance_init(void** indicator_state,
    const char* params, struct dentry* control_directory)
{
    struct indicator_real_state* indicator_real_state;
    const char* expression = (*params != '\0') ? params : "0";
    int addresses_max_number = ADDRESSES_MAX_NUMBER;
    
    indicator_real_state = indicator_state_create(expression,
		addresses_max_number,
        control_directory);
    if(indicator_real_state == NULL) return -1;
    
    *indicator_state = indicator_real_state;
    return 0;
}
//////////////////Implementation of auxiliary functions////////////////

struct address_local_state*
address_local_state_create(int capacity)
{
    struct address_local_state* state =
        kzalloc(sizeof(*state), GFP_KERNEL);
    if(state == NULL)
    {
        pr_err("Cannot allocate memory for local state.");
        return NULL;
    }
    
    state->data = kmalloc(sizeof(struct address_local_data) * capacity,
        GFP_KERNEL);

    if(state->data == NULL)
    {
        pr_err("Cannot allocate memory for local state data.");
        goto fail;
    }

    state->array_capacity = capacity;
    state->array_size = 0;
    state->misses = 0;
    
    spin_lock_init(&state->lock);
    
    return state;
fail:
    address_local_state_destroy(state);
    return NULL;
}

void
address_local_state_destroy(struct address_local_state* state)
{
    kfree(state->data);
    kfree(state);
}

struct address_local_data*
get_address_local_data(struct address_local_state* state, void* address)
{
    unsigned long flags;
    int i;
    struct address_local_data* local_data = NULL;

    spin_lock_irqsave(&state->lock, flags);
    for(i = 0; i < state->array_size; i++)
    {
        if(state->data[i].address == address)
        {
            local_data = &state->data[i];
            break;
        }
    }
    if((i == state->array_size)
        && (state->array_size < state->array_capacity))
    {
        state->array_size++;
        local_data = &state->data[i];
        local_data->address = address;
        atomic_set(&local_data->times, 0);
    }
    else if(local_data == NULL)
    {
        state->misses++;
    }
    spin_unlock_irqrestore(&state->lock, flags);

    return local_data;
}

// Note: Whenever this function returns 0 or not 0,
// indicator_state_destroy_files() should be called.
static int
indicator_state_create_files(struct indicator_real_state* state, 
    struct dentry* control_directory);

struct indicator_real_state*
indicator_state_create(const char* expression,
	int addresses_max_number,
	struct dentry* control_directory)
{
    struct indicator_real_state* state = 
		kzalloc(sizeof(*state), GFP_KERNEL);
	if(state == NULL)
    {
        pr_err("Cannot allocate memory for indicator state");
        return NULL;
    }

    // Initialize addresses state
	state->addresses_state = address_local_state_create(
		addresses_max_number);

	if(state->addresses_state == NULL)
	{
		goto fail;
	}

    // Initialize expression
    atomic_set(&state->times, 0);
	
    state->calc = kedr_calc_parse(expression,
        <$expressionConstParams$>,
        ARRAY_SIZE(var_names), var_names,
        ARRAY_SIZE(weak_vars), weak_vars);
    if(state->calc == NULL)
    {
        pr_err("Cannot parse string expression.");
        goto fail;
    }
    state->expression = kstrdup(expression , GFP_KERNEL);
    if(state->expression == NULL)
    {
        pr_err("Cannot allocate memory for string expression.");
        goto fail;
    }
    // Create control files
    if(control_directory != NULL)
    {
        if(indicator_state_create_files(state, control_directory))
        {
            goto fail;
        }
    }
    return state;

fail:
    indicator_state_destroy(state);
    return NULL;
}

static void
indicator_state_remove_files(struct indicator_real_state* state); 

void indicator_state_destroy(struct indicator_real_state* state)
{
    indicator_state_remove_files(state);
    if(state->expression != NULL)
        kfree(state->expression);
    if(state->calc != NULL)
        kedr_calc_delete(state->calc);
	if(state->addresses_state)
		address_local_state_destroy(state->addresses_state);
    kfree(state);
}

// Change expression of the indicator. On fail state is not changed.
// Should be executed with mutex taken.
static int
indicator_state_expression_set_internal(struct indicator_real_state* state, const char* expression)
{
    char *new_expression;
    kedr_calc_t *old_calc, *new_calc;
    
    new_calc = kedr_calc_parse(expression,
        <$expressionConstParams$>,
        ARRAY_SIZE(var_names), var_names,
        ARRAY_SIZE(weak_vars), weak_vars);
    if(new_calc == NULL)
    {
        pr_err("Cannot parse expression");
        return -EINVAL;
    }
    
    new_expression = kmalloc(strlen(expression) + 1, GFP_KERNEL);
    if(new_expression == NULL)
    {
        pr_err("Cannot allocate memory for string expression.");
        kedr_calc_delete(new_calc);
        return -1;
    }
    strcpy(new_expression, expression);
    
    old_calc = state->calc;
    atomic_set(&state->times, 0);
    
    rcu_assign_pointer(state->calc, new_calc);
    
    kfree(state->expression);
    state->expression = new_expression;
    
    synchronize_rcu();
    kedr_calc_delete(old_calc);
    
    return 0;
}

//Determine according to the value of 'pid' field of the indicator state,
//whether we may make fail this process or not.
int process_may_fail(pid_t filter_pid)
{
    struct task_struct* t, *t_prev;
    int result = 0;
    if(filter_pid == 0) return 1;
    
    //read list in rcu-protected manner(perhaps, rcu may sence)
    rcu_read_lock();
    for(t = current, t_prev = NULL; (t != NULL) && (t != t_prev); t_prev = t, t = rcu_dereference(t->parent))
    {
        if(task_tgid_vnr(t) == filter_pid) 
        {
            result = 1;
            break;
        }
    }
    rcu_read_unlock();
    return result;
}
// Change pid-constraint of the indicator.
// Should be executed with mutex taken.
int
indicator_state_pid_set_internal(struct indicator_real_state* state, pid_t pid)
{
    atomic_set(&state->pid, pid);
    //write under write lock, so doesn't need barriers
    return 0;
}


/////////////////////Files implementation/////////////////////////////
// Expression
static char* indicator_expression_file_get_str(struct inode* inode);
static int indicator_expression_file_set_str(const char* str, struct inode* inode);

CONTROL_FILE_OPS(indicator_expression_file_operations,
    indicator_expression_file_get_str, indicator_expression_file_set_str);

// Pid
static char* indicator_pid_file_get_str(struct inode* inode);
static int indicator_pid_file_set_str(const char* str, struct inode* inode);

CONTROL_FILE_OPS(indicator_pid_file_operations,
    indicator_pid_file_get_str, indicator_pid_file_set_str);

// Addresses max number
static char* indicator_addresses_max_number_file_get_str(struct inode* inode);

CONTROL_FILE_OPS(indicator_addresses_max_number_file_operations,
    indicator_addresses_max_number_file_get_str, NULL);

// Addresses current number
static char* indicator_addresses_current_number_file_get_str(struct inode* inode);

CONTROL_FILE_OPS(indicator_addresses_current_number_file_operations,
    indicator_addresses_current_number_file_get_str, NULL);

// Addresses misses
static char* indicator_addresses_misses_file_get_str(struct inode* inode);

CONTROL_FILE_OPS(indicator_addresses_misses_file_operations,
    indicator_addresses_misses_file_get_str, NULL);



int indicator_state_create_files(struct indicator_real_state* state,
    struct dentry* dir)
{
    // File for set/get expression
    state->expression_file = debugfs_create_file("expression",
        S_IRUGO | S_IWUSR | S_IWGRP,
        dir,
        state, &indicator_expression_file_operations);
    if(state->expression_file == NULL)
    {
        pr_err("Cannot create expression file for indicator.");
        return -1;
    }
    // File for set/get pid constraint
    state->pid_file = debugfs_create_file("pid",
        S_IRUGO | S_IWUSR | S_IWGRP,
        dir,
        state, &indicator_pid_file_operations);
    
    if(state->pid_file == NULL)
    {
        pr_err("Cannot create pid file for indicator.");
        return -1;
    }
    // File for get max addresses number
    state->addresses_max_number_file = 
		debugfs_create_file("addresses_max_number",
			S_IRUGO,
			dir,
			state, &indicator_addresses_max_number_file_operations);
    
    if(state->addresses_max_number_file == NULL)
    {
        pr_err("Cannot create file for maximum addresses number for indicator.");
        return -1;
    }
    
    // File for get current addresses number
    state->addresses_current_number_file = 
		debugfs_create_file("addresses_current_number",
			S_IRUGO,
			dir,
			state, &indicator_addresses_current_number_file_operations);
    
    if(state->addresses_current_number_file == NULL)
    {
        pr_err("Cannot create file for current addresses number for indicator.");
        return -1;
    }
    
	// File for get addresses misses
    state->addresses_misses_file = 
		debugfs_create_file("addresses_misses",
			S_IRUGO,
			dir,
			state, &indicator_addresses_misses_file_operations);
    
    if(state->addresses_misses_file == NULL)
    {
        pr_err("Cannot create file for addresses misses for indicator.");
        return -1;
    }
    return 0;
}

void
indicator_state_remove_files(struct indicator_real_state* state)
{
    // Expression file
    if(state->expression_file)
    {
        mutex_lock(&indicator_mutex);
        state->expression_file->d_inode->i_private = NULL;
        mutex_unlock(&indicator_mutex);

        debugfs_remove(state->expression_file);
    }
    // Pid file
    if(state->pid_file)
    {
        mutex_lock(&indicator_mutex);
        state->pid_file->d_inode->i_private = NULL;
        mutex_unlock(&indicator_mutex);

        debugfs_remove(state->pid_file);
    }
    // Addresses max number file
    if(state->addresses_max_number_file)
    {
        mutex_lock(&indicator_mutex);
        state->addresses_max_number_file->d_inode->i_private = NULL;
        mutex_unlock(&indicator_mutex);

        debugfs_remove(state->addresses_max_number_file);
    }
    // Addresses current number file
    if(state->addresses_current_number_file)
    {
        mutex_lock(&indicator_mutex);
        state->addresses_current_number_file->d_inode->i_private = NULL;
        mutex_unlock(&indicator_mutex);

        debugfs_remove(state->addresses_current_number_file);
    }
	// Addresses misses file
    if(state->addresses_misses_file)
    {
        mutex_lock(&indicator_mutex);
        state->addresses_misses_file->d_inode->i_private = NULL;
        mutex_unlock(&indicator_mutex);

        debugfs_remove(state->addresses_misses_file);
    }
}

/////Implementation of getter and setter for file operations/////////////
char* indicator_expression_file_get_str(struct inode* inode)
{
    char *str;
    struct indicator_real_state* state;
   
    if(mutex_lock_killable(&indicator_mutex))
    {
        debug0("Operation was killed");
        return NULL;
    }

    state = inode->i_private;
    if(state)
    {
        str = kstrdup(state->expression, GFP_KERNEL);
    }
    else
    {
        str = NULL;//'device', corresponed to file, is not exist
    }
    mutex_unlock(&indicator_mutex);
    
    return str;
}
int indicator_expression_file_set_str(const char* str, struct inode* inode)
{
    int error;
    struct indicator_real_state* state;
    
    if(mutex_lock_killable(&indicator_mutex))
    {
        debug0("Operation was killed");
        return -EINTR;
    }

    state = inode->i_private;
    if(state)
    {
        error = indicator_state_expression_set_internal(state, str);
    }
    else
    {
        error = -EINVAL;//'device', corresponed to file, is not exist
    }
    mutex_unlock(&indicator_mutex);
    return error;
}

static char* pid_to_str(pid_t pid)
{
    char *str;
    int str_len;

    //write pid as 'long'
    str_len = snprintf(NULL, 0, "%ld", (long)pid);
    
    str = kmalloc(str_len + 1, GFP_KERNEL);
    if(str == NULL)
    {
        pr_err("Cannot allocate string for pid");
        return NULL;
    }
    snprintf(str, str_len + 1, "%ld", (long)pid);
    return str;
}

int str_to_pid(const char* str, pid_t* pid)
{
    //read pid as long
    long pid_long;
    int result = strict_strtol(str, 10, &pid_long);
    if(!result)
        *pid = (pid_t)pid_long;
    return result;
}

char* indicator_pid_file_get_str(struct inode* inode)
{
    struct indicator_real_state* state;
    pid_t pid = 0;
   
    if(mutex_lock_killable(&indicator_mutex))
    {
        debug0("Operation was killed");
        return NULL;
    }
    state = inode->i_private;
    if(state)
        pid = atomic_read(&state->pid);//read under write lock, so doesn't need barriers

    mutex_unlock(&indicator_mutex);
    
    if(!state) return NULL;//'device', corresponed to file, doesn't not exist
    
    return pid_to_str(pid);
}
int indicator_pid_file_set_str(const char* str, struct inode* inode)
{
    int error;
    struct indicator_real_state* state;
    pid_t pid;
    
    error = str_to_pid(str, &pid);
    if(error) return -EINVAL;
        
    if(mutex_lock_killable(&indicator_mutex))
    {
        debug0("Operation was killed");
        return -EINTR;
    }

    state = inode->i_private;
    if(state)
    {
        error = indicator_state_pid_set_internal(state, pid);
    }
    else
    {
        error = -EINVAL;//'device', corresponed to file, is not exist
    }
    mutex_unlock(&indicator_mutex);
    return error;
}

static char* int_to_str(int value)
{
    char *str;
    int str_len;

    //write pid as 'long'
    str_len = snprintf(NULL, 0, "%d", value);
    
    str = kmalloc(str_len + 1, GFP_KERNEL);
    if(str == NULL)
    {
        pr_err("Cannot allocate string for write integer number to it.");
        return NULL;
    }
    snprintf(str, str_len + 1, "%d", value);
    return str;
}

char* indicator_addresses_max_number_file_get_str(struct inode* inode)
{
    struct indicator_real_state* state;
    int number = 0;
   
    if(mutex_lock_killable(&indicator_mutex))
    {
        debug0("Operation was killed");
        return NULL;
    }
    state = inode->i_private;
    //constant may be read without lock
    if(state)
        number = state->addresses_state->array_capacity;

    mutex_unlock(&indicator_mutex);
    
    if(!state) return NULL;//'device', corresponed to file, doesn't not exist
    
    return int_to_str(number);
}

char* indicator_addresses_current_number_file_get_str(struct inode* inode)
{
    struct indicator_real_state* state;
    int number = 0;
   
    if(mutex_lock_killable(&indicator_mutex))
    {
        debug0("Operation was killed");
        return NULL;
    }
    state = inode->i_private;
    //read without lock (reading of int should be atomic?)
    if(state)
        number = state->addresses_state->array_size;

    mutex_unlock(&indicator_mutex);
    
    if(!state) return NULL;//'device', corresponed to file, doesn't not exist
    
    return int_to_str(number);
}

char* indicator_addresses_misses_file_get_str(struct inode* inode)
{
    struct indicator_real_state* state;
    int number = 0;
   
    if(mutex_lock_killable(&indicator_mutex))
    {
        debug0("Operation was killed");
        return NULL;
    }
    state = inode->i_private;
    //read without lock (reading of int should be atomic?)
    if(state)
        number = state->addresses_state->misses;

    mutex_unlock(&indicator_mutex);
    
    if(!state) return NULL;//'device', corresponed to file, doesn't not exist
    
    return int_to_str(number);
}
