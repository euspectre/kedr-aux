<$if function.handler_pre$>    {
		.orig = (void*)&<$function.name$>,
		.pre  = (void*)&pre_<$function.name$>
	},
<$endif$>