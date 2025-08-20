#ifndef CLI_H
#define CLI_H

#include "guiding_light/commands.hpp"
#include "linenoise.hpp"
#include "color.hpp"

namespace cli {
    inline const auto HISTORY_PATH = "cli/history.txt";

    extern const std::map <std::string, std::function <void(std::vector <std::string>)>> commands;
    extern const std::map <std::string, std::string> manual;
    extern std::map <std::string, std::vector <std::string>> completions;

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