# Needed for all CMakeList files, otherwise cmake will warn
cmake_minimum_required (VERSION 2.6)

enable_language(C)
# Include definition of kmodule_try_compile_* functions
include(${CMAKE_SOURCE_DIR}/cmake_modules/kmodule.cmake)
# Macro to verify,
# that kernel modules can be compiled on this system.
macro(kmodule_is_supported)
	kmodule_try_compile_wout(KMODULE_IS_SUPPORTED_VAR
					${CMAKE_CURRENT_BINARY_DIR}/check_kmodule_support
					${CMAKE_SOURCE_DIR}/try_compiler/try_compile_module/hello.c
					out_var
					)

	if(NOT KMODULE_IS_SUPPORTED_VAR)
	message("output of compile process is \n\n${out_var}\n")
	message(FATAL_ERROR "Compiler unable to compile simple module.")
	endif(NOT KMODULE_IS_SUPPORTED_VAR)
endmacro(kmodule_is_supported)
# Macro to verify,
# that kernel version at least as specified in the arguments.
macro(kmodule_require_minversion VERSION_MAJOR VERSION_MINOR VERSION_MICRO)
	kmodule_try_compile_wout_wcflags(KMODULE_REQUIRE_MINVERSION_VAR
					${CMAKE_CURRENT_BINARY_DIR}/check_kernel_min_version
					${CMAKE_SOURCE_DIR}/try_compiler/check_minimum_version/hello.c
					out_var
					"-DVERSION_MAJOR=${VERSION_MAJOR} -DVERSION_MINOR=${VERSION_MINOR} -DVERSION_MICRO=${VERSION_MICRO}"
					)
	if(NOT KMODULE_REQUIRE_MINVERSION_VAR)
	message("output of compile process is \n\n${out_var}\n")
	message(FATAL_ERROR "Project cannot be built with kernel version less than ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.")
	endif(NOT KMODULE_REQUIRE_MINVERSION_VAR)
endmacro(kmodule_require_minversion VERSION_MAJOR VERSION_MINOR VERSION_MICRO)

kmodule_is_supported()
kmodule_require_minversion(2 6 28)

set(MODULE_NAME hello)
# Set names(with paths) for some used files
set(kofile ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.ko)
set(kosource ${CMAKE_CURRENT_BINARY_DIR}/hello.c)
# Source of ${kosource} file in src-tree
set(kossource ${CMAKE_CURRENT_SOURCE_DIR}/hello.c)

set(KERNELDIR /lib/modules/$\(shell uname -r\)/build)
set(PWD $\(shell pwd\))
# 'make all' should build ${kofile}
add_custom_target(hello ALL DEPENDS ${kofile})
# Rule for build ${kofile} from ${kosource}
add_custom_command(OUTPUT ${kofile}
					COMMAND $(MAKE) -C ${KERNELDIR} M=${PWD} modules
					MAIN_DEPENDENCY ${kosource})
# Rule for get source file in build-tree instead of src-tree
add_custom_command(OUTPUT ${kosource}
					COMMAND cp -p ${kossource} ${kosource}
					MAIN_DEPENDENCY ${kossource})
# Create Kbuild file(makefile in kbuild process)
configure_file(Kbuild.in ${CMAKE_CURRENT_BINARY_DIR}/Kbuild)