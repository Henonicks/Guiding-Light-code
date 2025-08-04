#ifndef TICKET_H
#define TICKET_H

#include <guidingLight/guiding_light.h>
 
extern std::map <user_snowflake, channel_snowflake> tickets;    // user-keyed tickets map
extern std::map <channel_snowflake, user_snowflake> ck_tickets; // channel-keyed tickets map

#endif