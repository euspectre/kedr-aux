# fault_indicator.data - intermediate representation of the indicator data

module.author = <$module.author$>
module.license = <$module.license$>
indicator.name = <$indicator.name$>

<$if concat(indicator.parameter.type)$><$indicatorParameters: join(\n)$><$endif$>

<$if concat(global)$><$globalSection: join(\n)$><$endif$>

################# PID ##################

indicator.state.name = pid
indicator.state.type = atomic_t

# Declarations for pid
global =>>
#include <linux/sched.h> /* task_pid */
<<

# Simulate for pid
indicator.simulate.name = pid
indicator.simulate.first = yes
indicator.simulate.code =>>
	struct task_struct* t, *t_prev;
	int may_simulate = 0;
	pid_t pid;
	smp_rmb(); //volatile semantic of 'pid' field
	pid =  (pid_t)atomic_read(&state(pid));
	if(pid == 0) return 0;
	
	//read list in rcu-protected manner(perhaps, rcu may sence)
	rcu_read_lock();
	for(t = current, t_prev = NULL; (t != NULL) && (t != t_prev); t_prev = t, t = rcu_dereference(t->parent))
	{
		if(task_tgid_vnr(t) == pid) 
		{
			may_simulate = 1;
			break;
		}
	}
	rcu_read_unlock();
	if(!may_simulate) simulate_never();
	return 0;
<<

# Initialize for pid
indicator.init.name = pid
indicator.init.code =>>
	atomic_set(&state(pid), 0);
	return 0;
<<

# Destroy for pid does not need.

# Control file for pid

indicator.file.name = pid
indicator.file.fs_name = pid
indicator.file.get =>>
	pid_t pid = atomic_read(&state(pid));

	//write pid as 'long'
	return kasprintf(GFP_KERNEL, "%ld", (long)pid);
<<
indicator.file.set =>>
	//read pid as long
	long pid_long;
	int result = strict_strtol(str, 10, &pid_long);
	if(!result)
		atomic_set(&state(pid), (pid_t)pid_long);
	return result ? -EINVAL : 0;
<<

###############  Caller addresses list ####################
indicator.state.name = caller_addresses_list
indicator.state.type = struct list_head

indicator.state.name = cal_lock
indicator.state.type = spinlock_t

indicator.state.name = caller_addresses_detector
indicator.state.type = bool

# Declarations for caller addresses list
global =>>
#include <linux/list.h> /* struct list_head */
#include <linux/spinlock.h> /* spinlock_t */

struct address_elem
{
	struct list_head list;
	void* address;
};
<<

# Simulate for caller addresses list(only update list)
indicator.simulate.name = caller_addresses_list
# Simulate code doesn't use simulate_never(), but it should come before caller address restriction, which does.
indicator.simulate.first = yes
indicator.simulate.code =>>
	if(state(caller_addresses_detector))
	{
		unsigned long flags;
		struct address_elem* elem;
		spin_lock_irqsave(&state(cal_lock), flags);
		list_for_each_entry(elem, &state(caller_addresses_list), list)
		{
			if(elem->address == caller_address) break;
		}
		if(&elem->list == &state(caller_addresses_list))
		{
			struct address_elem* elem_new = kmalloc(sizeof(*elem_new), GFP_ATOMIC);
			if(elem_new)
			{
				
				elem_new->address = caller_address;
				list_add_tail(&elem_new->list, &state(caller_addresses_list));
			}
			else
			{
				//TODO: register allocation fail.
			}
		}
		spin_unlock_irqrestore(&state(cal_lock), flags);
	}
	return 0;
<<

# Initialize for caller addresses list
indicator.init.name = caller_addresses_list
indicator.init.code =>>
	INIT_LIST_HEAD(&state(caller_addresses_list));
	spin_lock_init(&state(cal_lock));
	state(caller_addresses_detector) = false;
	return 0;
<<

# Destroy for caller addresses list
global =>>
void caller_addresses_list_clear(struct list_head* addresses_list)
{
	while(!list_empty(addresses_list))
	{
		struct address_elem* elem = list_first_entry(addresses_list, typeof(*elem), list);
		list_del(&elem->list);
		kfree(elem);
	}
}
<<

indicator.destroy.name = caller_addresses_list
indicator.destroy.code =>>
	if(state(caller_addresses_list).prev)
	{
		caller_addresses_list_clear(&state(caller_addresses_list));
	}
<<

# Control file for caller_addresses_list

