#ifndef JTC_VC_H
#define JTC_VC_H

#include "guidingLight/guiding_light.h"
#include "database.h"

struct jtc_vc {
    dpp::snowflake channel_id, guild_id;
};

bool operator <(jtc_vc vc1, jtc_vc vc2);

jtc_vc get_jtc_vc(std::string_view line);

extern std::map <channel_snowflake, jtc_vc> jtc_vcs;
extern std::map <channel_snowflake, dpp::channel> jtc_channels_map;
extern std::map <guild_snowflake, int8_t> jtc_vc_amount;

#endif
