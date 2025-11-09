#include <unistd.h>

#include "guiding_light/launch_options.hpp"

void exec_subcommand(const std::string_view cmd) {
	if (!command_options_list.contains(std::string(cmd))) {
		std::cout << "Unknown command: " << cmd << '\n';
	}
	else if (cmd == "--dev") {
		IS_DEV = true;
	}
	else if (cmd == "--return") {
		BOT_RETURN = dpp::st_return;
	}
	else if (cmd == "--cli") {
		IS_CLI = true;
	}
	else if (cmd == "--killall") {
		killall();
	}
	else {
		std::cout << "Command not implemented: " << cmd << '\n';
	}
}

void exec_subcommands(const int& argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		exec_subcommand(argv[i]);
	}
}

void killall() {
	system("echo Killing guidingLights...\nkillall guidingLight -s 15 -o $(expr $(ps -p $(pidof -s guidingLight) -o etimes=) + 1)s -v");
}
