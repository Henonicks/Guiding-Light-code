#ifndef COMMANDS_H
#define COMMANDS_H

#include "guiding_light/configuration.hpp"

extern const std::set <std::string> subcommand_list;

namespace logs {
    extern std::set <std::string> list;
}

namespace slashcommands {
    extern std::map <std::string, dpp::slashcommand> list_global;
    extern std::map <std::string, dpp::slashcommand> list_guild;

    /**
     * @brief Initialises the slashcommands list.
     */
    void init();
}

#endif
