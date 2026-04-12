#ifndef GUIDING_LIGHT_H
#define GUIDING_LIGHT_H

#define cast static_cast

#include <unordered_set>

#include "no_warns/dpp/dpp.h"

#include <fmt/format.h>

#include "guiding_light/config_values.hpp"
#include "guiding_light/atomic.hpp"

using user_snowflake = dpp::snowflake;
using channel_snowflake = dpp::snowflake;
using guild_snowflake = dpp::snowflake;

inline atomic_ptr <dpp::cluster> bot, bot_dev, bot_release;
inline std::atomic <bool> bot_dev_is_starting, bot_release_is_starting, *bot_is_starting;

inline constexpr char DEFAULT_LANG[] = "en";

inline std::unordered_set <guild_snowflake> ready_guilds;
inline std::condition_variable guild_readiness_cv;

/**
 * @brief Block a thread to wait for a guild to be ready to have its channels worked on.
 * @param guild_id The guild to wait the readiness of.
 */
void wait_for_guild_readiness(dpp::snowflake guild_id);

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

enum exec_verdicts : bool {
	f_success,
	f_failure,
};

inline exec_verdicts exec_verdict;
inline std::atomic <bool> ready_to_explode;
inline std::recursive_mutex bomb_mutex;
inline std::condition_variable_any bomb_cv;

/**
 * @brief Blow self up and dump data, finishing execution.
 * @param failure Whether the program executed successfully or not.
 */
void explode(exec_verdicts failure = f_success);

/**
 * @brief Blow self up and signal the OS about abnormal behaviour.
 */
void explode_painfully();

/**
 * @brief Dump the database and the logs in the log channel and mark ready to explode.
 * @param deadlock Whether the program got stuck in a deadlock or not. If true, the function will stop waiting for the mutexes to be free and will dump instantly.
 */
dpp::coroutine <> dump_data(bool deadlock = false);

// TODO: document
dpp::coroutine <dpp::user> lookup_user(dpp::snowflake user_id);
dpp::coroutine <dpp::channel> lookup_channel(dpp::snowflake channel_id);
dpp::coroutine <dpp::role> lookup_role(dpp::snowflake role_id, dpp::snowflake guild_id);
dpp::coroutine <dpp::guild> lookup_guild(dpp::snowflake guild_id);
dpp::coroutine <dpp::emoji> lookup_emoji(dpp::snowflake emoji_id);
dpp::coroutine <dpp::emoji> lookup_emoji(dpp::snowflake emoji_id, dpp::snowflake guild_id);
dpp::coroutine <dpp::guild_member> lookup_guild_member(dpp::snowflake guild_id, dpp::snowflake user_id);

#endif
