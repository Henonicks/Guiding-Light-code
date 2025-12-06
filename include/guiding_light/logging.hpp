#ifndef LOGGING_H
#define LOGGING_H

#include "guiding_light/guiding_light.hpp"
#include "guiding_light/database.hpp"
#include "guiding_light/cfg.hpp"

inline uint64_t last_error_message = -10;

/**
 * @brief Logs a message along with other logs from Discord.
 * @param _log The log to be printed.
 */
void bot_log(const dpp::log_t& _log);

/**
 * @brief Logs a message along with other logs of mine.
 * @param message The log to be printed.
 */
void log(std::string_view message);

/**
 * @brief Does the same as @ref log but also sends a message to a specific channel.
 * @param message The log to be printed.
 * @param human_readable The human-readable version of the error.
 */
void error_log(std::string_view message, std::string_view human_readable = "");

/**
 * @brief Logs a message briefly describing the guild the bot has joined/left.
 * @param message The log to be printed.
 */
void guild_log(std::string_view message);

/**
 * @brief Logs the cause of an exception from an SQL query.
 * @param e The exception.
 * @param function The function the query, along with the exception, came from.
 */
void sql_log(const sqlite::sqlite_exception& e, std::string_view function = {});

/**
 * @brief Prints the error of a request made by the bot.
 * @param callback The callback containing the error.
 * @return Whether the callback truly has an error.
 */
bool error_callback(const dpp::confirmation_callback_t& callback);

/**
 * @brief Prints the error of a request made by the bot.
 * @param callback The callback containing the error.
 * @param event The event to reply to.
 * @param lang The language to respond in (if available)
 * @param error_intro The part of the error message before
 * the actual error ("Error" by default).
 * @return Whether the callback truly has an error.
 */
bool error_feedback(const dpp::confirmation_callback_t& callback, const dpp::interaction_create_t& event,
					std::string_view lang, std::string_view error_intro = "Error");

/**
 * @brief Prints the error of a request made by the bot.
 * @param callback The callback containing the error.
 * @param event The event to reply to.
 * @param error_intro The part of the error message before
 * the actual error ("Error" by default).
 * @return Whether the callback truly has an error.
 */
bool error_feedback(const dpp::confirmation_callback_t& callback, const dpp::message_create_t& event,
					std::string_view error_intro = "Error");

#endif
