#ifndef HANDLERS_H_1750_INCLUDED
#define HANDLERS_H_1750_INCLUDED

#include <vector>

/* Build the DECLs for the handler functions. Call this when processing
 * START_UNIT event. */
void build_handler_decls(void);

/* Getters for function entry/exit handlers. */
tree get_entry_handler_decl(void);
tree get_exit_handler_decl(void);

/* A sequence of types. The first element is the return type, the rest (if 
 * any) are the types of the arguments in the same order as they appear in
 * the definition of the function. If the target function has a variable
 * argument list, only the types known from the function's declaration are
 * listed here. */
typedef std::vector<tree> TypeSeq;

/* A pair of handlers for the function calls along with the type sequence
 * for the handled function plus the replacement for the function (NULL 
 * means the replacement is not needed). 
 * ts[0] - the return type, the rest, if any, are the types of the 
 * arguments. */
struct HandlerInfo {
	tree pre;
	tree post;
	tree repl;
	TypeSeq ts;
	
	HandlerInfo() : pre(NULL_TREE), post(NULL_TREE), repl(NULL_TREE) {}
};

/* Get the pair of handlers for a given function type. Returns NULL if not
 * found. Can be used in the handling of indirect calls among other things. 
 */
const HandlerInfo *
get_handlers_by_fntype(tree fntype);

/* Get the pair of handlers for the function with the given name and decl.
 * Returns NULL if not found. 'fndecl' is actually used to check if the
 * found handlers actually are compatible with the target function. */
const HandlerInfo *
get_handlers_by_function_name(const char *name, tree fndecl);

/* Returns the decl for a function that reports memory access of the given
 * type (read/write) when size bytes are accessed. */
tree
get_memory_event_decl(unsigned int size, bool is_write);

#endif /*HANDLERS_H_1750_INCLUDED*/
