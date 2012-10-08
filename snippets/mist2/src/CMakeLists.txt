# Create mist library

set(MIST2_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/mist2)
set(MIST2_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/mist2.build)

set(MIST2_NAME mist)

# Create build and install directories
file(MAKE_DIRECTORY ${MIST2_BUILD_DIR} ${MIST2_INSTALL_DIR})

message(STATUS "Build mist2 library")

# Configure
execute_process (
    COMMAND ${CMAKE_COMMAND} 
        -DCMAKE_INSTALL_PREFIX=${MIST2_INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=Release
        -DMIST_GENERATE_PARSER=ON
        "${CMAKE_CURRENT_SOURCE_DIR}/mist2"
    WORKING_DIRECTORY ${MIST2_BUILD_DIR}
    RESULT_VARIABLE mist_build_result
    OUTPUT_VARIABLE mist_build_output
    ERROR_VARIABLE  mist_build_output
)

if (NOT mist_build_result EQUAL 0)
    message ("Failed to configure mist2 library.")
    message ("CMake output:\n${mist_build_output}\n")
	message ("CMake result:\n${mist_build_result}\n")
    message (FATAL_ERROR "Unable to build mist2 library, aborting.")
endif ()

# The temporary build directory should be removed during 'make clean'
# Note that the temporary install directory must  survive as library
# will be necessary if the user wants to rebuild main tool
# after 'make clean' without reconfiguring.
	set_property (DIRECTORY APPEND PROPERTY 
	    ADDITIONAL_MAKE_CLEAN_FILES yaml-cpp.build
	)

# Make install
execute_process (
    COMMAND make install

    WORKING_DIRECTORY ${MIST2_BUILD_DIR}
    RESULT_VARIABLE mist_build_result
    OUTPUT_VARIABLE mist_build_output
    ERROR_VARIABLE  mist_build_output
)

if (NOT mist_build_result EQUAL 0)
    message ("Failed to install mist2 library.")
    message ("CMake output:\n${mist_build_output}\n")
	message ("CMake result:\n${mist_build_result}\n")
    message (FATAL_ERROR "Unable to install mist2 library, aborting.")
endif ()

message(STATUS "Build mist2 library - done")

# Aware current CMake process about library builded and its include files
add_library(${MIST2_NAME} STATIC IMPORTED)

set_target_properties(${MIST2_NAME} PROPERTIES
    IMPORTED_LOCATION "${MIST2_INSTALL_DIR}/lib/lib${MIST2_NAME}.a")

include_directories("${MIST2_INSTALL_DIR}/include")

########################################################################
# Create yaml-cpp library(similary)

set(YAML_CPP_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp")
set(YAML_CPP_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp.build")

set(YAML_CPP_NAME yaml-cpp)


# Create build and install directories
file(MAKE_DIRECTORY ${YAML_CPP_BUILD_DIR} ${YAML_CPP_INSTALL_DIR})

message(STATUS "Build yaml-cpp library")

# Configure
execute_process (
    COMMAND ${CMAKE_COMMAND} 
        -DCMAKE_INSTALL_PREFIX=${YAML_CPP_INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=Release
        "${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp"
    WORKING_DIRECTORY ${YAML_CPP_BUILD_DIR}
    RESULT_VARIABLE yaml_cpp_build_result
    OUTPUT_VARIABLE yaml_cpp_build_output
    ERROR_VARIABLE  yaml_cpp_build_output
)

if (NOT yaml_cpp_build_result EQUAL 0)
    message ("Failed to configure yaml-cpp library.")
    message ("CMake output:\n${yaml_cpp_build_output}\n")
	message ("CMake result:\n${yaml_cpp_build_result}\n")
    message (FATAL_ERROR "Unable to build yaml-cpp library, aborting.")
endif ()

# The temporary build directory should be removed during 'make clean'
# Note that the temporary install directory must  survive as library
# will be necessary if the user wants to rebuild main tool
# after 'make clean' without reconfiguring.
	set_property (DIRECTORY APPEND PROPERTY 
	    ADDITIONAL_MAKE_CLEAN_FILES yaml-cpp.build
	)


# Make install
execute_process (
    COMMAND make install

    WORKING_DIRECTORY ${YAML_CPP_BUILD_DIR}
    RESULT_VARIABLE yaml_cpp_build_result
    OUTPUT_VARIABLE yaml_cpp_build_output
    ERROR_VARIABLE  yaml_cpp_build_output
)

if (NOT yaml_cpp_build_result EQUAL 0)
    message ("Failed to install yaml-cpp library.")
    message ("CMake output:\n${yaml_cpp_build_output}\n")
	message ("CMake result:\n${yaml_cpp_build_result}\n")
    message (FATAL_ERROR "Unable to install yaml-cpp library, aborting.")
endif ()

message(STATUS "Build yaml-cpp library - done")

# Aware current CMake process about library builded and its include files
add_library(${YAML_CPP_NAME} STATIC IMPORTED)

set_target_properties(${YAML_CPP_NAME} PROPERTIES
    IMPORTED_LOCATION "${YAML_CPP_INSTALL_DIR}/lib/lib${YAML_CPP_NAME}.a")

include_directories("${YAML_CPP_INSTALL_DIR}/include")

########################################################################
# Now build command-line tool itself

set(CMD_TOOL_NAME "mist-yaml")

add_executable("${CMD_TOOL_NAME}"
    "program.cpp")

target_link_libraries("${CMD_TOOL_NAME}" ${MIST2_NAME} ${YAML_CPP_NAME})