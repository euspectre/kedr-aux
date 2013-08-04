//***** Replacement for {{function.name}} *****//

<$if not function.fpoint.reuse_point$>
// Fault simulation point definition
static struct kedr_simulation_point* fsim_point_<$include 'point_name'$>;

<$if function.fpoint.params$>
struct fsim_point_data_<$include 'point_name'$>

{
<$for fpoint_param in function.fpoint.params$>
    {{fpoint_param.type}} {{fpoint_param.name}};
<$endfor$>
};
<$endif$>
<$endif$>
/*
 * 'Simulation' function: return non-zero if need to simulate fault and zero otherwise.
 * Note, that this function also set fault message if needed.
 */
static int kedr_simulate_{{function.name}}(<$include 'argumentSpec_comma'$>struct kedr_function_call_info* call_info)
{
    int __result;
    // Extract value of 'caller_address' from replacement function's parameters
    void* caller_address = call_info->return_address;

<$if function.fpoint.params$>
    struct fsim_point_data_<$include 'point_name'$> fsim_point_data;
<$endif$>

<$if function.prologue$>
    {{function.prologue | indent(4)}}
<$endif$>
<$for fpoint_param in function.fpoint.params$>
    fsim_point_data.{{fpoint_param.name}} = ({{fpoint_param.type}}) {{fpoint_param.name}};
<$endfor$>
    
    __result = kedr_fsim_point_simulate(fsim_point_<$include 'point_name'$>, <$if function.fpoint.params$>&fsim_point_data<$else$>NULL<$endif$>);
    if(__result)
    {
        kedr_fsim_fault_message("%s at [<%p>] %pS", "{{function.name}}", caller_address, caller_address);
    }
<$if function.epilogue$>
    {{function.epilogue | indent(4)}}
<$endif$>

    return __result;
}
// Fault variant of the function
static <$if function.returnType$>{{function.returnType}}<$else$>void<$endif$> kedr_fault_{{function.name}}(<$include 'argumentSpec_effective'$>)
{
<$if function.returnType$>
    {{function.returnType}} ret_val;
<$endif$>

    {{function.fpoint.fault_code | indent(4)}}
    
<$if function.returnType$>
    return ret_val;
<$endif$>
}

<$if function.ellipsis$><$if function.original_code$>
// Original variant of the function which takes 'va_list' argument.
static <$if function.returnType$>{{function.returnType}}<$else$>void<$endif$>
kedr_orig_{{function.name}}(<$include 'argumentSpec_effective'$>)
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

// Replacement function itself
static <$if function.returnType$>{{function.returnType}}<$else$>void<$endif$> kedr_repl_{{function.name}}(<$include 'argumentSpec_comma'$>struct kedr_function_call_info* call_info)
{
<$if function.returnType$>
    {{function.returnType}} ret_val;
<$endif$>
    int simulate_result;
    {
<$filter indent(8, True)$>
<$include 'argsCopy_declare'$>
simulate_result = kedr_simulate_{{function.name}}(<$include 'argumentList_comma'$>call_info);
<$include 'argsCopy_finalize'$>
<$endfilter$>
    
    }
    {
<$filter indent(8, True)$>
<$include 'argsCopy_declare'$>
if(simulate_result)
{
    <$if function.returnType$>ret_val = <$endif$>kedr_fault_{{function.name}}(<$include 'argumentList'$>);
}
else
{
    <$if function.returnType$>ret_val = <$endif$><$if function.ellipsis$>kedr_orig_<$endif$>{{function.name}}(<$include 'argumentList'$>);
}
<$include 'argsCopy_finalize'$>
<$endfilter$>

    }
<$if function.returnType$>
    return ret_val;
<$endif$>
}
