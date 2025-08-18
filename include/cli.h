#ifndef CLI_H
#define CLI_H

#include <readline/readline.h>
#include <readline/history.h>
#include "color.hpp"
#include "commands.h"

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
void exec_cli_command(const std::vector <std::string>& cmd);

/**
 * @brief Start the CLI loop.
 */
void enter_cli();

#endif