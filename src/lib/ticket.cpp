#include "guiding_light/ticket.hpp"

std::map <user_snowflake, channel_snowflake> tickets;    // user-keyed tickets map
std::map <channel_snowflake, user_snowflake> ck_tickets; // channel-keyed tickets map
