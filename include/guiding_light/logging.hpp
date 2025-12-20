#ifndef LOGGING_H
#define LOGGING_H

#include "guiding_light/guiding_light.hpp"
#include "guiding_light/database.hpp"
#include "guiding_light/cfg.hpp"

inline uint64_t last_error_message = -10;
inline constexpr int LOGS_MAX_SIZE = 8'000'000;
inline std::map <std::ofstream*, std::string> logfile_paths;
inline std::map <std::ofstream*, std::string> logfile_names;

/**
 * @brief Autodump a log file if it's about to be too big for upload on Discord.
 * @param logfile The pointer to the log file to autodump.
 */
void autodump(std::ofstream* logfile);

/**
 * @brief Open a log file, deleting its contents.
 * @param logfile The log file to open.
 */
void open_logfile(std::ofstream* logfile);

/**
 * @brief Dump a log file on Discord, back up locally upon fail through backup_logfile.
 * @param logfile The log file to dump.
 */
void dump_logfile(std::ofstream* logfile);

/**
 * @brief Locally back up a log file.
 * @param logfile The log file to back up.
 * @param logfile_content The content to write to the backup file.
 */
void backup_logfile(std::ofstream* logfile, std::string_view logfile_content);

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
 * @brief Prints the error of a request made by the bot in the logfile and a specified channel, optionally pinging a user.
 * @param callback The callback containing the error.
 * @param channel_id The ID of the channel to send the error in.
 * @param user_id The ID of the user to ping (default 0 for no ping).
 * @param error_intro The part of the error message before
 * the actual error ("Error" by default)
 * @return Whether the callback truly has an error.
 */
bool error_pingback(const dpp::confirmation_callback_t& callback, const channel_snowflake& channel_id,
	const user_snowflake& user_id = 0, std::string_view error_intro = "Error");

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
