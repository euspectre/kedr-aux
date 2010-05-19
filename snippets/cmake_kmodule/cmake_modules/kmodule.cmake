# kmodule_try_compile(RESULT_VAR bindir srcfile
#			[COMPILE_DEFINITIONS flags]
#			[OUTPUT_VARIABLE var])
# to be implemented...

# Similar to try_module in simplified form, but compile srcfile as
# kernel module, instead of user space program.


# kmodule_try_compile_wout(RESULT_VAR bindir srcfile out_var)
# is equivalent to 
# kmodule_try_compile(RESULT_VAR bindir srcfile OUTPUT_VARIABLE out_var)
function(kmodule_try_compile_wout RESULT_VAR bindir srcfile
	out_var)

try_compile(${RESULT_VAR} ${bindir}
				${CMAKE_SOURCE_DIR}/cmake_modules/kmodule_files
				kmodule_try_compile_target
				CMAKE_FLAGS "-DSRC_FILE:PATH=${srcfile}"
				OUTPUT_VARIABLE ${out_var})

endfunction(kmodule_try_compile_wout RESULT_VAR bindir srcfile out_var)

# kmodule_try_compile_wout_wcflags(RESULT_VAR bindir srcfile out_var cflags)
# is equivalent to 
# kmodule_try_compile(RESULT_VAR bindir srcfile COMPILE_DEFINITIONS cflags OUTPUT_VARIABLE out_var)
function(kmodule_try_compile_wout_wcflags RESULT_VAR bindir srcfile
	out_var my_cflags )

try_compile(${RESULT_VAR} ${bindir}
				${CMAKE_SOURCE_DIR}/cmake_modules/kmodule_files
				kmodule_try_compile_target
				CMAKE_FLAGS "-DSRC_FILE:PATH=${srcfile}" "-DCFLAGS:STRING=${my_cflags}"
				OUTPUT_VARIABLE ${out_var})

endfunction(kmodule_try_compile_wout_wcflags RESULT_VAR bindir srcfile
	out_var my_cflags)