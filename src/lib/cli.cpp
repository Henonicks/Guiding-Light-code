#include "cli.h"

void read_until_provided(std::string& line) {
	std::vector <std::string> tokens;
	while (tokens.empty()) {
		line = readline(color::rize("> ", "Magenta").c_str()); // Purple is my favourite colour, this is as close as I can get
		tokens = tokenise(line);
	}
	line = tokens[0];
}

bool confirmation(std::string_view line) {
	while (true) {
		std::string input = readline(fmt::format("{} (Y/n) ", line).c_str());
		if (input.size() > 1) {
			std::cout << "Input one letter or simply press enter for Y as the default.\n";
			continue;
		}
		if (input.empty() || tolower(input[0]) == 'y') {
			return true;
		}
		if (tolower(input[0]) == 'n') {
			std::cout << "Cancelled.\n";
			return false;
		}
		std::cout << "Invalid choice.\n";
	}
}

const std::map <std::string, std::function <void(std::vector <std::string>)>> cli_commands = {
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
				return;
			}
			bot = get_bot();
			bot_is_starting = &(!IS_DEV ? bot_release_is_starting : bot_dev_is_starting);
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
		for (const std::string& x : db::table_names) {
			std::ifstream file(fmt::format("../src/{0}/{1}.txt", MODE_NAME, x));
			std::string line;
			while (std::getline(file, line)) {
				uint64_t pos = line.find(' ');
				while (pos != std::string::npos) {
					line.replace(pos, 1, "','");
					pos = (int)line.find(' ');
				}
				line = '\'' + line + '\'';
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
			std::cout << fmt::format("Errors occured. Check logging/cli/{}/sql_logs.log\n", MODE_NAME);
			return;
		}
		try {
			db::sql << "UPDATE jtc_default_values SET vc_limit = REPLACE(vc_limit, 100, 0);";
		}
		catch (const sqlite::sqlite_exception& e) {
			sql_log(e);
			std::cout << fmt::format("Errors occured. Check logging/cli/{}/sql_logs.log\n", MODE_NAME);
			return;
		}
		try {
			db::sql << "UPDATE jtc_default_values SET name = REPLACE(name, '_', ' ');";
		}
		catch (const sqlite::sqlite_exception& e) {
			sql_log(e);
			std::cout << fmt::format("Errors occured. Check logging/cli/{}/sql_logs.log\n", MODE_NAME);
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
		if (db::table_names.contains(table_name)) {
			const int code = system(fmt::format(R"(sqlite '../database/{0}.db' '.mode markdown' ".output ../database/select/{0}/{1}.md" "SELECT * FROM {1}";)", MODE_NAME, table_name).c_str());
			std::cout << (code == 0 ? fmt::format("Generated an .md file in database/select/{0}/{1}.md\n", MODE_NAME, table_name) : "An error occured. Could not generate the .md file.\n");
		}
		else {
			std::cout << fmt::format("The table {} doesn't exist.\n", table_name);
		}
	}},
	{"globalcreate", [](const std::vector <std::string>& cmd) {
		if (!is_running()) {
			std::cout << fmt::format("{} isn't running. Use the launch command to launch it.\n", bot_name());
			return;
		}
		if (cmd.size() == 1) {
			std::cout << "You need to provide at least one slashcommand. See help globalcreate for more info.\n";
			return;
		}
		std::vector <dpp::slashcommand> created;
		std::vector <std::string> not_created;
		for (int i = 1; i < (int)cmd.size(); i++) {
			if (cmd[i] == "...") {
				created.clear();
				not_created.clear();
				break;
			}
			bool exists{};
			for (const auto& x : slashcommands::list_global) {
				if (cmd[i] == x.first) {
					created.push_back(x.second);
					exists = true;
					break;
				}
			}
			if (!exists) {
				not_created.push_back(cmd[i]);
			}
		}
		if (created.empty() && not_created.empty()) {
			bot->global_bulk_command_create(slashcommands::list_global_vector);
		}
		else {
			if (!created.empty()) {
				bot->global_bulk_command_create(created);
			}
			for (const std::string& x : not_created) {
				std::cout << fmt::format("{}: global slashcommand has NOT been defined.\n", x);
			}
		}
		configuration::write_down_slashcommands();
	}},
	{"guildcreate", [](const std::vector <std::string>& cmd) {
		if (!is_running()) {
			std::cout << fmt::format("{} isn't running. Use the launch command to launch it.\n", bot_name());
			return;
		}
		if (cmd.size() == 1) {
			std::cout << "You need to provide at least one slashcommand. See help globalcreate for more info.\n";
			return;
		}
		std::vector <dpp::slashcommand> created;
		std::vector <std::string> not_created;
		for (int i = 1; i < (int)cmd.size(); i++) {
			if (cmd[i] == "...") {
				created.clear();
				not_created.clear();
				break;
			}
			bool exists{};
			for (const auto& x : slashcommands::list_guild) {
				if (cmd[i] == x.first) {
					created.push_back(x.second);
					exists = true;
					break;
				}
			}
			if (!exists) {
				not_created.push_back(cmd[i]);
			}
		}
		if (created.empty() && not_created.empty()) {
			bot->guild_bulk_command_create(slashcommands::list_guild_vector, MY_PRIVATE_GUILD_ID);
		}
		else {
			if (!created.empty()) {
				bot->guild_bulk_command_create(created, MY_PRIVATE_GUILD_ID);
			}
			for (const std::string& x : not_created) {
				std::cout << fmt::format("{}: guild slashcommand has NOT been defined.\n", x);
			}
		}
		configuration::write_down_slashcommands();
	}},
	{"cdelete", [](const std::vector <std::string>& cmd) {
		if (!is_running()) {
			std::cout << fmt::format("{} isn't running. Use the launch command to launch it.\n", bot_name());
			return;
		}
		configuration::write_down_slashcommands();
		if (cmd.size() == 1) {
			bot->global_bulk_command_delete();
			bot->guild_bulk_command_delete(MY_PRIVATE_GUILD_ID);
			slash::global_created.clear();
			slash::guild_created.clear();
		}
		else if (!slash::global_created.contains(cmd[1]) && !slash::guild_created.contains(cmd[1])) {
			std::cout << fmt::format("{}: slashcommand hasn't been created or defined.\n", cmd[1]);
		}
		else {
			if (slash::global_created.contains(cmd[1])) {
				bot->global_command_delete(slash::global_created[cmd[1]].id);
				slash::global_created.erase(cmd[1]);
			}
			else {
				bot->guild_command_delete(slash::guild_created[cmd[1]].id, MY_PRIVATE_GUILD_ID);
				slash::guild_created.erase(cmd[1]);
			}
		}
	}},
	{"launch", [](const std::vector <std::string>&) {
		if (!is_running()) {
			if (*bot_is_starting) {
				std::cout << fmt::format("{} is already being launched.\n", bot_name());
				return;
			}
			const bool to_start = confirmation(fmt::format("This will launch {}. Proceed?", bot_name()));
			if (to_start) {
				*bot_is_starting = true;
				bot->on_ready([](const dpp::ready_t&) {
					configuration::write_down_slashcommands();
				});
				std::thread start([] {
					bot->start();
				});
				start.detach();
			}
		}
		else {
			std::cout << fmt::format("{} is already running.\n", bot_name());
		}
	}},
	{"list", [](const std::vector <std::string>& cmd) {
		std::string type;
		if (cmd.size() == 1) {
			read_until_provided(type);
		}
		else {
			type = cmd[1];
		}
		if (type == "logs") {
			std::cout << fmt::format("Log files are located in logging/<interface>/{}/<file_name>.log\n<interface> is either bot or cli. Here are the possible values for <file_name>:\n", MODE_NAME);
			for (const std::string& x : logs::list) {
				std::cout << x;
				if (x != *logs::list.rbegin()) {
					std::cout << ", ";
				}
			}
			std::cout << '\n';
		}
		else if (type == "slashcommands") {
			std::cout << fmt::format("Slashcommands are defined in src/lib/commands.cpp.\nHere is a list of them:\n");
			std::cout << "Global slashcommands: ";
			for (const auto& x : slashcommands::list_global) {
				std::cout << x.first;
				if (x != *slashcommands::list_global.rbegin()) {
					std::cout << ", ";
				}
			}
			std::cout << '\n';
			std::cout << "Guild slashcommands: ";
			for (const auto& x : slashcommands::list_guild) {
				std::cout << x.first;
				if (x != *slashcommands::list_guild.rbegin()) {
					std::cout << ", ";
				}
			}
			std::cout << '\n';
		}
		else if (type == "tables") {
			std::cout << fmt::format("SQL tables are defined in src/lib/database.cpp.\nHere is a list of them:\n");
			for (const std::string& x : db::table_names) {
				std::cout << x;
				if (x != *db::table_names.rbegin()) {
					std::cout << ", ";
				}
			}
			std::cout << '\n';
		}
		else {
			std::cout << fmt::format("{}: unknown type.\n", type);
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
	if (!token.empty()) {
		tokens.push_back(token);
	}
	return tokens;
}

void exec_cli_command(const std::vector <std::string>& cmd) {
	if (!cli_manual.contains(cmd[0])) {
		std::cout << fmt::format("Unknown command: {}. Try help to see the list of available commands.\n", cmd[0]);
		return;
	}
	cli_commands.at(cmd[0])(cmd);
}

void enter_cli() {
	configuration::read_config();
	slashcommands::init();
	while (true) {
		std::string line = readline(fmt::format("{} ", (!IS_DEV ? color::rize("guidingLight", "Cyan") : color::rize("curiousLight", "Yellow")) + color::rize(">", is_running() ? "Green" : *bot_is_starting ? "Yellow" : "Red")).c_str());
		std::vector <std::string> command = tokenise(line);
		if (!command.empty()) {
			add_history(line.c_str());
			exec_cli_command(command);
		}
	}
}
