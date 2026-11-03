#include "guiding_light/internal/launch_options.hpp"

#include "color.hpp"

bool exec_subcommand(const std::string_view cmd) {
	if (!command_options_list.contains(std::string(cmd))) {
		std::cout << color::rize(std::format("Unknown command: {}\n", cmd), "Red");
		return false;
	}
	command_options_list.at(std::string(cmd))();
	return true;
}

bool exec_subcommands(const int argc, char** argv) {
	bool success = true;
	for (int i = 1; i < argc; i++) {
		if (!exec_subcommand(argv[i])) {
			success = false;
		}
	}
	if (success) {
#ifdef GL_BOT_ENVIRONMENT
		if (TO_KILLALL) {
			killall();
		}
#endif
	}
	return success;
}

#ifdef GL_BOT_ENVIRONMENT

void killall() {
	system(std::format("echo Killing {0}s...\nkillall {0} -s 15 -o $(expr $(ps -p $(pidof -s {0}) -o etimes=) + 1)s -v", GL_EXE_NAME).c_str());
}

#endif
