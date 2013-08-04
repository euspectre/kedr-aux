<$if function.args$>
<$if function.ellipsis$>
va_end(args_copy);
<$endif$>
<$if function.args_copy_destroy$>
{{function.args_copy_destroy}}
<$endif$>
<$endif$>