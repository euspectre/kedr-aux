/* Some of the functions here are based on the implementation of 
 * ThreadSanitizer from GCC 4.9 (gcc/tsan.c). */

#include <assert.h>
#include <string.h>
#include <gcc-plugin.h>
#include <plugin-version.h>

#include "common_includes.h"
#include "handlers.h"

//<>
#include <stdio.h> // for debugging
//<>
/* ====================================================================== */

#if BUILDING_GCC_VERSION <= 4008
#define ENTRY_BLOCK_PTR_FOR_FN(FN)	ENTRY_BLOCK_PTR_FOR_FUNCTION(FN)
#define EXIT_BLOCK_PTR_FOR_FN(FN)	EXIT_BLOCK_PTR_FOR_FUNCTION(FN)
#endif

/* ====================================================================== */

/* Use this to mark the functions to be exported from this plugin. The 
 * remaining functions will not be visible from outside of this plugin even
 * if they are not static (-fvisibility=hidden GCC option is used to achieve
 * this). */
#define PLUGIN_EXPORT __attribute__ ((visibility("default")))
/* ====================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* This symbol is needed for GCC to know the plugin is GPL-compatible. */
PLUGIN_EXPORT int plugin_is_GPL_compatible;

/* Plugin initialization function, the only function to be exported */
PLUGIN_EXPORT int
plugin_init(struct plugin_name_args *plugin_info,
	    struct plugin_gcc_version *version);

#ifdef __cplusplus
}
#endif
/* ====================================================================== */

#if BUILDING_GCC_VERSION >= 4008
# define add_referenced_var(var)
#endif
/* ====================================================================== */

/* Before GCC starts processing a compilation unit, create the necessary 
 * declarations. 
 *
 * Based on the work from grsecurity patch ("stackleak" detector)
 * http://grsecurity.net/test/grsecurity-3.0-3.13.9-201404062127.patch */
static void 
my_start_unit(void * /*gcc_data*/, void * /*user_data*/)
{
	build_handler_decls();
}
/* ====================================================================== */

// TODO

/* Returns non-zero if the current function should be instrumented, 0 
 * otherwise. */
static int
should_instrument(void)
{
	// TODO: In a real system, adjust this function as needed to filter
	// out the functions that should not be instrumented.
	static char my_prefix[] = "my_func";
	static size_t len = sizeof(my_prefix) - 1;
	
	/* Do not instrument my_func_*() functions (may be needed in case 
	 * they are not in a my_func*.* file). */
	if (strncmp(current_function_name(), my_prefix, len) == 0)
		return 0;
	
	return 1;
}
/* ====================================================================== */

/* The tree to copy the value of the function parameter from. */
static tree
tree_for_param_rhs(tree param)
{
	unsigned int j;
	tree ddef;
			
	/* If there is an SSA_NAME that is the default definition of the
	 * parameter, use it. */
	for (j = 1; j < num_ssa_names; ++j) {
		tree name = ssa_name(j);
		if (SSA_NAME_VAR(name) == param &&
		    SSA_NAME_IS_DEFAULT_DEF(name)) {
			return name;
		}
	}
	
	/* Use PARM_DECL as is for the objects of aggregate types passed
	 * by value. */
	if (AGGREGATE_TYPE_P(TREE_TYPE(param)))
		return param;
	
	/* Create an SSA_NAME which is the default definition of the 
	 * parameter and use it. 
	 * [NB] The GIMPLE_NOP statement that "defines" it must not belong
	 * to any basic block. */
	ddef = make_ssa_name(param, gimple_build_nop());
	SSA_NAME_IS_DEFAULT_DEF(ddef) = 1;
	return ddef;
}

