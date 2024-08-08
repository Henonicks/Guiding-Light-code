#ifndef STRING_CONVERT_H
#define STRING_CONVERT_H

#include <iostream>
#include <math.h>

long long string_to_ll(std::string_view s);
double string_to_double(std::string_view s);
std::string double_to_string(const double& n);

#endif
