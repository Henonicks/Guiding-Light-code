#ifndef CLI_H
#define CLI_H

#include "guiding_light/cfg.hpp"
#include "linenoise.hpp"
#include "color.hpp"

namespace cli {
	inline const auto HISTORY_PATH = "cli/history.txt";

	extern const std::map <std::string, std::function <void(std::vector <std::string>)>> commands;
	inline const std::map <std::string, std::string> manual = {
		{"help", "Shows this message.\n"
			"Usage: \"help [cmd1] [cmd2]... [cmdN]\" or \"help ...\""},
		{"quit", "Quit the CLI. If the Discord bot is running, it won't stop simply because of this command.\n"
			"Usage: quit"},
		{"switch", "Switch between release and dev (guidingLight and curiousLight).\n"
			"Usage: switch <mode>"},
		{"init_db", "Initialise the database for the current mode (release or dev). Needs to be done before the bot is launched.\n"
			"Usage: init_db"},
		{"conv_db", "Converts the database from text file-based to SQL-based. Needs to be done before the bot is launched if you have a text file-based database (was used before SQLite was introduced). Also changes the values to match the current standard.\n"
			"Usage: conv_db"},
		{"select", "Make a SELECT query to generate a file in database/select/<mode> where <mode> can either be release or dev.\n"
			"Usage: select <table>"},
		{"globalcreate", "Create a global slashcommand. Multiple arguments can be provided for multiple slashcommands to be created. Commands are created in bulk, so only the commands provided will exist. The rest, if they exist, will be deleted.\n"
			"Usage: \"ccreate <slashcommand> [slashcommand2]... [slashcommandN]\" or \"cdelete ...\""},
		{"guildcreate", "Create a guild slashcommand. Multiple arguments can be provided for multiple slashcommands to be created. Commands are created in bulk, so only the commands provided will exist. The rest, if they exist, will be deleted from the guild.\n"
			"Usage: \"ccreate <slashcommand> [slashcommand2]... [slashcommandN]\" or \"cdelete ...\""},
		{"cdelete", "Delete a slashcommand. Leave no parameters for every slashcommand to be deleted.\n"
			"Usage: cdelete [slashcommand]"},
		{"launch", "Launch the bot with the same mode as the one being used. Stays launched until the CLI has been exited. Switching the mode will NOT shut down the bot.\n"
			"Usage: launch"},
		{"list", "Lists what you ask it to - the log files, database tables or slashcommands that can be created. Acceptable values: logs, slashcommands and tables.\n"
			"Usage: list <type>"},
	};

	inline std::map <std::string, std::vector <std::string>> completions;

	/**
	 * @brief Reads a line until a valid candidate for a command is provided.
	 * @param line the string to read the input into.
	 */
	void read_until_provided(std::string& line);

	/**
	 * @brief Asks the user to confirm the previous action.
	 * @param line The line stating what is being confirmed.
	 * @return True if the user confirms the action.
	 */
	bool confirmation(std::string_view line);

	/**
	 * @brief Converts the command input into a vector.
	 * @param line The command input.
	 * @return A vector with the command and each subcommand.
	 */
	std::vector <std::string> tokenise(std::string_view line);

	/**
	 * @brief Handles a CLI command.
	 * @param cmd The command to be handled.
	 */
	void exec_command(const std::vector <std::string>& cmd);

	/**
	 * @brief Initialises the CLI before it can be entered.
	 */
	void init();

	/**
	 * @brief Start the CLI loop.
	 */
	void enter();
}

#endif