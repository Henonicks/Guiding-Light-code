#ifndef JTC_DEFAULTS_H
#define JTC_DEFAULTS_H

#include "guidingLight/guiding_light.h"

struct jtc_defaults {
	dpp::snowflake channel_id;
	std::string name;
	int8_t limit;
	int16_t bitrate;
};

bool operator <(jtc_defaults jd1, jtc_defaults jd2);

extern std::map <channel_snowflake, jtc_defaults> jtc_default_values;

#endif