global =>>
	/* 
	 * Print list of addresses into string.
	 *
	 * Printing is doing in snprintf()-style.
	 *
	 * Should be executed under cal_lock.
	 */
static size_t print_addresses_list(char* buf, size_t size, struct list_head* addresses_list)
{
	size_t bytes_written = 0;
#define write_f(fmt, ...) {\
	size_t local_size = snprintf(buf, size, fmt, __VA_ARGS__); \
	bytes_written += local_size; \
	buf += local_size; \
	size = size > local_size ? size - local_size : 0; \
	}
	
	if(!list_empty(addresses_list))
	{
		//int n = 1;
		struct address_elem* elem = list_first_entry(addresses_list, typeof(*elem), list);

		write_f("0x%lx", (unsigned long)elem->address);

		list_for_each_entry_continue(elem, addresses_list, list)
		{
			write_f("\n0x%lx", (unsigned long)elem->address);
			//n++;
		}
		
		//write_f("\nTotal: %d", n);
	}
#undef write_f
	return bytes_written;
}
<<

indicator.file.name = caller_addresses_list
indicator.file.fs_name = caller_addresses_list
indicator.file.get =>>
	char *str;
	size_t str_len;

	unsigned long flags;
	spin_lock_irqsave(&state(cal_lock), flags);

	str_len = print_addresses_list(NULL, 0, &state(caller_addresses_list));
	
	str = kmalloc(str_len + 1, GFP_ATOMIC);
	if(str)
	{
		print_addresses_list(str, str_len + 1, &state(caller_addresses_list));
		str[str_len] = '\0';
	}
	else
	{
		pr_err("Cannot allocate string caller addresses list.\n");
	}

	spin_unlock_irqrestore(&state(cal_lock), flags);

	return str;
<<
indicator.file.set =>>
	unsigned long flags;
	spin_lock_irqsave(&state(cal_lock), flags);

	caller_addresses_list_clear(&state(caller_addresses_list));
	
	spin_unlock_irqrestore(&state(cal_lock), flags);
	return 0;
<<

# Control file for caller_addresses_detector
indicator.file.name = caller_addresses_detector
indicator.file.fs_name = caller_addresses_detector
indicator.file.get =>>
	return kasprintf(GFP_KERNEL, "%d", state(caller_addresses_detector)? 1 : 0);
<<

indicator.file.set =>>
	//read as long
	long value;
	int result = strict_strtol(str, 10, &value);
	if(!result)
		state(caller_addresses_detector) = value != 0;
	return result ? -EINVAL : 0;
<<

###############  Caller address ####################
indicator.state.name = caller_address_state
indicator.state.type = void*

# Simulate for caller_address
indicator.simulate.name = caller_address
indicator.simulate.first = yes
indicator.simulate.code =>>
	if(state(caller_address_state) && (caller_address != state(caller_address_state)))
		simulate_never();
	return 0;
<<

# Initialize for caller_address
indicator.init.name = caller_address
indicator.init.code =>>
	state(caller_address_state) = NULL;
	return 0;
<<

# Destroy for caller address does not need.

# Control file for caller address

indicator.file.name = caller_address
indicator.file.fs_name = caller_address
indicator.file.get =>>
	//write address as 'hex'
	return kasprintf(GFP_KERNEL, "0x%lx", (long)state(caller_address_state));
<<
indicator.file.set =>>
	//read address as hex
	long value;
	int result = strict_strtol(str, 16, &value);
	if(!result)
		state(caller_address_state) = (void*)value;
	return result ? -EINVAL : 0;
<<

###############  Expression and times ####################

# Declarations for the expression

global =>>
#include <kedr/defs.h>

#include <kedr/calculator/calculator.h>

#include <kedr/core/kedr.h> /* in_init */
#include <linux/random.h> /* random32() */

<$if expressionHasConstants$>// Constants in the expression
static struct kedr_calc_const constants[] = {
<$if concat(expression.constant.c_name)$>    <$expressionConstCDeclaration : join(,\n    )$>,
<$endif$><$if concat(expression.constant.name)$>    <$expressionConstGDeclaration : join(,\n    )$>,
<$endif$>};

static struct kedr_calc_const_vec all_constants[] =
{
	{ .n_elems = ARRAY_SIZE(constants), .elems = constants}
};

<$endif$>// Variables in the expression
static const char* var_names[]= {
	"times",//local variable of indicator state
<$if concat(expression.variable.name)$>    "<$expression.variable.name : join(",\n    ")$>",
<$endif$><$if concat(expression.variable.pname)$>    "<$expression.variable.pname : join(",\n    ")$>",
<$endif$>};
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

