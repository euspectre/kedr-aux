# CMake version 2.6 or newer is required
cmake_minimum_required (VERSION 2.6)

########################################################################
# to_abs_path(output_var path [...])
#
# Convert relative path of file to absolute path:
# use path in source tree, if file already exist there.
# otherwise use path in binary tree.
# If initial path already absolute, return it.
macro(to_abs_path output_var)
	set(${output_var})
	foreach(path ${ARGN})
		string(REGEX MATCH "^/" _is_abs_path ${path})
		if(_is_abs_path)
			list(APPEND ${output_var} ${path})
		else(_is_abs_path)
            set (to_abs_path_file "to_abs_path_file-NOTFOUND")
			find_file(to_abs_path_file ${path}
				${CMAKE_CURRENT_SOURCE_DIR}
                NO_DEFAULT_PATH
            )
			if(NOT to_abs_path_file)
                set (to_abs_path_file "${CMAKE_CURRENT_BINARY_DIR}/${path}")
			endif(NOT to_abs_path_file)
            list(APPEND ${output_var} ${to_abs_path_file})
		endif(_is_abs_path)
	endforeach(path ${ARGN})
endmacro(to_abs_path output_var path)


########################################################################
# Test-related macros
########################################################################

# Enables testing support and perform other initialization tasks.
# This macro should usually be used in the top-level CMakeLists.txt file
# before add_subdirectory () calls.
macro (kedr_test_init)
    enable_testing ()
    add_custom_target (check 
        COMMAND ${CMAKE_CTEST_COMMAND}
    )
    add_custom_target (build_tests)
    add_dependencies (check build_tests)
endmacro (kedr_test_init)

# Use this macro to specify an additional target to be built before the tests
# are executed.
macro (kedr_test_add_target target_name)
    set_target_properties (${target_name}
        PROPERTIES EXCLUDE_FROM_ALL true
    )
    add_dependencies (build_tests ${target_name})
endmacro (kedr_test_add_target target_name)

# This function adds a test script (a Bash script, actually) to the set of
# tests for the package. The script may reside in current source or binary 
# directory (the source directory is searched first).
function (kedr_test_add_script test_name script_file)
    set (TEST_SCRIPT_FILE)
    to_abs_path (TEST_SCRIPT_FILE ${script_file})
        
    add_test (${test_name}
        /bin/bash ${TEST_SCRIPT_FILE} ${ARGN}
    )
endfunction (kedr_test_add_script)

########################################################################
# Name of the project.
project (HelloWorld)

# Initialize test-related stuff
kedr_test_init ()

# Subdirectories to be processed
add_subdirectory (sample) # main component
add_subdirectory (tests)  # "global" tests for the components of the system
