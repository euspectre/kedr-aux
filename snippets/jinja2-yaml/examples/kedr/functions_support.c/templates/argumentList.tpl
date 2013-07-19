<$if function.args$>
<$for arg in function.args$>
<$include 'argCopy_name'$><$if not loop.last$>, <$endif$>
<$endfor$>
<$if function.ellipsis$>, args_copy<$endif$>
<$endif$>