<$if function.args$>
<$if function.ellipsis$>
va_list args_copy;
<$endif$>
<$if function.args_copy_declare_and_init$>
{{function.args_copy_declare_and_init}}
<$-else$>
<$for arg in function.args$>
{{arg.type}} <$include 'argCopy_name'$> = {{arg.name}};
<$endfor$>
<$endif$>
<$if function.ellipsis$>
va_start(args_copy, {{function.last_arg}});
<$endif$>
<$endif$>