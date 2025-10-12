#ifndef COMMANDS_H
#define COMMANDS_H

#include "guiding_light/guiding_light.hpp"

inline const std::set <std::string> command_options_list =
	{"--return", "--dev", "--cli", "--killall"};

namespace logs {
	inline const std::set <std::string> list =
		{"guild_logs", "my_logs", "other_logs", "sql_logs"};
}

/**
 * @brief Checks if an executable exists and is usable as a command line command.
 * @param command The name of the executable.
 * @return True if the executable exists, false otherwise
 */
bool command_exists(std::string_view command);

namespace slash {
	/**
	 * @brief Gets a mention for a slash command.
	 * @param command The command, including the subcommands.
	 * @return The command mention in the format of </cmd_name subcmds:cmd_id>
	 */
	std::string get_mention(std::string_view command);

	std::vector <std::string> get_mention(const std::vector <std::string>& command);
}

namespace slashcommands {
	inline std::map <std::string, dpp::slashcommand> list_global;
	inline std::map <std::string, dpp::slashcommand> list_guild;

	/**
	 * @brief Builds a slashcommand based on the name of it.
	 * @param name The name of the slashcommand to build.
	 * @return A dpp::slashcommand ready to be created.
	 */
	dpp::slashcommand build_cmd(std::string_view name);

	/**
	 * @brief Initialises the slashcommands list.
	 */
	void init();
}

#endif
