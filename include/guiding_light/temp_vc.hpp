#ifndef TEMP_VC_H
#define TEMP_VC_H

#include "guiding_light/guiding_light.hpp"
#include <unordered_set>

using temp_id_t = int;

struct temp_vc {
	temp_id_t id;
	dpp::snowflake channel_id, guild_id, creator_id, parent_id;
	[[nodiscard]] bool exists() const;
};

bool operator <(temp_vc vc1, temp_vc vc2);

struct temp_vc_query {
	temp_id_t id;
	dpp::snowflake user_id, channel_id, guild_id;
};

inline std::unordered_map <channel_snowflake, temp_vc> temp_vcs;
inline std::unordered_map <user_snowflake, uint64_t> join_time;
inline std::unordered_map <user_snowflake, std::unordered_map <guild_snowflake, channel_snowflake>> vc_statuses; // TODO: support multiple VCs at the same time
inline std::unordered_map <user_snowflake, bool> no_temp_ping;
inline std::unordered_map <guild_snowflake, int> temp_vc_amount;
inline std::recursive_mutex temp_vc_mutex;
inline std::condition_variable temp_vc_queue_cv;

enum restrictions_types : bool {
	RRT_BLOCKLIST,
	RRT_MUTELIST,
};

using restrictions_list_t = std::map <channel_snowflake, std::unordered_set <user_snowflake>>;

inline restrictions_list_t banned; // yep you're banned
inline restrictions_list_t muted; // GET MUTED GET MUTED
inline std::recursive_mutex restriction_mutex;

restrictions_list_t* get_restrictions_list(restrictions_types rest_type);
dpp::permission get_restriction_permissions(restrictions_types rest_type);

inline std::queue <temp_vc_query> temp_vcs_queue;

#endif
