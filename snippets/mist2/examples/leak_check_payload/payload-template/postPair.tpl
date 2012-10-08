<$if function.handler_post$>    {
		.orig = (void*)&<$function.name$>,
		.pre  = (void*)&post_<$function.name$>
	},
<$endif$>