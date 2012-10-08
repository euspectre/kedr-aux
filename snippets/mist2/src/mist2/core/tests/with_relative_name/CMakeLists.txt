set(executable_name "test_with_relative_name")
add_executable(${executable_name} test.cpp)

target_link_libraries(${executable_name} ${mist_name})

test_add_target(${executable_name})

add_test("core.with_relative_name" ${executable_name})