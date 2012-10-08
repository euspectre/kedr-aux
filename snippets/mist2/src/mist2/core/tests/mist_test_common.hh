#ifndef MIST_TEST_COMMON
#define MIST_TEST_COMMON

#include <iostream>
#include <string>
#include <stdexcept>

static inline void assert_instantiation(const std::string& result_str,
    const std::string& expected_str)
{
    if(result_str != expected_str)
    {
        std::cout << "Expected template instantiation:" << std::endl;
        std::cout << expected_str << std::endl;
        std::cout << "<INSTANTIATON ENDS>" << std::endl;
        std::cout << "But it is:" << std::endl;
        std::cout << result_str << std::endl;
        std::cout << "<INSTANTIATON ENDS>" << std::endl;
        
        throw std::logic_error("Incorrect template instantiation");
    }
}

#endif /* MIST_TEST_COMMON */