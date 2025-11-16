#include "guiding_light/launch_options.hpp"

bool exec_subcommand(const std::string_view cmd) {
	if (!command_options_list.contains(std::string(cmd))) {
		std::cout << "Unknown command: " << cmd << '\n';
		return false;
	}
	if (cmd == "--dev") {
		IS_DEV = true;
	}
	else if (cmd == "--return") {
		BOT_RETURN = dpp::st_return;
	}
	else if (cmd == "--cli") {
		IS_CLI = true;
	}
	else if (cmd == "--killall") {
		TO_KILLALL = true;
	}
	else if (cmd == "--dump") {
		TO_DUMP = true;
	}
	else {
		std::cout << "Command not implemented: " << cmd << '\n';
		return false;
	}
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
		if (TO_KILLALL) {
			killall();
		}
	}
	return success;
}

void killall() {
	system("echo Killing guidingLights...\nkillall guidingLight -s 15 -o $(expr $(ps -p $(pidof -s guidingLight) -o etimes=) + 1)s -v");
}
