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

#include "color.hpp"

#include "henifig/henifig.hpp"

namespace cfg {
	inline henifig::config_t config, responses;
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
}

#endif
