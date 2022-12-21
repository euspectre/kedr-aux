cmake_minimum_required(VERSION 2.6)

enable_language(C)
enable_language(CXX)

project(coverage_tool)

set(tool_name "coverage_tool")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g")

# Create 'shipped' file <filename>.o from <filename>.
function(add_shipped filename)
	add_custom_command(OUTPUT "${filename}.o"
		COMMAND ld -o "${filename}.o" "--format=binary"
			"${filename}" "-r"
		DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${filename}"
	)
endfunction(add_shipped outfile infile)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/usage.in"
	"${CMAKE_CURRENT_BINARY_DIR}/usage")
add_shipped(usage)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/usage_operation.in"
	"${CMAKE_CURRENT_BINARY_DIR}/usage_operation")
add_shipped(usage_operation)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/usage_optimize_tests.in"
	"${CMAKE_CURRENT_BINARY_DIR}/usage_optimize_tests")
add_shipped(usage_optimize_tests)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/usage_stat.in"
	"${CMAKE_CURRENT_BINARY_DIR}/usage_stat")
add_shipped(usage_stat)

add_executable(${tool_name}
	"program.cpp"
	"usage.o"
	"usage_operation.o"
	"usage_optimize_tests.o"
	"usage_stat.o"
	
	"${CMAKE_CURRENT_BINARY_DIR}/trace_parser_base.tab.cc"
	"${CMAKE_CURRENT_BINARY_DIR}/trace_scanner.cc"
	"trace_simple.cpp"
	
	"${CMAKE_CURRENT_BINARY_DIR}/trace_parser_base.tab.hh"
	"${CMAKE_CURRENT_BINARY_DIR}/location.hh"
)

target_link_libraries(${tool_name}
	dl # Need for dlopen() and others
)

add_custom_command(
	OUTPUT "trace_parser_base.tab.cc" "trace_parser_base.tab.hh" "location.hh"
	COMMAND bison -Wall -d --location "${CMAKE_CURRENT_SOURCE_DIR}/trace_parser_base.yy"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/trace_parser_base.yy"
)

add_custom_command(
	OUTPUT "trace_scanner.cc"
	COMMAND flex -o "trace_scanner.cc" "${CMAKE_CURRENT_SOURCE_DIR}/trace_scanner.l"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/trace_scanner.l"
)

set_source_files_properties(
	"${CMAKE_CURRENT_BINARY_DIR}/trace_parser_base.tab.cc"
    PROPERTIES COMPILE_FLAGS "-Wno-parentheses")

set_source_files_properties(
	"${CMAKE_CURRENT_BINARY_DIR}/trace_scanner.cc"
    PROPERTIES COMPILE_FLAGS "-Wno-parentheses -Wno-unused-parameter")

# Testing support
enable_testing()

add_custom_target (check 
	COMMAND ${CMAKE_CTEST_COMMAND}
)
add_custom_target (build_tests)
add_dependencies (check build_tests)

macro(add_test_script test_name test_script)
    add_test(${test_name} "/bin/sh" ${test_script} ${ARGN})
endmacro(add_test_script test_name test_script)

macro (test_add_target target_name)
	set_target_properties (${target_name}
		PROPERTIES EXCLUDE_FROM_ALL true
	)
	add_dependencies (build_tests ${target_name})
endmacro (test_add_target target_name)

# Tests themselves
add_subdirectory(tests)