static const struct kedr_calc_weak_var weak_vars[] = {
	{ .name = "in_init", .compute = in_init_weak_var_compute },
	{ .name = "rnd100", .compute = rnd100_weak_var_compute },
	{ .name = "rnd10000", .compute = rnd10000_weak_var_compute },
<$if concat(expression.rvariable.name)$>    <$expressionRvarDeclaration : join(,\n    )$>,
<$endif$>};
<<

indicator.state.name = calc
indicator.state.type = kedr_calc_t*

indicator.state.name = expression
indicator.state.type = char*

indicator.state.name = times
indicator.state.type = atomic_t

# Simulate for expression
indicator.simulate.name = expression
indicator.simulate.first =
indicator.simulate.code =>>
	int result;
	int *kcalc;
	kedr_calc_int_t vars[ARRAY_SIZE(var_names)];
	kedr_calc_int_t* var_next = vars;

	*var_next++ = atomic_inc_return(&state(times));

<$if concat(expression.variable.name)$>    <$expressionVarGSet : join(\n    )$>

<$endif$>    <$if concat(expression.variable.pname)$><$expressionVarPSet : join(\n    )$>
	
<$endif$>    rcu_read_lock();
	kcalc = (int *)(state(calc));
	result = kedr_calc_evaluate((kedr_calc_t *)(rcu_dereference(kcalc)), vars);
	rcu_read_unlock();
	return result;
<<

#Initialize expresssion part of the indicator

indicator.init.name = expression
indicator.init.code =>>
	const char* expression = params && *params ? params : "0";
	// Initialize expression
	atomic_set(&state(times), 0);
	
	state(calc) = kedr_calc_parse(expression,
		<$if expressionHasConstants$>ARRAY_SIZE(all_constants), all_constants<$else$>0, NULL<$endif$>,
		ARRAY_SIZE(var_names), var_names,
		ARRAY_SIZE(weak_vars), weak_vars);
	if(state(calc) == NULL)
	{
		pr_err("Cannot parse string expression.\n");
		return -1;
	}
	state(expression) = kstrdup(expression , GFP_KERNEL);
	if(state(expression) == NULL)
	{
		pr_err("Cannot allocate memory for string expression.\n");
		return -ENOMEM;
	}
	return 0;
<<

#Destroy expresssion part of the indicator

indicator.destroy.name = expression
indicator.destroy.code =>>
	if(state(expression) != NULL)
		kfree(state(expression));
	if(state(calc) != NULL)
		kedr_calc_delete(state(calc));
<<

# Control file for the expression

indicator.file.name = expression
indicator.file.fs_name = expression
indicator.file.get =>>
	return kstrdup(state(expression), GFP_KERNEL);
<<
indicator.file.set =>>
	char *new_expression;
	kedr_calc_t *old_calc;
	kedr_calc_t *new_calc;
	
	new_calc = kedr_calc_parse(str,
		<$if expressionHasConstants$>ARRAY_SIZE(all_constants), all_constants<$else$>0, NULL<$endif$>,
		ARRAY_SIZE(var_names), var_names,
		ARRAY_SIZE(weak_vars), weak_vars);
	if(new_calc == NULL)
	{
		pr_err("Cannot parse expression.\n");
		return -EINVAL;
	}

	new_expression = kstrdup(str, GFP_KERNEL);
	if(new_expression == NULL)
	{
		pr_err("Cannot allocate memory for string expression.\n");
		kedr_calc_delete(new_calc);
		return -ENOMEM;
	}
	
	old_calc = state(calc);
	{
		int *kcalc = (int *)new_calc;
		int **tmp = (int **)(&state(calc));
		rcu_assign_pointer(*tmp, kcalc);
	}
	
	synchronize_rcu();

	kfree(state(expression));
	state(expression) = new_expression;

	kedr_calc_delete(old_calc);
	
	return 0;
<<


# Control file for times

indicator.file.name = times
indicator.file.fs_name = times
indicator.file.get =>>
	char *str;
	int str_len;
	unsigned long times = (unsigned long)atomic_read(&state(times));

	str_len = snprintf(NULL, 0, "%lu", times);
	
	str = kmalloc(str_len + 1, GFP_KERNEL);
	if(str == NULL)
	{
		pr_err("Cannot allocate string for times.\n");
		return NULL;
	}
	snprintf(str, str_len + 1, "%lu", times);
	return str;
<<
indicator.file.set =>>
	atomic_set(&state(times), 0);
	return 0;
<<