static void
instrument_fentry(tree *ls_ptr)
{
	basic_block on_entry;
	gimple_stmt_iterator gsi;
	gimple g;
	gimple_seq seq = NULL;
	tree param;
	int nparams;

	tree array_type; /* type of the array to store the arguments */
	tree args_ptr;
	tree args_array;
	
	on_entry = single_succ(ENTRY_BLOCK_PTR_FOR_FN(cfun));
	gsi = gsi_start_bb(on_entry);
	
	nparams = 0;
	for (param = DECL_ARGUMENTS(current_function_decl);
	     param;
             param = DECL_CHAIN(param)) {
		++nparams;
	}
	
	//<>
	fprintf(stderr, "Number of parameters: %d\n", nparams);
	//<>
	
	if (nparams == 0) {
		/* Pass NULL as args to the entry handler if there are no
		 * arguments. */
		args_ptr = null_pointer_node;
	}
	else {
		unsigned int i = 0;
		
		/* void * __stored_args[nparams]; */
		array_type = build_array_type_nelts(ptr_type_node, nparams);
		// [NB] May use NULL instead of name.
		args_array = create_tmp_var(array_type, "__stored_arg_ptrs");
		add_referenced_var(args_array);
		mark_addressable(args_array);
		
		args_ptr = build_addr(args_array, current_function_decl);
		
		/* Copy the arguments to addressable local variables and 
		 * store the addresses of these in __stored_arg_ptrs[]. */
		for (param = DECL_ARGUMENTS(current_function_decl);
		     param;
		     param = DECL_CHAIN(param), ++i) {
			tree lhs;
			tree stored;
			
			/* stored = param */
			stored = create_tmp_var(TREE_TYPE(param), NULL);
			add_referenced_var(stored);
			TREE_THIS_VOLATILE(stored) = 1;
			
			g = gimple_build_assign(stored, 
						tree_for_param_rhs(param));
			gimple_set_location(g, cfun->function_start_locus);
			gimple_seq_add_stmt(&seq, g);
			
			/* __stored_arg_ptrs[i] */
			lhs = build4(ARRAY_REF, ptr_type_node, args_array,
				     build_int_cst(unsigned_type_node, i),
				     NULL_TREE, NULL_TREE);

			g = gimple_build_assign(lhs, 
				build_fold_addr_expr(stored));
			gimple_set_location(g, cfun->function_start_locus);
			gimple_seq_add_stmt(&seq, g);
		}
	}

	/* my_func_dummy_entry(nargs, &args[]) */
	g = gimple_build_call(get_entry_handler_decl(), 3,
			      build_fold_addr_expr(current_function_decl),
			      build_int_cst(unsigned_type_node, nparams), 
			      args_ptr);
	gimple_call_set_lhs(g, *ls_ptr);
	gimple_set_location(g, cfun->function_start_locus);
	gimple_seq_add_stmt(&seq, g);
	
	gsi_insert_seq_before (&gsi, seq, GSI_SAME_STMT);
	return;	
}

static void
instrument_fexit(tree *ls_ptr)
{
	location_t loc;
	basic_block at_exit;
	gimple_stmt_iterator gsi;
	gimple stmt;
	gimple g;
	edge e;
	edge_iterator ei;
	
	/* We need to place the call to our exit handler right before the 
	 * last operation in the function ("return" of some kind). Note that
	 * this is NOT right before the end node of the function. 
	 * 
	 * Let us find the exits from the function first (similar to what
	 * TSan does in GCC). */
	at_exit = EXIT_BLOCK_PTR_FOR_FN(cfun);
	FOR_EACH_EDGE(e, ei, at_exit->preds) {
		gsi = gsi_last_bb(e->src);
		stmt = gsi_stmt(gsi);
		/* Sanity check: the statement must be 'return' of some 
		 * kind. */
		gcc_assert(gimple_code(stmt) == GIMPLE_RETURN || 
			   gimple_call_builtin_p(stmt, BUILT_IN_RETURN));
		loc = gimple_location(stmt);
		
		/* Call: my_func_dummy_exit(ls) */
		g = gimple_build_call(get_exit_handler_decl(), 1, *ls_ptr);
		gimple_set_location(g, loc);
		gsi_insert_before(&gsi, g, GSI_SAME_STMT);
	}
	return;
}

