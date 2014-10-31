/* Managing the handlers of the function calls.
 * The handlers themselves are defined elsewhere (in my_funcs.c) */

#include <string>
#include <map>
#include <vector>

#include "common_includes.h"
#include "handlers.h"
/* ====================================================================== */

/* The mapping {name_of_the_target_func => pair of handlers} */
typedef std::map<std::string, HandlerInfo> HandlerMap;
static HandlerMap handler_map;

/* The mapping {type sequence => pair of handlers} used to handle indirect 
 * calls, for example. */
typedef std::vector<HandlerInfo> HandlerMapByTypes;
static HandlerMapByTypes handler_map_by_types;
/* ====================================================================== */

/* A non-strict partial ordering relationship on the types.
 * 
 * type1 is usually the type node of some argument or of the return value of
 * a target function, type2 is usually the type node used in the handler for
 * that function.
 * 
 * type1 <= type2 means type2 can be used in the appropriate handler instead
 * of type1. For example, an argument of the target function having type of
 * 'struct file_operations *' (type1) can be passed as 'void *' to the 
 * handler. This is used to avoid constructing the types unknown to GCC by
 * default during the instrumentation.
 *
 * The handlers will get the arguments of the types known to GCC and then
 * will cast them appropriately for their use.
 *
 * type1 <= type2 if and only if at least one of the following is true:
 * 
 * 1. type1 and type2 are equivalent, in the sense that no conversion is 
 *    needed to cast from type1 to type2 and vice versa (if both are 'void',
 *    it falls into this category too).
 *
 * 2. type1 and type2 are pointer types (yes, we can ignore their qualifiers
 *    here for now).
 * 
 * 3. type1 and type2 are integral types (integral type: integer, boolean or
 *    enum) and precision (the number of bits used by the value) of type1 is
 *    less than or equal to the precision of type2. That is, type1 can be 
 *    cast to type2 without data loss.
 * 
 * 4. type1 is neither a pointer, nor an integral type or void, and type2 is
 *    a pointer. For such type1, a copy of the respective value will be 
 *    created in a temporary variable and its address will be passed to the
 *    handler. */
struct TypeLessOrEqual {
	bool operator()(const tree type1, const tree type2) const;
};

bool
TypeLessOrEqual::operator()(const tree type1, const tree type2) const
{
	if (types_compatible_p(type1, type2))
		return true;
	
	if (POINTER_TYPE_P(type1) && POINTER_TYPE_P(type2))
		return true;
	
	/* Ignore the possible difference in signedness too, only the number
	 * of bits to store the values matters. */
	if (INTEGRAL_TYPE_P(type1) && INTEGRAL_TYPE_P(type2) &&
	    TYPE_PRECISION(type1) <= TYPE_PRECISION(type2)) {
		return true;
	}
	
	if (POINTER_TYPE_P(type2) && 
	    !types_compatible_p(type1, void_type_node) &&
	    !POINTER_TYPE_P(type1) &&
	    !INTEGRAL_TYPE_P(type1)) {
		return true;
	}

	return false;
}

/* A comparator for the type sequences that uses TypeLessOrEqual for each 
 * pair of elements.
 *
 * This induces a partial ordering relationship on the type sequences. */
struct TypeSeqLessOrEqual
{
	bool operator()(const TypeSeq& ts1, const TypeSeq& ts2) const
	{
		if (ts1.size() != ts2.size())
			return false;
		
		TypeLessOrEqual less_or_equal;
		
		for (unsigned int i = 0; i < ts1.size(); ++i) {
			if (!less_or_equal(ts1[i], ts2[i]))
				return false;
		}
		return true;
	}
};

/* Returns a pointer to the pair of handlers appropriate for the given type 
 * sequence (ts[0] - return type, the rest - types of the arguments), or 
 * NULL if there is no such pair. 
 * "Appropriate" means the number of types is the same and each type in 'ts'
 * is <= the corresponding type in the key. */
static const HandlerInfo * 
get_handlers_by_type_seq(const TypeSeq& ts)
{
	TypeSeqLessOrEqual seq_less_or_equal;
	unsigned int nelems = handler_map_by_types.size();
	
	for (unsigned int i = 0; i < nelems; ++i) {
		if (seq_less_or_equal(ts, handler_map_by_types[i].ts))
			return &handler_map_by_types[i];
	}
	return NULL;
}

