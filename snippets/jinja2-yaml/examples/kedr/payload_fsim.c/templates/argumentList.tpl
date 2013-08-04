<$if function.args$>
<$for arg in function.args$>
<$if not loop.first$>, <$endif$>
<$include 'argCopy_name'$>
<$endfor$>
<$if function.ellipsis$>, args_copy<$endif$>
<$endif$>