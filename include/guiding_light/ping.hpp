#ifndef PING_H
#define PING_H

#include <random>
#include <dpp/snowflake.h>

std::string random_response(const dpp::snowflake& user_id);

#endif