static tree
make_copy_get_addr(tree var, gimple_seq *seq, location_t loc)
{
	gimple g;
	
	tree copy_var = make_ssa_name(TREE_TYPE(var), NULL);
	g = gimple_build_assign(copy_var, var);
	gimple_set_location(g, loc);
	gimple_seq_add_stmt(seq, g);
	
	tree new_var  = make_ssa_name(ptr_type_node, NULL);
	g = gimple_build_assign(new_var, build_fold_addr_expr(copy_var));
	gimple_set_location(g, loc);
	gimple_seq_add_stmt(seq, g);
	
	return new_var;
}

static tree
convert_if_needed(tree var, tree dest_type, gimple_seq *seq, location_t loc)
{
	gimple g;
	tree src_type = TREE_TYPE(var);
	
	if (!useless_type_conversion_p(dest_type, src_type)) {
		//<>
		fprintf(stderr, "[DBG] Adding type conversion.\n");
		//<>
		tree conv_var = make_ssa_name(dest_type, NULL);
		g = gimple_build_assign_with_ops(
			CONVERT_EXPR, conv_var, var, NULL);
		gimple_set_location(g, loc);
		gimple_seq_add_stmt(seq, g);
		
		var = conv_var;
	}
	return var;
}

/* Add the handlers for the function calls of interest.
 * 
 * [NB] 
 * The handlers get the following arguments:
 * 
 * Pre-handlers (direct calls):
 * 	(args exist) ? (args, ls) : (ls)
 * Pre-handlers (indirect calls):
 * 	(args exist) ? (args, callee, ls) : (callee, ls)
 * Post-handlers (direct and indirect calls):
 * 	(target returns void) ? (ls) : (ret, ls)
 * 
 * 'args' is the list of (non-variable) arguments of the target function,
 * 'callee' - the address of the called function,
 * 'ls' - pointer to the local storage. */
