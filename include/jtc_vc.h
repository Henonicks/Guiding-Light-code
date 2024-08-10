#ifndef JTC_VC_H
#define JTC_VC_H

#include "guidingLight/guiding_light.h"

struct jtc_vc {
    dpp::snowflake channelid, guildid;
};

bool operator <(jtc_vc vc1, jtc_vc vc2);

jtc_vc get_jtc_vc(std::string line);

extern std::map <dpp::snowflake, jtc_vc> jtc_vcs;
extern std::map <dpp::snowflake, dpp::channel> jtc_channels_map;

#endif