/* Append the type sequence for a given fntype to 'ts' */
static void
type_seq_for_fntype(TypeSeq& ts, tree fntype)
{
	ts.push_back(TREE_TYPE(fntype)); /* return type */
	
	/* types of the arguments (except the variable ones) */
	tree parm_types = TYPE_ARG_TYPES(fntype);
	for (tree tp = parm_types; tp; tp = TREE_CHAIN(tp)) {
		if (types_compatible_p(TREE_VALUE(tp), void_type_node))
			break;
		
		ts.push_back(TREE_VALUE(tp));
	}
}

const HandlerInfo *
get_handlers_by_fntype(tree fntype)
{
	TypeSeq ts;
	type_seq_for_fntype(ts, fntype);	
	return get_handlers_by_type_seq(ts);
}

const HandlerInfo *
get_handlers_by_function_name(const char *name, tree fndecl)
{
	tree fntype = TREE_TYPE(fndecl);
	
	HandlerMap::const_iterator it;
	it = handler_map.find(name);
	if (it == handler_map.end())
		return NULL; 
	
	const HandlerInfo *hi = &it->second;
	TypeSeqLessOrEqual seq_less_or_equal;
	TypeSeq ts;
	
	/* Sanity check: do not add the handlers if the types of the 
	 * arguments and/or return value are not compatible. */
	type_seq_for_fntype(ts, fntype);
	if (!seq_less_or_equal(ts, hi->ts)) {
		fprintf(stderr, 
			"The registered handlers for \"%s()\" "
			"are not compatible with that function, skipping.\n",
			name);
		//<>
		fprintf(stderr, "fntype: \n");
		debug_tree(fntype);
		for (unsigned int i = 0; i < hi->ts.size(); ++i) {
			debug_tree(hi->ts[i]);
		}
		fprintf(stderr, "----------------------------------------\n\n");
		//<>
		return NULL;
	}
	return hi;
}
/* ====================================================================== */

static tree entry_handler_decl;
static tree exit_handler_decl;

tree
get_entry_handler_decl(void) 
{
	return entry_handler_decl;
}

tree
get_exit_handler_decl(void) 
{
	return exit_handler_decl;
}

static void
set_handler_decl_properties(tree hdecl)
{
	DECL_ASSEMBLER_NAME(hdecl);
	TREE_PUBLIC(hdecl) = 1;
	DECL_EXTERNAL(hdecl) = 1;
	DECL_ARTIFICIAL(hdecl) = 1;
}
/* ====================================================================== */

/* void my_func_vmalloc_pre(unsigned long size, struct my_struct *ls)
 * void my_func_vmalloc_post(void *ret, struct my_struct *ls) */
static void 
build_handler_decls_vmalloc(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(ptr_type_node); /* return type */
	hi.ts.push_back(long_unsigned_type_node); /* arg1 */
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1],
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_vmalloc_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_vmalloc_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["vmalloc"] = hi;
}

/* void my_func_vfree_pre(void *addr, struct my_struct *ls)
 * void my_func_vfree_post(struct my_struct *ls) */
static void 
build_handler_decls_vfree(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(void_type_node); /* return type */
	hi.ts.push_back(ptr_type_node); /* arg1 */
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1],
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_vfree_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_vfree_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["vfree"] = hi;
}

/* void my_func_alloc_chrdev_region_pre(
 *	dev_t *dev, unsigned baseminor, 
 * 	unsigned count, const char *name, struct my_struct *ls)
 * void my_func_alloc_chrdev_region_post(int ret, 
 * struct my_struct *ls) */
static void 
build_handler_decls_alloc_chrdev_region(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(integer_type_node); /* return type */
	hi.ts.push_back(ptr_type_node); /* arg1 */
	hi.ts.push_back(unsigned_type_node); /* arg2 */
	hi.ts.push_back(unsigned_type_node); /* arg3 */
	hi.ts.push_back(ptr_type_node); /* arg4 */
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1], hi.ts[2], hi.ts[3], hi.ts[4],
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_alloc_chrdev_region_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_alloc_chrdev_region_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["alloc_chrdev_region"] = hi;
}

