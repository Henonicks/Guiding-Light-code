#ifndef COMMANDS_H
#define COMMANDS_H

#include "guidingLight/guiding_light.h"
#include "configuration.h"
#include <readline/readline.h>
#include <readline/history.h>
#include "color.hpp"

namespace logs {
    extern std::set <std::string> list;
}

namespace slashcommands {
    extern std::map <std::string, dpp::slashcommand> list_global;
    extern std::map <std::string, dpp::slashcommand> list_guild;
    extern std::vector <dpp::slashcommand> list_global_vector;
    extern std::vector <dpp::slashcommand> list_guild_vector;

    void init();
}

void read_until_provided(std::string& line);
bool confirmation(std::string_view line);
std::vector <std::string> tokenise(std::string_view line);
void exec_cli_command(const std::vector <std::string>& cmd);
void enter_cli();
void exec_subcommand(std::string_view cmd);
void exec_subcommands(const int& argc, char** argv);


#endif
