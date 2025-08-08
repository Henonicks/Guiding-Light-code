#include "commands.h"

std::set <std::string> command_list =
	{"--return", "--dev", "--cli"};
std::map <std::string, std::string> cli_manual = {
	{"help", "Shows this message.\n"
		"Usage: \"help [cmd1] [cmd2]... [cmdN]\" or \"help ...\""},
	{"quit", "Quit the CLI. If the Discord bot is running, it won't stop simply because of this command.\n"
		"Usage: quit"},
	{"switch", "Switch between release and dev (guidingLight and curiousLight).\n"
		"Usage: switch <mode>"},
	{"init_db", "Initialise the database for the current mode (release or dev). Needs to be done before the bot is launched.\n"
		"Usage: init_db"},
	{"conv_db", "Converts the database from text file-based to SQL-based. Needs to be done before the bot is launched if you have a text file-based database (was used before SQLite was introduced). Also changes the values to match the current standard.\n"
		"Usage: conv_db"},
	{"select", "Make a SELECT query to generate a file in database/select/<mode> where <mode> can either be release or dev.\n"
		"Usage: select <table>"},
	{"globalcreate", "Create a global slashcommand. Multiple arguments can be provided for multiple slashcommands to be created. Commands are created in bulk, so only the commands provided will exist. The rest, if they exist, will be deleted.\n"
		"Usage: \"ccreate <slashcommand> [slashcommand2]... [slashcommandN]\" or \"cdelete ...\""},
	{"guildcreate", "Create a guild slashcommand. Multiple arguments can be provided for multiple slashcommands to be created. Commands are created in bulk, so only the commands provided will exist. The rest, if they exist, will be deleted from the guild.\n"
		"Usage: \"ccreate <slashcommand> [slashcommand2]... [slashcommandN]\" or \"cdelete ...\""},
	{"cdelete", "Delete a slashcommand. Leave no parameters for every slashcommand to be deleted.\n"
		"Usage: cdelete [slashcommand]"},
	{"launch", "Launch the bot with the same mode as the one being used. Stays launched until the CLI has been exited. Switching the mode will NOT shut down the bot.\n"
		"Usage: launch"},
	{"list", "Lists what you ask it to - the log files, database tables or slashcommands that can be created. Acceptable values: logs, slashcommands and tables.\n"
		"Usage: list <type>"},
};

std::set <std::string> logs::list = {"guild_logs", "my_logs", "other_logs", "sql_logs"};

std::map <std::string, dpp::slashcommand> slashcommands::list_global;
std::map <std::string, dpp::slashcommand> slashcommands::list_guild;
std::vector <dpp::slashcommand> slashcommands::list_global_vector;
std::vector <dpp::slashcommand> slashcommands::list_guild_vector;

