set(executable_name "test_if_joined_condition")
add_executable(${executable_name} test.cpp)

target_link_libraries(${executable_name} ${mist_name})

test_add_target(${executable_name})

add_test("core.if_joined_condition" ${executable_name})