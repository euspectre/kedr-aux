set(executable_name "test_join_text_index")
add_executable(${executable_name} test.cpp)

target_link_libraries(${executable_name} ${mist_name})

test_add_target(${executable_name})

add_test("core.join_text_index" ${executable_name})