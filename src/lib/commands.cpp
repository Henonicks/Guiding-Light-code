#include "commands.h"

std::set <std::string> command_list =
	{"--return", "--dev", "--cli"};
std::set <std::string> slashcommand_list =
	{"--chelp", "--csetup", "--cset", "--cguild", "--cget", "--cvote", "--cblocklist", "--clogs", "--cticket", "--cselect", "--call"};
std::map <std::string, std::string> cli_manual = {
	{"help", "Shows this message.\n"
		"Usage: \"help [cmd1] [cmd2]... [cmdn]\" or \"help ...\""},
	{"quit", "Stop the program. The CLI does not affect the Discord bot while it's running.\n"
		"Usage: quit"},
	{"switch", "Switch between release and dev (guidingLight and curiousLight).\n"
		"Usage: switch <mode>"},
	{"init_db", "Initialise the database for the current mode (release or dev). Needs to be done before the bot is launched.\n"
		"Usage: init_db"},
	{"conv_db", "Convert the database from text file-based to SQL-based. Needs to be done before the bot is launched if you have a text file-based database (was used before SQLite was introduced).\n"
		"Usage: conv_db"},
	{"100_to_0", "Convert the \"infinite users\" limit in the jtc_default_values table from 100 to 0 (100 was used before this was changed). Needs to be done before the bot is launched. If you have a text file-based database, you need to convert it to SQLite-based first.\n"
		"Usage: 100_to_0"},
	{"select", "Make a SELECT query to generate a file in database/select/<mode> where <mode> can either be release or dev."
		"Usage: select <table>"},
};

void read_until_provided(std::string& line) {
	std::vector <std::string> tokens;
	while (tokens.empty()) {
		line = readline(color::rize("> ", "Magenta").c_str());
		tokens = tokenise(line);
	}
	line = tokens[0];
}

