#ifndef TEMP_VC_H
#define TEMP_VC_H

#include "guiding_light.hpp"
#include <unordered_set>

struct temp_vc {
	dpp::snowflake channel_id, guild_id, creator_id, parent_id;
};

bool operator <(temp_vc vc1, temp_vc vc2);

struct temp_vc_query {
	dpp::user* usr;
	dpp::snowflake channel_id, guild_id;
};

inline std::map <channel_snowflake, temp_vc> temp_vcs;
inline std::map <user_snowflake, uint64_t> join_time;
inline std::map <user_snowflake, dpp::snowflake> vc_statuses;
inline std::map <user_snowflake, bool> no_temp_ping;
inline std::map <guild_snowflake, int> temp_vc_amount;
inline std::map <channel_snowflake, std::unordered_set <user_snowflake>> banned; // yep you're banned
inline std::queue <temp_vc_query> temp_vcs_queue;

#endif
