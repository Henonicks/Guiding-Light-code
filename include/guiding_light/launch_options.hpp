#ifndef OPTIONS_H
#define OPTIONS_H

#include "guiding_light/commands.hpp"

inline const std::set <std::string> command_options_list =
	{"--dev", "--cli", "--killall", "--dump"};

inline bool IS_DEV, IS_CLI, TO_KILLALL, TO_DUMP;

/**
 * @brief Handles a launch option.
 * @param cmd The launch option to be handled.
 * @return True if succeeded, false otherwise.
 */
bool exec_subcommand(std::string_view cmd);

/**
 * @brief Handles the launch options using @ref exec_subcommand(std::string_view).
 * @param argc The amount of launch options.
 * @param argv A C-style string array of the launch options.
 * @return True if succeeded, false otherwise.
 */
bool exec_subcommands(int argc, char** argv);

/**
 * @brief Kills every single guidingLight process besides the current one.
 */
void killall();

#endif
