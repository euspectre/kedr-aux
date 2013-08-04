/*********************************************************************
 * Module: {{module.name}}
 *********************************************************************/
#include <linux/module.h>
#include <linux/init.h>

MODULE_AUTHOR("{{module.author}}");
MODULE_LICENSE("{{module.license}}");
/*********************************************************************/

#include <kedr/core/kedr.h>

#include <kedr/fault_simulation/fault_simulation.h>

<$for header in headers$>
{{header}}
<$endfor$>

<$for function in functions if function.ellipsis$>
<$if loop.first$>#include <stdarg.h><$endif$>
<$endfor$>

/*********************************************************************
 * Replacement functions
 *********************************************************************/
<$for function in functions$>
<$include 'block'$>

<$endfor$>
/*********************************************************************/

/* Replace pairs */
static struct kedr_replace_pair replace_pairs[] =
{
<$for function in functions$>
    {
        .orig = (void*)&{{function.name}},
        .replace = (void*)&kedr_repl_{{function.name}}
    },
<$endfor$>
    {
        .orig = NULL
    }
};

static struct kedr_payload payload = {
    .mod            = THIS_MODULE,

    .replace_pairs  = replace_pairs,
};
/*********************************************************************/
extern int functions_support_register(void);
extern void functions_support_unregister(void);

static void
{{module.name}}_cleanup_module(void)
{
    kedr_payload_unregister(&payload);
<$for function in functions | reverse if not function.fpoint.reuse_point$>
    kedr_fsim_point_unregister(fsim_point_<$include 'point_name'$>);
<$endfor$>
    functions_support_unregister();    
}

static int __init
{{module.name}}_init_module(void)
{
    struct sim_point_attributes* sim_point;
    int result;

    result = functions_support_register();
    if(result) return result;

    /* Register simulation points */
<$for function in functions if not function.fpoint.reuse_point$>
    fsim_point_<$include 'point_name'$> = kedr_fsim_point_register("<$include 'point_name'$>",
        "<$for fpoint_param in function.fpoint.params$><$if not loop.first$>, <$endif$>{{fpoint_param.type}}<$endfor$>");
    
    if(fsim_point_<$include 'point_name'$> == NULL)
    {
        pr_err("Failed to register simulation point %s\n", "<$include 'point_name'$>");
        result = -EINVAL;
        goto sim_point_<$include 'point_name'$>_err;
    }
<$endfor$>

    result = kedr_payload_register(&payload);
    if(result) goto payload_err;

    return 0;

payload_err:
<$for function in functions | reverse if not function.fpoint.reuse_point$>
    kedr_fsim_point_unregister(fsim_point_<$include 'point_name'$>);
sim_point_<$include 'point_name'$>_err:
<$endfor$>
    functions_support_unregister();    

    return result;
}

module_init({{module.name}}_init_module);
module_exit({{module.name}}_cleanup_module);
/*********************************************************************/
