#ifndef COMMANDS_H
#define COMMANDS_H

#include "configuration.h"

extern std::set <std::string> subcommand_list;
extern std::map <std::string, std::string> cli_manual;

namespace logs {
    extern std::set <std::string> list;
}

namespace slashcommands {
    extern std::map <std::string, dpp::slashcommand> list_global;
    extern std::map <std::string, dpp::slashcommand> list_guild;
    extern std::vector <dpp::slashcommand> list_global_vector;
    extern std::vector <dpp::slashcommand> list_guild_vector;

    /**
     * @brief Initialises the slashcommands list.
     */
    void init();
}

#endif
