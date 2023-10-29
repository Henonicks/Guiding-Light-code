#pragma once
#include <guidingLight/guiding_light.h>
#include <string_convert.h>

struct jtc_defaults {
    dpp::snowflake channelid;
    std::string name;
    int limit, bitrate;
};

bool operator <(jtc_defaults jd1, jtc_defaults jd2);

jtc_defaults get_jtc_defs(std::string line);

extern std::map <dpp::snowflake, jtc_defaults> jtc_default_values;