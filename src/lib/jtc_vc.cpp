#include <jtc_vc.h>

bool operator <(jtc_vc vc1, jtc_vc vc2) {
    unsigned long long c1 = vc1.channel_id, c2 = vc2.channel_id;
    return c1 < c2;
}

std::map <channel_snowflake, jtc_vc> jtc_vcs;
std::map <channel_snowflake, dpp::channel> jtc_channels_map;
std::map <guild_snowflake, int8_t> jtc_vc_amount;
