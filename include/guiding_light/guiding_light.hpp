#ifndef GUIDING_LIGHT_H
#define GUIDING_LIGHT_H

#define cast static_cast

#include "no_warns/dpp/dpp.h"
#include <fmt/format.h>
#include "guiding_light/config_values.hpp"
#include "guiding_light/atomic.hpp"

using user_snowflake = dpp::snowflake;
using channel_snowflake = dpp::snowflake;
using guild_snowflake = dpp::snowflake;

inline atomic_ptr <dpp::cluster> bot, bot_dev, bot_release;
inline std::atomic <bool> bot_dev_is_starting, bot_release_is_starting, *bot_is_starting;
inline std::atomic <bool> already_prayed;

inline dpp::cluster *server_cluster;
inline std::thread *topgg_server_thread;
inline dpp::http_server *topgg_server;
inline std::recursive_mutex server_mutex;

inline constexpr char DEFAULT_LANG[] = "en";

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

#define get_rest_list() \
	auto* list = get_restrictions_list(rest_type)

enum fatality : bool {
	f_non_fatal,
	f_fatal,
};

/**
 * @brief Dump the database and the logs in the log channel and quit.
 * @param fatal Whether this function was called due to a fatal condition or not. If true, the function will call std::abort().
 * @param deadlock Whether the program got stuck in a deadlock or not. If true, the function will stop waiting for the mutexes to be free and will dump instantly.
 */
void dump_data(bool fatal = false, bool deadlock = false);

#endif
