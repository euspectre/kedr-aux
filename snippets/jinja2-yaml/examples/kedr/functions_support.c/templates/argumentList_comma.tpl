<$if function.args$>
<$for arg in function.args$><$include 'argCopy_name'$>, <$endfor$>
<$if function.ellipsis$>args_copy, <$endif$>
<$endif$>