#ifndef OPTIONS_H
#define OPTIONS_H

#include "guiding_light/commands.hpp"

inline bool TO_DUMP = false;

/**
 * @brief Handles a launch option.
 * @param cmd The launch option to be handled.
 */
void exec_subcommand(std::string_view cmd);

/**
 * @brief Handles the launch options using @ref exec_subcommand(std::string_view).
 * @param argc The amount of launch options.
 * @param argv A C-style string array of the launch options.
 */
void exec_subcommands(const int& argc, char** argv);

/**
 * @brief Kills every single guidingLight process besides the current one.
 */
void killall();

#endif
