cmake_minimum_required (VERSION 2.8)
#######################################################################

project (kmodule-test)

enable_language (C)
enable_language (CXX)
#######################################################################

# Prohibit a common type of an in-source build.
# Note that building in a subdirectory in the source tree is still allowed 
# as it can be convenient.
string (COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" in_source)
if (in_source)
    message (FATAL_ERROR 
"It is not allowed to build the project in its top source directory."
    )
endif () 
#######################################################################

# Currently, only GNU C compilers are supported.
if (NOT CMAKE_COMPILER_IS_GNUCC)
    message (FATAL_ERROR 
"Only GNU C compiler is currently supported."
    )
endif ()
#######################################################################

# Make "Release" the default build type
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release")
endif ()
message (STATUS "Build type is \"${CMAKE_BUILD_TYPE}\"")
#######################################################################

message (STATUS 
"The package is configured to be installed to ${CMAKE_INSTALL_PREFIX}"
)
#######################################################################

# Check if the compiler supports -fvisibility option (necessary to build 
# the shared library)
include (CheckCCompilerFlag)
check_c_compiler_flag(-fvisibility=hidden MIST_GCC_HAS_FVISIBILITY)
if (NOT MIST_GCC_HAS_FVISIBILITY)
    message(FATAL_ERROR 
"This version of GCC does not support \'-fvisibility=hidden\' option."
    )
endif ()
#######################################################################

# the plugin itself
add_subdirectory (src)

# the example based on my "kedr_sample_target" guinea pig to demonstrate
# the instrumentation
add_subdirectory (samples/sample_target)
#######################################################################
