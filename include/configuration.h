#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "notification_channel.h"
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "logging.h"
#include "slash_funcs.h"
#include "topgg.h"
#include "ticket.h"

namespace configuration {
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
