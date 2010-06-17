set (CMAKE_INCLUDE_CURRENT_DIR ON)

# Create a variable called helloworld_SOURCES containing all .cpp files:
set (hello_world_SOURCES helloworld.cpp main.cpp)

# Create an executable file called helloworld from sources:
add_executable (hello_world ${hello_world_SOURCES})

# Add the subdirectory with the tests for the component
add_subdirectory (tests)