#ifndef LOGGING_H
#define LOGGING_H

#include "guiding_light/guiding_light.hpp"
#include "guiding_light/database.hpp"
#include "guiding_light/configuration.hpp"

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
 * @brief Logs a message briefly describing the guild the bot
 * has joined/left.
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
 */
void error_callback(const dpp::confirmation_callback_t& callback);

/**
 *
 * @param callback The callback containing the error.
 * @param event The event to reply to.
 * @param error_intro The part of the error message before
 * the actual error ("Error" by default).
 */
void error_feedback(const dpp::confirmation_callback_t& callback, const dpp::interaction_create_t& event, std::string_view error_intro = "Error");

/**
 *
 * @param callback The callback containing the error.
 * @param event The event to reply to.
 * @param error_intro The part of the error message before
 * the actual error ("Error" by default).
 */
void error_feedback(const dpp::confirmation_callback_t& callback, const dpp::message_create_t& event, std::string_view error_intro = "Error");

#endif
