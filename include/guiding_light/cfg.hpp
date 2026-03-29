#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "guiding_light/commands.hpp"
#include "guiding_light/notification_channel.hpp"
#include "guiding_light/jtc_vc.hpp"
#include "guiding_light/jtc_defaults.hpp"
#include "guiding_light/logging.hpp"
#include "guiding_light/slash_funcs.hpp"
#include "guiding_light/topgg.hpp"
#include "guiding_light/ticket.hpp"
#include "guiding_light/ratelimit.hpp"

#include "color.hpp"

#include "henifig/henifig.hpp"

/**
 * @brief Formats a string using fmt::vformat with the to-be substituted values being stored in an std::vector <std::string>.
 * @param base The base formatted string.
 * @param values The std::vector <std::string> with the substituted values.
 * @return A formated std::string.
 */
std::string format_if_filled(std::string_view base, const std::vector <std::string>& values);

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
	void read_config();

	/**
	 * @brief Creates the needed directories and opens the log files.
	 */
	void init_logs();

	/**
	 * @brief Writes down the database values into a map.
	 * When this function is executed, we pray that we have received all the cache.
	 */
	void pray();

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
