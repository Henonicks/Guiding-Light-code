#include "guiding_light/launch_options.hpp"

void exec_subcommand(std::string_view cmd) {
    if (!command_options_list.contains(std::string(cmd))) {
        std::cout << "Unknown command: " << cmd << '\n';
    }
    if (cmd == "--dev") {
        IS_DEV = true;
    }
    if (cmd == "--return") {
        BOT_RETURN = dpp::st_return;
    }
    if (cmd == "--cli") {
        IS_CLI = true;
    }
}

void exec_subcommands(const int& argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        exec_subcommand(argv[i]);
    }
}
