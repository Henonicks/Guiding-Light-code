#include <jtc_defaults.h>

bool operator <(jtc_defaults jd1, jtc_defaults jd2) {
    return jd1.name.size() < jd2.name.size();
}

std::map <channel_snowflake, jtc_defaults> jtc_default_values;
