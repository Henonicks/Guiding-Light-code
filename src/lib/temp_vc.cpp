#include <temp_vc.h>

bool operator <(temp_vc vc1, temp_vc vc2) {
    long long crtr_id1 = vc1.creatorid;
    long long crtr_id2 = vc2.creatorid;
    return crtr_id1 < crtr_id2;
}

std::map <dpp::snowflake, temp_vc> temp_vcs;
std::map <dpp::snowflake, uint64_t> join_time;
std::map <dpp::snowflake, dpp::snowflake> vc_statuses;