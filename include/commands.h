#ifndef COMMANDS_H
#define COMMANDS_H

#include "configuration.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "color.hpp"

extern std::set <std::string> command_list;
extern std::set <std::string> slashcommand_list;

std::vector <std::string> tokenise(std::string_view line);
void exec_cli_command(const std::vector <std::string>& cmd);
void exec_subcommand(std::string_view cmd);
void exec_subcommands(const int& argc, char** argv);

#endif
