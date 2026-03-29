#ifndef TICKET_H
#define TICKET_H

#include "guiding_light/guiding_light.hpp"

inline std::unordered_map <user_snowflake, channel_snowflake> tickets;    // user-keyed tickets map
inline std::unordered_map <channel_snowflake, user_snowflake> ck_tickets; // channel-keyed tickets map
inline std::recursive_mutex ticket_mutex;

#endif