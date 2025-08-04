#include <temp_vc.h>

bool operator <(temp_vc vc1, temp_vc vc2) {
    long long crtr_id1 = vc1.creator_id;
    long long crtr_id2 = vc2.creator_id;
    return crtr_id1 < crtr_id2;
}

std::map <channel_snowflake, temp_vc> temp_vcs;
std::map <user_snowflake, uint64_t> join_time;
std::map <user_snowflake, dpp::snowflake> vc_statuses;
std::map <user_snowflake, bool> no_temp_ping;
std::map <guild_snowflake, int> temp_vc_amount;
std::map <channel_snowflake, std::unordered_set <user_snowflake>> banned;
std::queue <temp_vc_query> temp_vcs_queue;