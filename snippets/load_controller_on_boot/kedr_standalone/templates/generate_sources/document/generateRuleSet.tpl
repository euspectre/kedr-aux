trace_<$module.name$>.h: <$payload.datafile$>
	$(kedr_generator) $(templates_dir)/trace_payload.h $< > $@

<$module.name$>.c: <$payload.datafile$>
	$(kedr_generator) $(templates_dir)/payload.c $< > $@