static void
instrument_function_call(gimple_stmt_iterator *gsi, tree *ls_ptr)
{
	gimple stmt = gsi_stmt(*gsi);
	const HandlerInfo* hi = NULL;
	gimple_seq seq = NULL;
	gimple g;
	tree ret_tmp;
	
	tree fntype;
	tree fndecl = gimple_call_fndecl(stmt);
	
	if (fndecl) { /* Direct call */
		const char *name = 
			IDENTIFIER_POINTER(DECL_NAME(fndecl));
		/* Needed when adding the handlers */
		fntype = TREE_TYPE(fndecl);
		hi = get_handlers_by_function_name(name, fndecl);
		
		//<>
		if (hi == NULL) {
			fprintf(stderr, "[DBG] No handlers found for \"%s()\"\n",
				name);
		}
		//<>
	}
	else { 	/* Indirect call  */
		fntype = gimple_call_fntype(stmt);
		if (!fntype) {
			fprintf(stderr, 
		"Failed to obtain type info for the function.\n");
			return;
		}
		
		hi = get_handlers_by_fntype(fntype);
		//<>
		if (hi == NULL) {
			fprintf(stderr, "[!] no handlers for this indirect call.\n");
		}
		//<>
	}
	
	if (hi == NULL)
		return;
	
	//<>
	location_t loc = gimple_location(stmt);
	fprintf(stderr, "[DBG] Processing call at %s, line %u, col %u.\n", 
		LOCATION_FILE(loc), LOCATION_LINE(loc), 
		LOCATION_COLUMN(loc));
	//<>
	
	/* Pre-handler */
	if (hi->pre) {
		/* Prepare the arguments, convert the values to the
		 * appropriate types if necessary. */
		tree arg_types = TYPE_ARG_TYPES(fntype);
		unsigned int i = 0; 
		vec<tree> args = vNULL;
		
		for (tree tp = arg_types; tp; tp = TREE_CHAIN(tp), ++i) {
			tree src_type = TREE_VALUE(tp);
			if (types_compatible_p(src_type, void_type_node))
				break;
			
			tree dest_type = hi->ts[i + 1];
			tree arg = gimple_call_arg(stmt, i);
			
			if (!POINTER_TYPE_P(src_type) && 
			    !INTEGRAL_TYPE_P(src_type)) 
			{
				tree new_arg = make_copy_get_addr(
					arg, &seq, gimple_location(stmt));
				args.safe_push(new_arg);
				continue;
			}
			
			arg = convert_if_needed(arg, dest_type, &seq, 
						gimple_location(stmt));
			args.safe_push(arg);
		}
		
		/* Pre-handlers for the indirect calls also get the address
		 * of the target function ('callee') as an argument, before 
		 * 'ls'. */
		if (!fndecl) {
			tree callee = gimple_call_fn(stmt);
			if (!callee) {
				fprintf(stderr, 
			"Failed to obtain the address of the function.\n");
				return;
			}
			args.safe_push(callee);
		}
		args.safe_push(*ls_ptr);
		g = gimple_build_call_vec(hi->pre, args);
		gimple_set_location(g, gimple_location(stmt));
		gimple_seq_add_stmt(&seq, g);
		
		gsi_insert_seq_before(gsi, seq, GSI_SAME_STMT);
	}
	
	/* Post-handler */
	if (hi->post) {
		vec<tree> args_post = vNULL;
		seq = NULL;
		
		tree ret_type = gimple_call_return_type(stmt);
		if (!ret_type) {
			fprintf(stderr, 
			"Failed to obtain the return type of a call.\n");
			return;
		}
		
		tree ret_val = NULL_TREE;
		if (!types_compatible_p(ret_type, void_type_node)) {
			ret_val = gimple_call_lhs(stmt);
			if (!ret_val) {
				/* The return value is ignored. Store it in 
				 * a temporary. */
				ret_tmp = create_tmp_var(ret_type, 
							 "__ret_tmp");
				add_referenced_var(ret_tmp);
				gimple_call_set_lhs(stmt, ret_tmp);
				ret_val = make_ssa_name(ret_tmp, stmt);
			}
		}
		
		if (ret_val) {
			tree tp = TREE_TYPE(ret_val);
			if (!POINTER_TYPE_P(tp) && !INTEGRAL_TYPE_P(tp)) {
				/* Make a copy and pass its address to the 
				 * handler instead. */
				tree ptr_ret = make_copy_get_addr(
					ret_val, &seq, 
					gimple_location(stmt));
				args_post.safe_push(ptr_ret);
			}
			else {
				ret_val = convert_if_needed(
					ret_val, hi->ts[0], &seq, 
					gimple_location(stmt));
				args_post.safe_push(ret_val);
			}
		}
		args_post.safe_push(*ls_ptr);
		
		g = gimple_build_call_vec(hi->post, args_post);
		gimple_set_location(g, gimple_location(stmt));
		gimple_seq_add_stmt(&seq, g);
		
		gsi_insert_seq_after(gsi, seq, GSI_CONTINUE_LINKING);
	}
	
	/* Replacement of the function. 
	 * The new function has the same return type and has 'ls' appended
	 * to the list of its arguments. */
	if (hi->repl) {
		tree lhs = gimple_call_lhs(stmt);
		vec<tree> args = vNULL;
		unsigned int nargs = gimple_call_num_args(stmt);
		
		for (unsigned int i = 0; i < nargs; ++i) {
			args.safe_push(gimple_call_arg(stmt, i));
		}
		args.safe_push(*ls_ptr);
		
		g = gimple_build_call_vec(hi->repl, args);
		gimple_set_location(g, gimple_location(stmt));
		
		if (lhs) 
			gimple_call_set_lhs(g, lhs);
		
		gsi_replace(gsi, g, false);
	}
}

/* This function was taken from the implementation of TSan in GCC 4.9 
 * (gcc/tsan.c), with several modifications. */