void slashcommands::init() {
	dpp::slashcommand help("help", "See what I can do!", bot->me.id);
	dpp::slashcommand setup("setup", "Set up a part of JTC feature.", bot->me.id);
	dpp::slashcommand set("set", "Edit an attribute of the temp VC you are in (or of a JTC).", bot->me.id);
	dpp::slashcommand guild("guild", "Get/set the guild you're going to vote in favor of.", bot->me.id);
	dpp::slashcommand get("get", "Get the voting progress of a guild.", bot->me.id);
	dpp::slashcommand vote("vote", "Show the top.gg vote link.", bot->me.id);
	dpp::slashcommand logs("logs", "Drop the logs of choice.", bot->me.id);
	dpp::slashcommand blocklist("blocklist", "Add/Remove a user from your channel's blocklist", bot->me.id);
	dpp::slashcommand ticket("ticket", "Create/Delete a ticket.", bot->me.id);
	dpp::slashcommand select("select", "SELECT everything from one of the tables in the database.", bot->me.id);
	
    set.add_option(
        dpp::command_option(dpp::co_sub_command, "name", "Change the VC name.").
            add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VC to have.", true).set_max_length(100))
    );
    set.add_option(
        dpp::command_option(dpp::co_sub_command, "limit", "Change the member count limit.").
            add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VC to have.", true).set_min_value(0).set_max_value(99))
    );
    set.add_option(
        dpp::command_option(dpp::co_sub_command, "bitrate", "Change the bitrate of the VC.").
            add_option(dpp::command_option(dpp::co_integer, "bitrate", "The bitrate you want the VC to have.", true).set_max_value(384))
    );

    //---------------------------------------------------
    dpp::command_option sub_cmd_group_default = dpp::command_option(dpp::co_sub_command_group, "default", "Change a default attribute of temp VCs.");

    dpp::command_option name_sub_cmd = dpp::command_option(dpp::co_sub_command, "name", "Change default name of temp VCs.");
    name_sub_cmd.add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VCs to have.", true).set_max_length(100));
    name_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
    sub_cmd_group_default.add_option(name_sub_cmd);

    dpp::command_option limit_sub_cmd = dpp::command_option(dpp::co_sub_command, "limit", "Change default limit of temp VCs.");
    limit_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VCs to have.", true).set_min_value(0).set_max_value(99));
    limit_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
    sub_cmd_group_default.add_option(limit_sub_cmd);
    
    dpp::command_option bitrate_sub_cmd = dpp::command_option(dpp::co_sub_command, "bitrate", "Change default name of temp VCs.");
    bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "bitrate","The bitrate you want the VCs to have.", true).set_max_value(384));
    bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
    sub_cmd_group_default.add_option(bitrate_sub_cmd);
    //---------------------------------------------------

    set.add_option(sub_cmd_group_default);

    setup.add_option(
        dpp::command_option(dpp::co_sub_command, "jtc", "Setup a JTC voice channel.").
            add_option(dpp::command_option(dpp::co_integer, "maxmembers", "The max number of members in temporary VCs created from this one.", true).set_min_value(0).set_max_value(99))
    );
    
    dpp::command_option notifications_subcommands = dpp::command_option(dpp::co_sub_command_group, "notifications", "Setup a notification channel.");
    notifications_subcommands.add_option(dpp::command_option(dpp::co_sub_command, "jtc", "Setup a notification channel for JTCs."));
    notifications_subcommands.add_option(dpp::command_option(dpp::co_sub_command, "topgg", "Setup a notification channel for top.gg votes."));
    
    setup.add_option(notifications_subcommands);

    setup.set_default_permissions(dpp::permissions::p_manage_channels);

    guild.add_option(dpp::command_option(dpp::co_sub_command, "get", "Get the guild you're going to vote in favor of. Votes go nowhere by default."));
    guild.add_option(dpp::command_option(dpp::co_sub_command, "set", "Set the guild you're going to vote in favor of."));

    get.add_option(dpp::command_option(dpp::co_sub_command, "progress", "Get the voting progress of a guild."));

    blocklist.add_option(
        dpp::command_option(dpp::co_sub_command, "add", "Add a user to the blocklist.").
            add_option(dpp::command_option(dpp::co_user, "user", "The user to be added to the blocklist."))
    );

    blocklist.add_option(
        dpp::command_option(dpp::co_sub_command, "remove", "Remove a user from the blocklist.").
            add_option(dpp::command_option(dpp::co_user, "user", "The user to be removed from the blocklist."))
    );

    blocklist.add_option(
        dpp::command_option(dpp::co_sub_command, "status", "Remove a user from the blocklist.").
            add_option(dpp::command_option(dpp::co_user, "user", "The user to be removed from the blocklist."))
    );

    logs.add_option(dpp::command_option(dpp::co_sub_command, "dpp", "D++ logs, sent by bot->on_log()."));
    logs.add_option(dpp::command_option(dpp::co_sub_command, "mine", "Logs, written by me."));
    logs.add_option(dpp::command_option(dpp::co_sub_command, "guild", "Guild logs."));
    logs.set_default_permissions(dpp::permissions::p_administrator);

    select.add_option(dpp::command_option(dpp::co_sub_command, "jtc-vcs", "The table containing JTC VCs."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "temp_vc_notifications", "The table containing temporary VC notification channels."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "jtc-default-values", "The table containing the default values of the JTCs."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "no-temp-ping", "The table containing the IDs of the users who don't want to be auto-pinged in temporary VCs."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "topgg-guild-choices", "The table containing the IDs of the guilds that the users have chosen for topgg."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "topgg-guild-votes-amount", "The table containing the amount of votes in favour of the guilds."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "no-noguild-reminder", "The table containing the user IDs who have been notified that they're not voting for any guilds."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "topgg-notifications", "The table containing the channels IDs of where it's notified when a user votes for that guild."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "tickets", "The table containing the IDs of tickets."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "temp-vcs", "The table containing the IDs of temporary VCs."));
    logs.set_default_permissions(dpp::permissions::p_administrator);

    ticket.add_option(dpp::command_option(dpp::co_sub_command, "create", "Create a support ticket."));
    ticket.add_option(dpp::command_option(dpp::co_sub_command, "close", "Delete a support ticket."));
    ticket.set_dm_permission(true);

	slashcommands::list_global = { {"help", help}, {"setup", setup}, {"set", set}, {"guild", guild}, {"get", get}, {"vote", vote}, {"blocklist", blocklist}, {"ticket", ticket} };
	slashcommands::list_guild = { {"logs", logs}, {"select", select} };
	slashcommands::list_global_vector = { help, setup, set, guild, get, vote, blocklist, ticket };
	slashcommands::list_guild_vector = { logs, select };
}

void read_until_provided(std::string& line) {
	std::vector <std::string> tokens;
	while (tokens.empty()) {
		line = readline(color::rize("> ", "Magenta").c_str()); // Purple is my favourite colour, this is as close as I can get
		tokens = tokenise(line);
	}
	line = tokens[0];
}

bool confirmation(std::string_view line) {
	std::string input;
	while (true) {
		input = readline(fmt::format("{} (Y/n) ", line).c_str());
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
				return;
			}
			get_bot(bot);
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
		catch (sqlite::sqlite_exception& e) {
			sql_log(e);
			std::cout << fmt::format("Errors occured. Check logging/cli/{}/sql_logs.log\n", MODE_NAME);
			return;
		}
		try {
			db::sql << "UPDATE jtc_default_values SET name = REPLACE(name, '_', ' ');";
		}
		catch (sqlite::sqlite_exception& e) {
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
		if (std::find(db::table_names.begin(), db::table_names.end(), table_name) != db::table_names.end()) {
			int code = system(fmt::format("sqlite '../database/{0}.db' '.mode markdown' \".output ../database/select/{0}/{1}.md\" \"SELECT * FROM {1}\";", MODE_NAME, table_name).c_str());
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
			const bool is_global{slash::global_created.contains(cmd[1])};
			if (is_global) {
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
				std::thread start([]() {
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
	if (token != "") {
		tokens.push_back(token);
	}
	return tokens;
}

void exec_cli_command(const std::vector <std::string>& cmd) {
	if (!cli_manual.contains(cmd[0])) {
		std::cout << fmt::format("Unknown command: {}. Try help to see the list of available commands.\n", cmd[0]);
		return;
	}
	cli_commands[cmd[0]](cmd);
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
	for (int i = 1; i < argc; i++) {
		exec_subcommand(argv[i]);
	}
}
