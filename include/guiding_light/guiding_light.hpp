#ifndef GUIDING_LIGHT_H
#define GUIDING_LIGHT_H

#include <no_warns/dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>
#include <fmt/args.h>
#include "config_values.hpp"

using user_snowflake = dpp::snowflake;
using channel_snowflake = dpp::snowflake;
using guild_snowflake = dpp::snowflake;

extern dpp::cluster *bot, *bot_dev, *bot_release;
extern bool bot_dev_is_starting, bot_release_is_starting, *bot_is_starting;

/**
 * @brief Get the pointer to the bot for the mode we're on.
 * @return The pointer.
 */
dpp::cluster* get_bot();

/**
 * @brief Check if the bot for the mode we're on is running.
 * @return True if the bot is running, false otherwise.
 */
bool is_running();

/**
 * @brief Get the username of the bot for the mode we're on
 * @return The username.
 */
std::string bot_name();

#endif