static void
instrument_expr(gimple_stmt_iterator gsi, tree expr, bool is_write, 
		tree *ls_ptr)
{
	gimple stmt = gsi_stmt(gsi);
	gimple_seq seq = NULL;
	gimple g;

	HOST_WIDE_INT size = int_size_in_bytes(TREE_TYPE (expr));
	if (size < 1)
		return;

	/* TODO: Check how this works when bit fields are accessed, update 
	 * if needed (~ report touching the corresponding bytes as a 
	 * whole?) */
	HOST_WIDE_INT bitsize;
	HOST_WIDE_INT bitpos;
	tree offset;
	enum machine_mode mode;
	int volatilep = 0;
	int unsignedp = 0;
	tree base = get_inner_reference(
		expr, &bitsize, &bitpos, &offset, &mode, &unsignedp, 
		&volatilep, false);

	/* No need to instrument accesses to decls that don't escape,
	 * they can't escape to other threads then. 
	 * 
	 * [?] Looks like (gcc/passes.def) IPA passes come after "einline" 
	 * pass, so we may need another pass to use the results of IPA 
	 * analysis. This is because most of the instrumentation is done 
	 * here before "einline" pass. */
	if (DECL_P(base)) {
		struct pt_solution pt;
		memset(&pt, 0, sizeof(pt));
		pt.escaped = 1;
		pt.ipa_escaped = flag_ipa_pta != 0;
		pt.nonlocal = 1;
		if (!pt_solution_includes(&pt, base)) {
			//<>
			fprintf(stderr, "[DBG] The decl does not escape.\n");
			//<>
			return;
		}
		if (!is_global_var(base) && !may_be_aliased(base)) {
			//<>
			fprintf(stderr, "[DBG] Neither global nor may be aliased.\n");
			//<>
			return;
		}
	}

	if (TREE_READONLY (base) || 
	   (TREE_CODE (base) == VAR_DECL && DECL_HARD_REGISTER (base))) {
		//<>
		fprintf(stderr, "[DBG] Read-only or register variable.\n");
		//<>
		return;
	}

	// TODO: bit field access. How to handle it properly?
	if (bitpos % (size * BITS_PER_UNIT) ||
	    bitsize != size * BITS_PER_UNIT) {
		return;
	}

	gcc_checking_assert(is_gimple_addressable (expr));
	tree expr_ptr = build_fold_addr_expr(unshare_expr(expr));

	if (!is_gimple_val(expr_ptr)) {
		g = gimple_build_assign(
			make_ssa_name(TREE_TYPE(expr_ptr), NULL),
			expr_ptr);
		expr_ptr = gimple_assign_lhs(g);
		gimple_set_location(g, gimple_location(stmt));
		gimple_seq_add_stmt(&seq, g);
	}

	/* call my_func_{read|write}N(addr, ls) */
	g = gimple_build_call(get_memory_event_decl(size, is_write), 2, 
			      expr_ptr, *ls_ptr);
	gimple_set_location(g, gimple_location(stmt));
	gimple_seq_add_stmt(&seq, g);
	gsi_insert_seq_before(&gsi, seq, GSI_SAME_STMT);
}

static void
instrument_gimple(gimple_stmt_iterator *gsi, tree *ls_ptr)
{
	gimple stmt;
	stmt = gsi_stmt (*gsi);
	
	if (is_gimple_call(stmt)) {
		instrument_function_call(gsi, ls_ptr);
	}
	else if (is_gimple_assign(stmt) && !gimple_clobber_p(stmt)) {
		if (gimple_store_p(stmt)) {
			tree lhs = gimple_assign_lhs(stmt);
			instrument_expr(*gsi, lhs, true, ls_ptr);
		}
		if (gimple_assign_load_p(stmt)) {
			tree rhs = gimple_assign_rhs1(stmt);
			instrument_expr(*gsi, rhs, false, ls_ptr);
		}
	}
}

/* Process the body of the function. */
static void
instrument_function(tree *ls_ptr)
{
	basic_block bb;
	gimple_stmt_iterator gsi;
	
	FOR_EACH_BB_FN (bb, cfun) {
		for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); 
		     gsi_next(&gsi)) {
			instrument_gimple(&gsi, ls_ptr);
		}
	}
}

/* The main function of the pass. Called for each function to be processed.
 */