/* void my_func_unregister_chrdev_region_pre(unsigned long long from, 
 *	unsigned count, 
 *      struct my_struct *ls)
 * void my_func_unregister_chrdev_region_post(struct my_struct *ls) */
static void 
build_handler_decls_unregister_chrdev_region(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(void_type_node); /* return type */
	hi.ts.push_back(long_long_unsigned_type_node); /* arg1 */
	hi.ts.push_back(unsigned_type_node); /* arg2 */
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1], hi.ts[2], 
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_unregister_chrdev_region_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_unregister_chrdev_region_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["unregister_chrdev_region"] = hi;
}

/* void my_func_strlen_pre(const char *s, struct my_struct *ls)
 * void my_func_strlen_post(unsigned long ret, struct my_struct *ls) 
 */
static void 
build_handler_decls_strlen(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(long_unsigned_type_node); /* return type */
	hi.ts.push_back(ptr_type_node); /* arg1 */
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1], 
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_strlen_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_strlen_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["strlen"] = hi;
}

/* void my_func_memcpy_pre(void *dest, const void *src, 
 * 	unsigned long count, struct my_struct *ls)
 * void my_func_memcpy_post(void *ret, struct my_struct *ls) */
static void 
build_handler_decls_memcpy(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(ptr_type_node); /* return type */
	hi.ts.push_back(ptr_type_node); /* arg1 */
	hi.ts.push_back(ptr_type_node); /* arg2 */
	hi.ts.push_back(long_unsigned_type_node); /* arg3 */
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1], hi.ts[2], hi.ts[3],
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_memcpy_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_memcpy_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["memcpy"] = hi;
}
/* void my_func_memset_pre(void *s, int c, unsigned long count, 
 * 	struct my_struct *ls)
 * void my_func_memset_post(void *ret, struct my_struct *ls) */
static void 
build_handler_decls_memset(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(ptr_type_node); /* return type */
	hi.ts.push_back(ptr_type_node); /* arg1 */
	hi.ts.push_back(integer_type_node); /* arg2 */
	hi.ts.push_back(long_unsigned_type_node); /* arg3 */
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1], hi.ts[2], hi.ts[3],
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_memset_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_memset_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["memset"] = hi;
}

/* void my_func_snprintf_pre(char *s, unsigned long n, 
 *	const char *format, struct my_struct *ls)
 * void my_func_snprintf_post(int ret, struct my_struct *ls) */
static void 
build_handler_decls_snprintf(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(integer_type_node); /* return type */
	hi.ts.push_back(ptr_type_node); /* arg1 */
	hi.ts.push_back(long_unsigned_type_node); /* arg2 */
	hi.ts.push_back(ptr_type_node); /* arg3 */
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1], hi.ts[2], hi.ts[3],
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_snprintf_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_snprintf_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["snprintf"] = hi;
}

/* void my_func_kmalloc_pre(unsigned long size, unsigned int flags, 
 * 	struct my_struct *ls)
 * void my_func_kmalloc_post(void *ret, struct my_struct *ls) 
 * [NB] The handlers are the same for both kmalloc and kzalloc. */
static void 
build_handler_decls_kmalloc(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(ptr_type_node); /* return type */
	hi.ts.push_back(long_unsigned_type_node); /* arg1 */
	hi.ts.push_back(unsigned_type_node); /* arg2 */
		
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1], hi.ts[2],
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_kmalloc_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_kmalloc_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["kmalloc"] = hi;
	handler_map["kzalloc"] = hi;
}

/* No pre/post handlers here yet, just a replacement function:
 * void * my_func___kmalloc_repl(size_t size, unsigned int flags, 
 *                               struct my_struct *ls) */
static void 
build_handler_decls___kmalloc(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(ptr_type_node); /* return type */
	hi.ts.push_back(size_type_node); /* arg1 */
	hi.ts.push_back(unsigned_type_node); /* arg2 */
		
	fntype = build_function_type_list(
		hi.ts[0] /* return type of the target function*/,
		hi.ts[1], hi.ts[2], 
		ptr_type_node /* ls */, NULL_TREE);
	hi.repl = build_fn_decl("my_func___kmalloc_repl", fntype);
	set_handler_decl_properties(hi.repl);
	
	handler_map["__kmalloc"] = hi;
}

