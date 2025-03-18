#ifndef TICKET_H
#define TICKET_H

#include <guidingLight/guiding_light.h>

struct ticket {
    dpp::snowflake user_id;     // the user who created the ticket
    dpp::snowflake channel_id;  // the channel that lets me chat with the user
};

ticket get_ticket(std::string_view line);

extern std::map <user_snowflake, channel_snowflake> tickets;    // user-keyed tickets map
extern std::map <channel_snowflake, user_snowflake> ck_tickets; // channel-keyed tickets map

#endif