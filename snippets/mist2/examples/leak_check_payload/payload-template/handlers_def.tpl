<$with function$><$! 
    Within 'with' scope template behave itself as old "block".
    That is, all 'join' constructions combine parameters only within one 'function' parameter.
    Also, it is possible to use relative names for refer to subparameters of 'function'.
    Relative names are start with '.'.
$>/* Interception of the <$.name$> function */
<$if .handler_pre$>void pre_<$.name$>(<$argumentSpec_comma$>struct kedr_function_call_info* call_info)
{
	void* caller_address = call_info->return_address;
<$.handler_pre$>
}
<$endif$><$if .handler_post$>void post_<$.name$>(<$argumentSpec_comma$><$if function.returnType$><$function.returnType$> ret_val, <$endif$>struct kedr_function_call_info* call_info)
{
	void* caller_address = call_info->return_address;
<$.handler_post$>
}
<$endif$><$endwith$>