cmake_minimum_required (VERSION 2.6)

#######################################################################
# Name of the project. 
# The primary programming language is C++, C is necessary for some 
# CMake-specific stuff.
project (bs_mailer)

#######################################################################
# Prohibit a common type of an in-source build.
# Note that building in a subdirectory in the source tree is still allowed 
# as it can be convenient.
string (COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" in_source)
if (in_source)
    message (FATAL_ERROR 
"It is not allowed to build the project in its top source directory."
    )
endif (in_source) 

#######################################################################
# Make "Debug" the default build type
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Debug")
endif ()
message (STATUS "Build type is \"${CMAKE_BUILD_TYPE}\"")

#######################################################################
# Find VMime librarу

message (STATUS "Looking for VMime library")
find_library (VMIME NAMES vmime)
if (NOT VMIME)
    message (FATAL_ERROR 
"VMime library is not found on the system."
    )
endif ()
message (STATUS "VMime library: ${VMIME}")

string (REGEX REPLACE "/libvmime\\..*$" "" VMIME_LIB_DIR ${VMIME})
message (STATUS "VMime library is in directory ${VMIME_LIB_DIR}")

#######################################################################
# Subdirectories
add_subdirectory (src)

#######################################################################
