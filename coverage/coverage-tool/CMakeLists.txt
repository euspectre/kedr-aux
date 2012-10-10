cmake_minimum_required(VERSION 2.6)

enable_language(C)
enable_language(CXX)

project(coverage_tool)

set(tool_name "coverage_tool")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(${tool_name}
	"program.cpp"
	
	"${CMAKE_CURRENT_BINARY_DIR}/trace_parser_base.tab.cc"
	"${CMAKE_CURRENT_BINARY_DIR}/trace_scanner.cc"
	"trace.cpp"
	"trace_modifier.cpp"
	
	"${CMAKE_CURRENT_BINARY_DIR}/trace_parser_base.tab.hh"
	"${CMAKE_CURRENT_BINARY_DIR}/location.hh")

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
