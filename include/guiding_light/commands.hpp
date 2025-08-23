#ifndef COMMANDS_H
#define COMMANDS_H

#include "guiding_light/guiding_light.hpp"

extern const std::set <std::string> subcommand_list;

namespace logs {
    extern std::set <std::string> list;
}

bool command_exists(std::string_view command);

namespace slash {
    /**
     * @brief Gets a mention for a slash command.
     * @param command The command, including the subcommands.
     * @return The command mention in the format of </cmd_name subcmds:cmd_id>
     */
    std::string get_mention(std::string_view command);
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
