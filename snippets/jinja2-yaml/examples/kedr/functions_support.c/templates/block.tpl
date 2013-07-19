/* Trampoline function for {{function.name}} */
static struct kedr_intermediate_info kedr_intermediate_info_{{function.name}};

<$if function.ellipsis$><$if function.original_code$>
#include <stdarg.h>
// Original variant of the function which takes 'va_list' argument.
static <$if function.returnType$>{{function.returnType}}<$else$>void<$endif$> kedr_orig_{{function.name}}(<$include 'argumentSpec_effective'$>)
{
<$if function.returnType$>
    {{function.returnType}} ret_val;
<$endif$>
    {{function.original_code | indent(4)}}
<$if function.returnType$>
    return ret_val;
<$endif$>
}
<$else$>
#error 'original_code' parameter should be non-empty for function with variable number of arguments.
<$endif$><$endif$>

/* Intermediate function itself */
static <$if function.returnType$>{{function.returnType}}<$else$>void<$endif$> kedr_intermediate_func_{{function.name}}(<$include 'argumentSpec'$>)
{
    struct kedr_function_call_info call_info;
<$if function.returnType$>
    {{function.returnType}} ret_val;
<$endif$>
    call_info.return_address = __builtin_return_address(0);
    
    // Call all pre-functions.
    if(kedr_intermediate_info_{{function.name}}.pre != NULL)
    {
        void (**pre_function)(<$include 'argumentSpec_comma'$>struct kedr_function_call_info* call_info);
        for(pre_function = (typeof(pre_function))kedr_intermediate_info_{{function.name}}.pre;
            *pre_function != NULL;
            ++pre_function)
        {
<$filter indent(12,true)$>
<$include 'argsCopy_declare'$>
(*pre_function)(<$include 'argumentList_comma'$>&call_info);
<$include 'argsCopy_finalize'$>
<$endfilter$><#Empty string for preserve spaces before '}'#>

        }
    }
    // Call replacement function
    if(kedr_intermediate_info_{{function.name}}.replace != NULL)
    {
        <$if function.returnType$>{{function.returnType}}<$else$>void<$endif$> (*replace_function)(<$include 'argumentSpec_comma'$>struct kedr_function_call_info* call_info) =
            (typeof(replace_function))kedr_intermediate_info_{{function.name}}.replace;
<$filter indent(8,true)$>        
<$include 'argsCopy_declare'$>
<$if function.returnType$>ret_val = <$endif$>replace_function(<$include 'argumentList_comma'$>&call_info);
<$include 'argsCopy_finalize'$>
<$endfilter$>

    }
    // .. or original one.
    else
    {
<$filter indent(8,true)$>
<$include 'argsCopy_declare'$>
<$if function.returnType$>ret_val = <$endif$><$if function.ellipsis$>kedr_orig_<$endif$>{{function.name}}(<$include 'argumentList'$>);
<$include 'argsCopy_finalize'$>
<$endfilter$>

    }
    // Call all post-functions.
    if(kedr_intermediate_info_{{function.name}}.post != NULL)
    {
        void (**post_function)(<$include 'argumentSpec_comma'$><$if function.returnType$><{{function.returnType}}, <$endif$>struct kedr_function_call_info* call_info);
        for(post_function = (typeof(post_function))kedr_intermediate_info_{{function.name}}.post;
            *post_function != NULL;
            ++post_function)
        {
<$filter indent(12,true)$>
<$include 'argsCopy_declare'$>
(*post_function)(<$include 'argumentList_comma'$><$if function.returnType$>ret_val, <$endif$>&call_info);
<$include 'argsCopy_finalize'$>
<$endfilter$>

        }
    }
<$if function.returnType$>
    return ret_val;
<$endif$>
}