std::map <std::string, std::function <void(std::vector <std::string>)>> cli_commands = {
	{"help", [](const std::vector <std::string>& cmd) {
		if (cmd.size() == 1) {
			std::cout << "Usage: <command> [subcommand]\nTry help <command> to see how to use one of the commands or "
			"help <list of commands> to see how to use multiple at once. Example:\nhelp cmd1 cmd2 cmd3.\nhelp ... explains every command.\nCommands: ";
			for (const auto& x : cli_manual) {
				std::cout << x.first;
				if (x.first != cli_manual.rbegin()->first) {
					std::cout << ", ";
				}
			}
			std::cout << '\n';
		}
		else {
			std::set <std::string> requested;
			for (int i = 1; i < (int)cmd.size(); i++) {
				if (cmd[i] == "...") {
					requested.clear();
					break;
				}
				requested.insert(cmd[i]);
			}
			if (requested.empty()) {
				for (const auto& x : cli_manual) {
					std::cout << fmt::format("{0}: {1}\n\n", x.first, x.second);
				}
			}
			else {
				for (const std::string& x : requested) {
					if (!cli_manual.contains(x)) {
						std::cout << fmt::format("{}: doesn't exist.\n", x);
					}
					else {
						std::cout << fmt::format("{0}: {1}\n", x, cli_manual[x]);
					}
				}
			}
		}
	}},
	{"quit", [](const std::vector <std::string>&) {
		std::exit(0);
	}},
	{"switch", [](const std::vector <std::string>& cmd) {
		std::string requested_mode;
		if (cmd.size() == 1) {
			read_until_provided(requested_mode);
		}
		else {
			requested_mode = cmd[1];
		}
		if (requested_mode != "release" && requested_mode != "dev") {
			std::cout << fmt::format("{} is not an acceptable mode.\n", requested_mode);
			return;
		}
		if (requested_mode == MODE_NAME) {
			std::cout << fmt::format("The mode is already {}.\n", requested_mode);
		}
		else {
			IS_DEV = !IS_DEV;
			MODE_NAME = IS_DEV ? "dev" : "release";
			if (!db::connection_successful()) {
				std::cout << fmt::format("ERROR: Failed to connect to the database in the {0} mode. Staying in the {1} mode.\n", requested_mode, MODE_NAME);
				IS_DEV = !IS_DEV;
				MODE_NAME = IS_DEV ? "dev" : "release";
			}
			configuration::init_logs();
		}
	}},
	{"init_db", [](const std::vector <std::string>&) {
		db::sql << "CREATE TABLE jtc_vcs (channel_id BIGINT PRIMARY KEY, guild_id BIGINT);";
		db::sql << "CREATE TABLE temp_vc_notifications (channel_id BIGINT, guild_id BIGINT PRIMARY KEY);";
		db::sql << "CREATE TABLE jtc_default_values (channel_id BIGINT PRIMARY KEY, name VARCHAR(100), vc_limit SMALLINT, bitrate SMALLINT);";
		db::sql << "CREATE TABLE no_temp_ping (user_id BIGINT PRIMARY KEY);";
		db::sql << "CREATE TABLE topgg_guild_choices (user_id BIGINT PRIMARY KEY, guild_id BIGINT);";
		db::sql << "CREATE TABLE topgg_guild_votes_amount (guild_id BIGINT PRIMARY KEY, votes INT);";
		db::sql << "CREATE TABLE no_noguild_reminder (user_id BIGINT PRIMARY KEY);";
		db::sql << "CREATE TABLE topgg_notifications (channel_id BIGINT PRIMARY KEY, guild_id BIGINT);";
		db::sql << "CREATE TABLE tickets (user_id BIGINT PRIMARY KEY, channel_id BIGINT);";
		db::sql << "CREATE TABLE temp_vcs (channel_id BIGINT PRIMARY KEY, guild_id BIGINT, creator_id BIGINT, parent_id BIGINT);";
	}},
	{"conv_db", [](const std::vector <std::string>&) {
		bool is_error{};
		for (const char* x : db::table_names) {
			std::ifstream file(fmt::format("../src/{0}/{1}.txt", MODE_NAME, x));
			std::string line;
			while (std::getline(file, line)) {
				std::cout << line << '\n';
				int64_t pos = line.find(' ');
				while (pos != std::string::npos) {
					line.replace(pos, 1, "','");
					pos = (int)line.find(' ');
				}
				line = '\'' + line + '\'';
				std::cout << line << '\n';
				try {
					db::sql << "INSERT INTO " + (std::string)x + " VALUES (" + line + ");";
				}
				catch (sqlite::sqlite_exception& e) {
					sql_log(e);
					is_error = true;
				}
			}
		}
		if (is_error) {
			std::cout << fmt::format("Errors occured. Check logging/{}/sql_logs.log\n", MODE_NAME);
		}
	}},
	{"100_to_0", [](const std::vector <std::string>&) {
		try {
			db::sql << "UPDATE jtc_default_values SET vc_limit = REPLACE(vc_limit, 100, 0);";
		}
		catch (sqlite::sqlite_exception& e) {
			sql_log(e);
			std::cout << fmt::format("Errors occured. Check logging/{}/sql_logs.log\n", MODE_NAME);
		}
	}},
	{"select", [](const std::vector <std::string>& cmd) {
		std::string table_name;
		if (cmd.size() == 1) {
			read_until_provided(table_name);
		}
		else {
			table_name = cmd[1];
		}
		if (std::find(db::table_names.begin(), db::table_names.end(), table_name) != db::table_names.end()) {
			int code = system(fmt::format("sqlite '../database/{0}.db' '.mode markdown' \".output ../database/select/{0}/{1}.md\" \"SELECT * FROM {1}\";", MODE_NAME, table_name).c_str());
			std::cout << (code == 0 ? fmt::format("Generated an .md file in database/select/{0}/{1}.md\n", MODE_NAME, table_name) : "An error occured. Could not generate the .md file.\n");
		}
		else {
			std::cout << fmt::format("The table {} doesn't exist.\n", table_name);
		}
	}},
};

std::vector <std::string> tokenise(std::string_view line) {
	std::string token;
	std::vector <std::string> tokens;
	for (const char& x : line) {
		if (x == ' ') {
			if (!token.empty()) {
				tokens.push_back(token);
				token = "";
			}
		}
		else {
			token += x;
		}
	}
	if (token != "") {
		tokens.push_back(token);
	}
	return tokens;
}

void exec_cli_command(const std::vector <std::string>& cmd) {
	if (cli_manual[cmd[0]].empty()) {
		std::cout << fmt::format("Unknown command: {}. Try help to see the list of available commands.\n", cmd[0]);
		return;
	}
	cli_commands[cmd[0]](cmd);
}

void enter_cli() {
	configuration::read_config();
	while (true) {
		std::string line = readline(fmt::format("{} ", !IS_DEV ? color::rize("guidingLight", "Cyan") + '>' : color::rize("curiousLight", "Yellow") + '>').c_str());
		std::vector <std::string> command = tokenise(line);
		if (!command.empty()) {
			add_history(line.c_str());
			exec_cli_command(command);
		}
	}
}

void exec_subcommand(std::string_view cmd) {
	if (!command_list.count(std::string(cmd))) {
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
	std::vector <std::string> commands;
	for (int i = 1; i < argc; i++) {
		exec_subcommand(argv[i]);
		commands.emplace_back(argv[i]);
	}
	if (IS_CLI) {
		enter_cli();
	}
}
