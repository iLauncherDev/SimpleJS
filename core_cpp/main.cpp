#include <default.hpp>
#include <simplejs_cpp/variable>
#include <iostream>

extern "C" int SIMPLEJS_CPP_API stub()
{
    simplejs::variable my_variable = 10.0;
    simplejs::variable my_variable_2 = 20.0;

    my_variable = my_variable_2;

    double my_value = my_variable.get_variable_value<double>();

    my_variable = "hello world";

    char *my_string = my_variable.get_variable_value<char *>();

    std::cout << "my_value = " << my_value << ", my_string = " << my_string << "\n";

    return 0xDEAD;
}
