set (PLUGIN_TARGET ${PROJECT_NAME}-shared)
set (PLUGIN_SOURCES
    kmodule-test.cpp
    handlers.cpp
    handlers.h
    common_includes.h
)

add_definitions(-Wall -Wextra -fno-rtti)

# Find the directory with the header files for GCC plugins
execute_process (
    COMMAND ${CMAKE_C_COMPILER} -print-file-name=plugin
    OUTPUT_VARIABLE plugin_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT plugin_dir)
    message (FATAL_ERROR "Failed to find GCC plugin directory")
endif ()

include_directories (
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${plugin_dir}/include"
)

set_source_files_properties (${PLUGIN_SOURCES} PROPERTIES
    COMPILE_FLAGS "-fvisibility=hidden"
)

add_library(${PROJECT_NAME} MODULE ${PLUGIN_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX ""
)
#######################################################################
