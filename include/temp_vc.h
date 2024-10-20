#ifndef TEMP_VC_H
#define TEMP_VC_H

#include <guidingLight/guiding_light.h>
#include <unordered_set>

struct temp_vc {
    dpp::snowflake channelid, guildid, creatorid, parentid;
};

bool operator <(temp_vc vc1, temp_vc vc2);

extern std::map <dpp::snowflake, temp_vc> temp_vcs;
extern std::map <dpp::snowflake, uint64_t> join_time;
extern std::map <dpp::snowflake, dpp::snowflake> vc_statuses;
extern std::map <dpp::snowflake, bool> no_temp_ping;
extern std::map <dpp::snowflake, int> temp_vc_amount;
extern std::map <dpp::snowflake, std::unordered_set <dpp::snowflake>> banned;

#endif
