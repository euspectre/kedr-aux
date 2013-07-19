<$if function.args$>
<$for arg in function.args$><$include 'arg_elem'$>, <$endfor$>
<$if function.ellipsis$>va_arg args, <$endif$>
<$endif$>
