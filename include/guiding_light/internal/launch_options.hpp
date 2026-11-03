#ifndef OPTIONS_H
#define OPTIONS_H

#include "guiding_light/internal/commands.hpp"

inline std::atomic <bool> IS_DEV, IS_CLI, TO_KILLALL, TO_DUMP, ENABLE_TOPGG;

#ifdef GL_BOT_ENVIRONMENT
inline const std::unordered_map <std::string, std::function <void()>> command_options_list = {
	{"--dev", [] {
		IS_DEV = true;
	}},
	{"--cli", [] {
		IS_CLI = true;
	}},
	{"--killall", [] {
		TO_KILLALL = true;
	}},
	{"--dump", [] {
		TO_DUMP = true;
	}},
	{"--topgg", [] {
		ENABLE_TOPGG = true;
	}},
};
#else
inline const std::unordered_map <std::string, std::function <void()>> command_options_list = {
	{"--dev", [] {
		IS_DEV = true;
	}},
};
#endif

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
 * @brief Kills every single GL_EXE_NAME process that is older than the current one. This includes any cli process!
 */
void killall();

#endif
