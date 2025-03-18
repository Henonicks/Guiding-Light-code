#include "ticket.h"

std::map <user_snowflake, channel_snowflake> tickets;    // user-keyed tickets map
std::map <channel_snowflake, user_snowflake> ck_tickets; // channel-keyed tickets map

ticket get_ticket(std::string_view line) {
    ticket result_line;
    std::string userid_line, channelid_line;
    int i = 0;
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            userid_line += line[i];
        }
        else {
            ++i;
            result_line.user_id = (dpp::snowflake)userid_line;
            break;
        }
    }
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            channelid_line += line[i];
        }
        else {
            ++i;
            break;
        }
    }
    result_line.channel_id = (dpp::snowflake)channelid_line;
    return result_line;
}
