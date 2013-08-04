<$if function.args$>
<$for arg in function.args$>{{arg.type}} {{arg.name}}, <$endfor$>
<$if function.ellipsis$>va_list args, <$endif$>
<$endif$>