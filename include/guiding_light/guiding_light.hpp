#ifndef GUIDING_LIGHT_H
#define GUIDING_LIGHT_H

#define cast static_cast

#include "no_warns/dpp/dpp.h"
#include <fmt/format.h>
#include "guiding_light/config_values.hpp"

using user_snowflake = dpp::snowflake;
using channel_snowflake = dpp::snowflake;
using guild_snowflake = dpp::snowflake;

inline dpp::cluster *bot, *bot_dev, *bot_release;
inline bool bot_dev_is_starting, bot_release_is_starting, *bot_is_starting;
inline const char* DEFAULT_LANG = "en";

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

#define get_lang() \
	const std::string_view lang = event.command.locale

/**
 * @brief Dump the database and the logs in the log channel and quit.
 * @param code The code to quit with.
 */
void dump_data(int code);

#endif