static unsigned int
do_execute()
{
	tree ls_ptr; /* Pointer to the local storage struct. */
	
	if (!should_instrument()) {
		//<>
		fprintf(stderr, "[DBG] Skipping function \"%s\".\n",
			current_function_name());
		//<>
		return 0;
	}
	
	//<>
	fprintf(stderr, "[DBG] Processing function \"%s\".\n",
		current_function_name());
	
	// [NB] May use NULL instead of name.
	ls_ptr = create_tmp_var(ptr_type_node, "__local_storage_ptr");
	add_referenced_var(ls_ptr);
	mark_addressable(ls_ptr);
	// TREE_THIS_VOLATILE(ls_ptr) = 1;
	
	instrument_function(&ls_ptr); /* Keep it first. */
	instrument_fentry(&ls_ptr);
	instrument_fexit(&ls_ptr);
	

	/*
	 {
	 basic_block bb;
	 FOR_EACH_BB(bb) {
		for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
			gimple stmt;
			stmt = gsi_stmt(gsi);
			
			print_gimple_stmt(stdout, stmt, 0, 
					  TDF_VOPS|TDF_MEMSYMS);
			printf("\n");
		}
	}
	}
	*/
	// TODO
	return 0;
}
/* ====================================================================== */

#if BUILDING_GCC_VERSION >= 4009
static const struct pass_data my_pass_data = {
#else
static struct gimple_opt_pass my_pass = {
	.pass = {
#endif
		.type = 	GIMPLE_PASS,
		.name = 	"kedr-i13n",
#if BUILDING_GCC_VERSION >= 4008
		.optinfo_flags = OPTGROUP_NONE,
#endif
#if BUILDING_GCC_VERSION >= 4009
		.has_gate	= false,
		.has_execute	= true,
#else
		.gate = 	NULL,
		.execute =	do_execute, /* main function of the pass */
		.sub = 		NULL,
		.next = 	NULL,
		.static_pass_number = 0,
#endif
		.tv_id = 	TV_NONE,
		.properties_required = PROP_ssa | PROP_cfg,
		.properties_provided = 0,
		.properties_destroyed = 0,
		.todo_flags_start = 0,
		.todo_flags_finish = TODO_verify_all | TODO_update_ssa
#if BUILDING_GCC_VERSION < 4009
	}
#endif
};

#if BUILDING_GCC_VERSION >= 4009
namespace {
class my_pass : public gimple_opt_pass {
public:
	my_pass() 
	  : gimple_opt_pass(my_pass_data, g) 
	{}
	unsigned int execute() { return do_execute(); }
}; /* class my_pass */
}  /* anon namespace */
#endif

static struct opt_pass *make_my_pass(void)
{
#if BUILDING_GCC_VERSION >= 4009
	return new my_pass();
#else
	return &my_pass.pass;
#endif
}
/* ====================================================================== */

int
plugin_init(struct plugin_name_args *plugin_info,
	    struct plugin_gcc_version *version)
{
	struct register_pass_info pass_info;
	
	if (!plugin_default_version_check(version, &gcc_version))
		return 1;
	
	// TODO: help string for the plugin, etc.
	
	pass_info.pass = make_my_pass();
	pass_info.reference_pass_name = "ssa";
	/* consider only the 1st occasion of the reference pass */
	pass_info.ref_pass_instance_number = 1;
	pass_info.pos_op = PASS_POS_INSERT_AFTER;
	
	//<>
	/*printf("base: %s, full: %s, argc: %d\n", plugin_info->base_name,
		plugin_info->full_name, plugin_info->argc);
	printf("GCC basever: %s, datestamp: %s, devphase: %s, revision: %s\n",
		version->basever, version->datestamp, version->devphase,
		version->revision);*/
	//<>
	
	/* We need the callback to execute at the start of each compilation
	 * unit to create declarations of the needed functions. The calls
	 * to these functions will be inserted in the code later, during the
	 * GIMPLE pass. */
	register_callback(plugin_info->base_name, PLUGIN_START_UNIT, 
			  &my_start_unit, NULL);
	
	/* Register the pass */
	register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP,
			  NULL, &pass_info);
	return 0;
}
/* ====================================================================== */
