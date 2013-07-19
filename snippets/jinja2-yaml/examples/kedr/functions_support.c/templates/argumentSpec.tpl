<$if function.args$>
<$for arg in function.args$>
<$include 'arg_elem'$><$if not loop.last$>, <$endif$>
<$endfor$>
<$if function.ellipsis$>, ...<$endif$>
<$else$>
void
<$-endif$>
