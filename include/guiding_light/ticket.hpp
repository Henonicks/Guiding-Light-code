#ifndef TICKET_H
#define TICKET_H

#include "guiding_light.hpp"

inline std::map <user_snowflake, channel_snowflake> tickets;    // user-keyed tickets map
inline std::map <channel_snowflake, user_snowflake> ck_tickets; // channel-keyed tickets map

#endif