#ifndef JTC_DEFAULTS_H
#define JTC_DEFAULTS_H

#include <guidingLight/guiding_light.h>
#include "string_convert.h"

struct jtc_defaults {
    dpp::snowflake channelid;
    std::string name;
    long limit;
	int bitrate;
};

bool operator <(jtc_defaults jd1, jtc_defaults jd2);

jtc_defaults get_jtc_defs(std::string line);

extern std::map <channel_snowflake, jtc_defaults> jtc_default_values;

#endif
