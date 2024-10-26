#ifndef TEMP_VC_H
#define TEMP_VC_H

#include <guidingLight/guiding_light.h>
#include <unordered_set>

struct temp_vc {
    dpp::snowflake channelid, guildid, creatorid, parentid;
};

bool operator <(temp_vc vc1, temp_vc vc2);

extern std::map <channel_snowflake, temp_vc> temp_vcs;
extern std::map <user_snowflake, uint64_t> join_time;
extern std::map <user_snowflake, dpp::snowflake> vc_statuses;
extern std::map <user_snowflake, bool> no_temp_ping;
extern std::map <guild_snowflake, int> temp_vc_amount;
extern std::map <channel_snowflake, std::unordered_set <user_snowflake>> banned; // yep you're banned

#endif
