#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "guiding_light/resources/ticket.hpp"

#include "henifig/henifig.hpp"

/**
 * @brief Builds a dpp::emoji from a henifig::value_map.
 * @param config The henifig::value_map to build from.
 * @return A dpp::emoji built from the henifig::value_map.
 */
dpp::emoji build_emoji(const henifig::value_map& config);

namespace cfg {
	inline henifig::config_t config, responses, slashcommands;
	inline std::recursive_mutex config_mutex;

	/**
	 * @brief Checks if sqlite3 is installed. If it's not, the program crashes.
	 */
	void check_sqlite3();

	/**
	 * @brief Reads the config and writes down the values.
	 */
	void read_config(std::string_view path);

	/**
	 * @brief Creates the needed directories and opens the log files.
	 */
	void init_logs();

	/**
	 * @brief Reads the database to initialise the guild channel side of it, cleaning up whatever
	 * entries don't exist on Discord anymore.
	 * @param channels The channels sent by Discord.
	 */
	dpp::coroutine <> init_restriction_lists(const std::vector <dpp::snowflake>& channels);

	/**
	 * @brief Connect to the database and initialise a bunch of bot-specific stuff.
	 */
	void init_bot();

	/**
	 * @brief Writes down slashcommands in certain maps to
	 * make their mentions easily accessible.
	 */
	void write_down_slashcommands();

	/**
	 * @brief Get a help message, based on the interface language of the user invoking /help.
	 * @param lang The interface language of the user.
	 * @param page The number of the page of the help message.
	 * @return A dpp::message object with the embeds attached, ready to be replied as a help message.
	 */
	dpp::message help_message(std::string_view lang, uint8_t page = 0);
}

#endif
