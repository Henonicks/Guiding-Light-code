#include <temp_vc.h>

bool operator <(temp_vc vc1, temp_vc vc2) {
    long long crtr_id1 = vc1.creatorid;
    long long crtr_id2 = vc2.creatorid;
    return crtr_id1 < crtr_id2;
}

temp_vc get_temp_vc(std::string_view line) {
    temp_vc result_line;
    std::string channelid_line, guildid_line, creatorid_line, parentid_line;
    int i = 0;
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            channelid_line += line[i];
        }
        else {
            ++i;
            result_line.channelid = (dpp::snowflake)channelid_line;
            break;
        }
    }
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            guildid_line += line[i];
        }
        else {
            ++i;
            result_line.guildid = (dpp::snowflake)guildid_line;
            break;
        }
    }
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            creatorid_line += line[i];
        }
        else {
            ++i;
            result_line.creatorid = (dpp::snowflake)creatorid_line;
            break;
        }
    }
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            parentid_line += line[i];
        }
        else {
            ++i;
            break;
        }
    }
    result_line.guildid = (dpp::snowflake)parentid_line;
    return result_line;
}

std::map <channel_snowflake, temp_vc> temp_vcs;
std::map <user_snowflake, uint64_t> join_time;
std::map <user_snowflake, dpp::snowflake> vc_statuses;
std::map <user_snowflake, bool> no_temp_ping;
std::map <guild_snowflake, int> temp_vc_amount;
std::map <channel_snowflake, std::unordered_set <user_snowflake>> banned;
std::queue <temp_vc_query> temp_vcs_queue;