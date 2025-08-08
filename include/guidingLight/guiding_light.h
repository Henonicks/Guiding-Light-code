#ifndef GUIDING_LIGHT_H
#define GUIDING_LIGHT_H

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>
#include "config_values.h"

using user_snowflake = dpp::snowflake;
using channel_snowflake = dpp::snowflake;
using guild_snowflake = dpp::snowflake;


extern dpp::cluster *bot, *bot_dev, *bot_release;
extern bool bot_dev_is_starting, bot_release_is_starting, *bot_is_starting;

void get_bot(dpp::cluster*& bot);
bool is_running();
std::string bot_name();

#endif
