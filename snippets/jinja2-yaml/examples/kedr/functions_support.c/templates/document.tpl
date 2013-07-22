/*
 * Implementation of intermediate functions for concrete functions set.
 */

#include <kedr/core/kedr_functions_support.h>
#include <kedr/core/kedr.h> /* only for types definition */

<$for header in headers$>
{{header}}
<$endfor$>
<$for function in functions if function.ellipsis$>
<$if loop.first$>
#include <stdarg.h>
<$endif$>
<$endfor$>

<$if functions$><$for function in functions$>
<$include 'block'$>

<$endfor$><$endif$>

static struct kedr_intermediate_impl intermediate_impl[] =
{
<$if functions$><$for function in functions$>
<$include 'intermediate_impl'$>

<$endfor$><$endif$>// Terminating element
	{
		.orig = NULL,
	}
};

static struct kedr_functions_support functions_support =
{
	<$if compile_as_module$>.mod = THIS_MODULE,<$endif$>
	.intermediate_impl = intermediate_impl
};

<$if compile_as_module$>
#include <linux/module.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_AUTHOR("{{module.author}}");
MODULE_LICENSE("{{module.license}}");

static void __exit
{{module.name}}_cleanup_module(void)
{
	kedr_functions_support_unregister(&functions_support);
}

static int __init
{{module.name}}_init_module(void)
{
	return kedr_functions_support_register(&functions_support);
}

module_init({{module.name}}_init_module);
module_exit({{module.name}}_cleanup_module);
<$else$>int
functions_support_register(void)
{
	return kedr_functions_support_register(&functions_support);
}

void
functions_support_unregister(void)
{
	kedr_functions_support_unregister(&functions_support);
}
<$endif$>/*********************************************************************/
