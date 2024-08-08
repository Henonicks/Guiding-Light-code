#include <string_convert.h>

long long string_to_ll(const std::string_view& s) {
    long long res = 0;
    for (long long i = 0; i < s.size(); i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            res += (s[i] - '0');
            res *= 10;
        }
    }
    res /= 10;
    if (s[0] == '-') {
        res *= (-1);
    }
    return res;
}

double string_to_double(const std::string_view& s) {
    double res = 0;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            res += (s[i] - '0');
            res *= 10;
        }
        if (s[i] == '.') {
            break;
        }
    }
    res /= 10;
    if (s[0] == '-') {
        res *= (-1);
    }
    return res;
}

std::string double_to_string(const double& n) {
    std::string res;
    res = std::to_string(std::floor(n));
    std::string temp;
    for (char x : res) {
        if (x != '.') {
            temp += x;
        }
        else {
            break;
        }
    }
    res = temp;
    return res;
}
