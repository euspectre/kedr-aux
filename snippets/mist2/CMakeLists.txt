cmake_minimum_required (VERSION 2.6)

project (mist_tool)
enable_language (C)
enable_language (CXX)

#Create rule for obtain one file by copying another one
function(rule_copy_file target_file source_file)
    add_custom_command(OUTPUT ${target_file}
                    COMMAND cp -p ${source_file} ${target_file}
                    DEPENDS ${source_file}
                    )
endfunction(rule_copy_file target_file source_file)

#Create rule for obtain file in binary tree by copiing it from source tree
function(rule_copy_source rel_source_file)
    rule_copy_file(${CMAKE_CURRENT_BINARY_DIR}/${rel_source_file} ${CMAKE_CURRENT_SOURCE_DIR}/${rel_source_file})
endfunction(rule_copy_source rel_source_file)


# Create target for testing 
enable_testing ()
add_custom_target (check 
    COMMAND ${CMAKE_CTEST_COMMAND}
)
add_custom_target (build_tests)
add_dependencies (check build_tests)

macro(test_add_target target_name)

    set_target_properties (${target_name}
        PROPERTIES EXCLUDE_FROM_ALL true
    )
	add_dependencies (build_tests ${target_name})
endmacro(test_add_target target_name)


macro(add_test_script test_name script)
    add_test("${test_name}" "/bin/sh" "${script}")
endmacro(add_test_script test_name script)


add_subdirectory(src)
# For tests
add_subdirectory(examples)