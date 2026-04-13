#ifndef TICKET_H
#define TICKET_H

#include "guiding_light/guiding_light.hpp"

struct ticket {
	dpp::snowflake user_id, channel_id, dm_channel_id;
};

bool operator <(const ticket& t1, const ticket& t2);

inline std::unordered_map <user_snowflake, ticket> tickets;    // user-keyed tickets map
inline std::unordered_map <channel_snowflake, ticket> ck_tickets; // channel-keyed tickets map
inline std::recursive_mutex ticket_mutex;

#endif