/* void my_func_kfree_pre(void *addr, struct my_struct *ls)
 * void my_func_kfree_post(struct my_struct *ls) */
static void 
build_handler_decls_kfree(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(void_type_node); /* return type */
	hi.ts.push_back(ptr_type_node); /* arg1 */

	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1],
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_kfree_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_kfree_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map["kfree"] = hi;
}
/* ====================================================================== */

/* Handlers for indirect calls. */

/* void my_func_call_pvoid_ulong_pre(unsigned long arg0, 
 * 	void *callee, struct my_struct *ls)
 * void my_func_call_pvoid_ulong_post(void *ret, struct my_struct *ls) */
static void 
build_handler_decls_call_pvoid_ulong(void)
{
	tree fntype;
	struct HandlerInfo hi;

	/* Type sequence for the target function. */
	hi.ts.push_back(ptr_type_node); /* return type */
	hi.ts.push_back(long_unsigned_type_node); /* arg0 */
	
	if (get_handlers_by_type_seq(hi.ts) != NULL) {
		fprintf(stderr, "build_handler_decls_call_pvoid_ulong: "
			"handlers for this type sequence already exist.\n");
		exit(EXIT_FAILURE);
		// TODO: resolve this situation automatically: make the 
		// existing handlers check for the function to be handled
		// here as well.
	}
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1],
		ptr_type_node /* callee */,
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_call_pvoid_ulong_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_call_pvoid_ulong_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map_by_types.push_back(hi);
}

/* void my_func_call_void_pvoid_int_pre(void *arg0, int arg1, 
 * 	void *callee, struct my_struct *ls)
 * void my_func_call_void_pvoid_int_post(struct my_struct *ls) */
static void 
build_handler_decls_call_void_pvoid_int(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	/* Type sequence for the target function. */
	hi.ts.push_back(void_type_node); /* return type */
	hi.ts.push_back(ptr_type_node); /* arg0 */
	hi.ts.push_back(integer_type_node); /* arg1 */
	
	if (get_handlers_by_type_seq(hi.ts) != NULL) {
		fprintf(stderr, "build_handler_decls_call_void_pvoid_int: "
			"handlers for this type sequence already exist.\n");
		exit(EXIT_FAILURE);
		// TODO: resolve this situation automatically: make the 
		// existing handlers check for the function to be handled
		// here as well.
	}
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[1], hi.ts[2],
		ptr_type_node /* callee */,
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_call_void_pvoid_int_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_call_void_pvoid_int_post", fntype);
	set_handler_decl_properties(hi.post);

	handler_map_by_types.push_back(hi);
}

/* void my_func_call_int_void_pre(void *callee, struct my_struct *ls)
 * void my_func_call_int_void_post(int ret, struct my_struct *ls) */
static void 
build_handler_decls_call_int_void(void)
{
	tree fntype;
	struct HandlerInfo hi;

	/* Type sequence for the target function. */
	hi.ts.push_back(integer_type_node); /* return type */
	/* the target function has no arguments */
	
	if (get_handlers_by_type_seq(hi.ts) != NULL) {
		fprintf(stderr, "build_handler_decls_call_int_void: "
			"handlers for this type sequence already exist.\n");
		exit(EXIT_FAILURE);
		// TODO: resolve this situation automatically: make the 
		// existing handlers check for the function to be handled
		// here as well.
	}
	
	fntype = build_function_type_list(void_type_node /* return type */,
		ptr_type_node /* callee */,
		ptr_type_node /* ls */, NULL_TREE);
	hi.pre = build_fn_decl("my_func_call_int_void_pre", fntype);
	set_handler_decl_properties(hi.pre);
	
	fntype = build_function_type_list(void_type_node /* return type */,
		hi.ts[0],
		ptr_type_node /* ls */, NULL_TREE);
	hi.post = build_fn_decl("my_func_call_int_void_post", fntype);
	set_handler_decl_properties(hi.post);
	
	handler_map_by_types.push_back(hi);
}
/* ====================================================================== */

enum EMemHandlers {
	MH_READ1 = 0,
	MH_READ2,
	MH_READ4,
	MH_READ8,
	MH_READ16,
	MH_WRITE1,
	MH_WRITE2,
	MH_WRITE4,
	MH_WRITE8,
	MH_WRITE16,
	MH_NUM_HANDLERS
};

static tree memory_event_handlers[MH_NUM_HANDLERS];

static void 
build_memory_event_decls(void)
{
	tree fntype;
	
	/* void my_func_{read|write}N(void *addr, struct my_struct *ls). */
	fntype = build_function_type_list(void_type_node /* return type */,
		ptr_type_node /* addr */,
		ptr_type_node /* ls */, NULL_TREE);
	
	memory_event_handlers[MH_READ1] = 
		build_fn_decl("my_func_read1", fntype);
	memory_event_handlers[MH_READ2] = 
		build_fn_decl("my_func_read2", fntype);
	memory_event_handlers[MH_READ4] = 
		build_fn_decl("my_func_read4", fntype);
	memory_event_handlers[MH_READ8] = 
		build_fn_decl("my_func_read8", fntype);
	memory_event_handlers[MH_READ16] = 
		build_fn_decl("my_func_read16", fntype);
		
	memory_event_handlers[MH_WRITE1] = 
		build_fn_decl("my_func_write1", fntype);
	memory_event_handlers[MH_WRITE2] = 
		build_fn_decl("my_func_write2", fntype);
	memory_event_handlers[MH_WRITE4] = 
		build_fn_decl("my_func_write4", fntype);
	memory_event_handlers[MH_WRITE8] = 
		build_fn_decl("my_func_write8", fntype);
	memory_event_handlers[MH_WRITE16] = 
		build_fn_decl("my_func_write16", fntype);

	for (unsigned int i = 0; i < MH_NUM_HANDLERS; ++i) {
		set_handler_decl_properties(memory_event_handlers[i]);
	}
}

tree
get_memory_event_decl(unsigned int size, bool is_write)
{
	enum EMemHandlers mh;
	if (size == 0)
		return NULL_TREE;
	
	if (size == 1)
		mh = is_write ? MH_WRITE1 : MH_READ1;
	else if (size <= 3)
		mh = is_write ? MH_WRITE2 : MH_READ2;
	else if (size <= 7)
		mh = is_write ? MH_WRITE4 : MH_READ4;
	else if (size <= 15)
		mh = is_write ? MH_WRITE8 : MH_READ8;
	else 
		mh = is_write ? MH_WRITE16 : MH_READ16;
	
	return memory_event_handlers[mh];
}
/* ====================================================================== */

void
build_handler_decls(void)
{
	tree fntype;
	
	/* my_struct * my_func_dummy_entry(func, nargs, &args[]) */
	fntype = build_function_type_list(
		ptr_type_node /* type of the return value */,
		ptr_type_node, unsigned_type_node, ptr_type_node /* args */,
		NULL_TREE);
	entry_handler_decl = build_fn_decl("my_func_dummy_entry", fntype);
	set_handler_decl_properties(entry_handler_decl);
	
	/* void my_func_dummy_exit(my_struct *ls) */
	fntype = build_function_type_list(void_type_node, ptr_type_node, 
					  NULL_TREE);
	exit_handler_decl = build_fn_decl("my_func_dummy_exit", fntype);
	set_handler_decl_properties(exit_handler_decl);
	
	/* DECLs for the handlers of direct calls. */
	build_handler_decls_vmalloc();
	build_handler_decls_vfree();
	build_handler_decls_alloc_chrdev_region();
	build_handler_decls_unregister_chrdev_region();
	build_handler_decls_strlen();
	build_handler_decls_memcpy();
	build_handler_decls_memset();
	build_handler_decls_snprintf();
	build_handler_decls_kmalloc();
	build_handler_decls___kmalloc();
	build_handler_decls_kfree();
	
	/* DECLs for the handlers of indirect calls. */
	build_handler_decls_call_pvoid_ulong();
	build_handler_decls_call_void_pvoid_int();
	build_handler_decls_call_int_void();
	
	/* DECLs for the handlers memory events. */
	build_memory_event_decls();
}
/* ====================================================================== */
