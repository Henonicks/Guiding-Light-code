#include "guiding_light/internal/cli.hpp"

#include "dpptgg/topgg_poker.hpp"

#include "guiding_light/helpers/conversions.tpp"
#include "guiding_light/handlers/slash_funcs.hpp"
#include "guiding_light/internal/cfg.hpp"
#include "guiding_light/internal/config_values.hpp"
#include "guiding_light/database/database.hpp"

#include "guiding_light/resources/jtc_vc.hpp"
#include "guiding_light/database/markdown.hpp"
#include "guiding_light/resources/notification_channel.hpp"
#include "guiding_light/resources/temp_ping_blocker.hpp"
#include "guiding_light/resources/topgg_guild_choice.hpp"
#include "guiding_light/resources/topgg_guild_vote_amount.hpp"
#include "guiding_light/resources/noguild_reminder_blocker.hpp"
#include "guiding_light/generic/table_mapper.hpp"

void cli::read_until_provided(std::string& line) {
	std::vector <std::string> tokens;
	while (tokens.empty()) {
		const bool quit = linenoise::Readline(color::rize("> ", "Magenta").c_str(), line); // Purple is my favourite colour, this is as close as I can get
		if (quit) {
			line = "";
			std::cout << "Cancelled.\n";
			return;
		}
		tokens = tokenise(line);
	}
	line = tokens[0];
}

bool cli::confirmation(std::string_view line) {
	while (true) {
		std::string input;
		const bool quit = linenoise::Readline(std::format("{} (Y/n) ", line).c_str(), input);
		if (quit || (!input.empty() && tolower(input[0]) == 'n')) {
			std::cout << "Cancelled." << '\n';
			return false;
		}
		if (input.size() > 1) {
			std::cout << color::rize("Input one letter or simply press enter for Y as the default.\n", "Blue");
			continue;
		}
		if (input.empty() || tolower(input[0]) == 'y') {
			return true;
		}
		std::cout << color::rize("Invalid choice.\n", "Red");
	}
}

const std::unordered_map <std::string, std::function <void(std::vector <std::string>)>> cli::commands = {
	{"help", [](const std::vector <std::string>& cmd) {
		if (cmd.size() == 1) {
	        std::cout << color::rize("Usage: ", "Blue") + "<command> [subcommand]\n" + color::rize("Try ", "Blue") + "help <command> " +
	            color::rize("to see how to use one of the commands or ", "Blue") +
	            "help <list of commands>" + color::rize(" to see how to use multiple at once. Example:\n", "Blue") +
	            "help cmd1 cmd2 cmd3.\n" "help ... " + color::rize("explains every command.\nCommands: ", "Blue");
			for (const std::string& cmd : manual | std::views::keys) {
				std::cout << cmd;
				if (cmd != manual.rbegin()->first) {
					std::cout << ", ";
				}
			}
			std::cout << '\n';
		}
		else {
			std::set <std::string> requested;
			for (int i = 1; i < cast <int>(cmd.size()); i++) {
				if (cmd[i] == "...") {
					requested.clear();
					break;
				}
				requested.insert(cmd[i]);
			}
			if (requested.empty()) {
				for (const auto& [cmd_name, cmd_manual] : manual) {
					std::cout << std::format("{0}: {1}\n\n", cmd_name, cmd_manual);
				}
			}
			else {
				for (const std::string& x : requested) {
					if (!manual.contains(x)) {
						std::cout << color::rize(std::format("{}: doesn't exist.\n", x), "Red");
					}
					else {
						std::cout << std::format("{0}: {1}\n", x, manual.at(x));
					}
				}
			}
		}
	}},
	{"quit", [](const std::vector <std::string>&) {
		linenoise::SaveHistory(HISTORY_PATH);
		explode(); // this is a point of no return
	}},
	{"switch", [](const std::vector <std::string>& cmd) {
		std::string requested_mode;
		if (cmd.size() == 1) {
			read_until_provided(requested_mode);
			if (requested_mode.empty()) {
				return;
			}
		}
		else {
			requested_mode = cmd[1];
		}

		std::lock_guard L(cfg_values_mutex);
		if (requested_mode != "release" && requested_mode != "dev") {
			std::cout << color::rize(std::format("{} is not an acceptable mode.\n", requested_mode), "Red");
			return;
		}
		if (requested_mode == MODE_NAME) {
			std::cout << color::rize(std::format("The mode is already {}.\n", requested_mode), MODE_NAME == "release" ? "Cyan" : "Light Yellow");
		}
		else {
			IS_DEV = !IS_DEV;
			MODE_NAME = IS_DEV ? "dev" : "release";
#ifdef GL_USE_LOCAL_DB
			if (!db::test_conn(std::format("../database/{}.db", MODE_NAME))) {
				std::cout << std::format("{0} Failed to connect to the database in the {1} mode.\n", color::rize("Warning:", "Yellow"), requested_mode);
			}
#endif
			bot = get_bot();
			bot_is_starting = &(!IS_DEV ? bot_release_is_starting : bot_dev_is_starting);
		}
	}},
	{"init_db", [](const std::vector <std::string>&) {
#ifdef GL_USE_LOCAL_DB
		std::lock_guard L(db::mutex);

		bool jtc_vcs_exists{};
		auto count_binder = db::sql << "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='jtc_vcs'; -- conv_db";
		count_binder >> jtc_vcs_exists;
		if (jtc_vcs_exists) {
			std::cout << color::rize("The database has already been initialised! If you need to upgrade, use conv_db instead!\n", "Light Red");
			return;
		}

		db::errors_pending.insert("init_db");
		db::sql << "CREATE TABLE jtc_vcs (channel_id BIGINT PRIMARY KEY, guild_id BIGINT); -- init_db";
		db::sql << "CREATE TABLE temp_vc_notifications (channel_id BIGINT, guild_id BIGINT PRIMARY KEY); -- init_db";
		db::sql << "CREATE TABLE jtc_default_values (channel_id BIGINT PRIMARY KEY, name VARCHAR(100), vc_limit SMALLINT, bitrate SMALLINT); -- init_db";
		db::sql << "CREATE TABLE no_temp_ping (user_id BIGINT PRIMARY KEY); -- init_db";
		db::sql << "CREATE TABLE topgg_guild_choices (user_id BIGINT PRIMARY KEY, guild_id BIGINT); -- init_db";
		db::sql << "CREATE TABLE topgg_guild_votes_amount (guild_id BIGINT PRIMARY KEY, votes INT); -- init_db";
		db::sql << "CREATE TABLE no_noguild_reminder (user_id BIGINT PRIMARY KEY); -- init_db";
		db::sql << "CREATE TABLE topgg_notifications (channel_id BIGINT PRIMARY KEY, guild_id BIGINT); -- init_db";
		db::sql << "CREATE TABLE tickets (user_id BIGINT PRIMARY KEY, channel_id BIGINT); -- init_db";
		db::sql << "CREATE TABLE temp_vcs (channel_id BIGINT PRIMARY KEY, guild_id BIGINT, creator_id BIGINT, parent_id BIGINT); -- init_db";
		db::sql << "CREATE TABLE channel_name_edit_timers (channel_id BIGINT PRIMARY KEY, timer BIGINT); -- init_db";
		db::errors_pending.erase("init_db");

		std::cout << color::rize("Basic initialision ran. Running conv_db to upgrade.\n", "Blue");

		commands.at("conv_db")({});
#else
		std::cout << color::rize("The bot needs to be compiled with GL_USE_LOCAL_DB. To do so, pass -DUSE_LOCAL_DB=1 to the cmake command.\n", "Red");
#endif
	}},
	{"conv_db", [](const std::vector <std::string>&) {
#ifdef GL_USE_LOCAL_DB
		std::scoped_lock L(cfg_values_mutex, db::mutex);

		db::errors_pending.insert("conv_db");

		for (const std::string_view x : field_lists | std::views::keys) {
			std::ifstream file(std::format("../src/{0}/{1}.txt", MODE_NAME, x));
			std::string line;
			while (std::getline(file, line)) {
				uint64_t pos = line.find(' ');
				while (pos != std::string::npos) {
					line.replace(pos, 1, "','");
					pos = cast <int>(line.find(' '));
				}
				line = '\'' + line + '\'';
				db::sql << std::format("INSERT INTO {0} VALUES ({1}); -- conv_db", x, line);
			}
		}
		db::sql << "UPDATE jtc_default_values SET vc_limit = REPLACE(vc_limit, 100, 0); -- conv_db";
		db::sql << "UPDATE jtc_default_values SET name = REPLACE(name, '_', ' '); -- conv_db";

		{
			bool already_renamed{};
			auto count_binder = db::sql << "SELECT COUNT(*) FROM pragma_table_info('tickets') WHERE name='dm_channel'; -- conv_db";
			count_binder >> already_renamed;
			if (!already_renamed) {
				db::sql << "ALTER TABLE tickets ADD dm_channel_id BIGINT; -- init_db";
			}
		}

		// GoR-ify the database
		[] {
			bool defaults_exists{};
			auto count_binder = db::sql << "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='jtc_default_values'; -- conv_db";
			count_binder >> defaults_exists;
			if (!defaults_exists) {
				return;
			}
			db::sql << "ALTER TABLE jtc_vcs RENAME TO old_jtc_vcs;";
			db::sql << "CREATE TABLE jtc_vcs AS "
				"SELECT "
				"t1.channel_id, "
				"t1.guild_id, "
				"t2.name, "
				"t2.vc_limit, "
				"t2.bitrate "
				"FROM old_jtc_vcs AS t1 "
				"LEFT JOIN jtc_default_values AS t2 "
				"ON t1.channel_id = t2.channel_id; -- conv_db";
			db::sql << "DROP TABLE old_jtc_vcs; -- conv_db";
			db::sql << "DROP TABLE jtc_default_values; -- conv_db";
		}();
		db::sql << "ALTER TABLE temp_vc_notifications ADD source TINYINT; -- conv_db";
		db::sql << "UPDATE temp_vc_notifications SET source = 1; -- conv_db";

		db::sql << "ALTER TABLE topgg_notifications ADD source TINYINT; -- conv_db";
		db::sql << "UPDATE topgg_notifications SET source = 2; -- conv_db";

		db::sql << "CREATE TABLE notification_channels (channel_id BIGINT, guild_id BIGINT, source TINYINT); -- conv_db";
		db::sql << "INSERT INTO notification_channels (channel_id, guild_id, source) SELECT * FROM temp_vc_notifications; -- conv_db";
		db::sql << "INSERT INTO notification_channels (channel_id, guild_id, source) SELECT * FROM topgg_notifications; -- conv_db";

		db::sql << "DROP TABLE temp_vc_notifications; -- conv_db";
		db::sql << "DROP TABLE topgg_notifications; -- conv_db";

		db::sql << "ALTER TABLE no_temp_ping RENAME TO temp_ping_blockers; -- conv_db";
		db::sql << "ALTER TABLE no_noguild_reminder RENAME TO noguild_reminder_blockers; -- conv_db";
		db::sql << "ALTER TABLE topgg_guild_votes_amount RENAME TO topgg_guild_vote_amounts; -- conv_db";

		// Renaming to strip the _id part off the columns

		db::sql << "ALTER TABLE jtc_vcs RENAME COLUMN 'channel_id' TO 'channel'; -- conv_db";
		db::sql << "ALTER TABLE jtc_vcs RENAME COLUMN 'guild_id' TO 'guild'; -- conv_db";

		db::sql << "ALTER TABLE notification_channels RENAME COLUMN 'channel_id' TO 'channel'; -- conv_db";
		db::sql << "ALTER TABLE notification_channels RENAME COLUMN 'guild_id' TO 'guild'; -- conv_db";

		db::sql << "ALTER TABLE temp_ping_blockers RENAME COLUMN 'user_id' TO 'user'; -- conv_db";

		db::sql << "ALTER TABLE topgg_guild_choices RENAME COLUMN 'user_id' TO 'user'; -- conv_db";
		db::sql << "ALTER TABLE topgg_guild_choices RENAME COLUMN 'guild_id' TO 'guild'; -- conv_db";

		db::sql << "ALTER TABLE topgg_guild_vote_amounts RENAME COLUMN 'guild_id' TO 'guild'; -- conv_db";

		db::sql << "ALTER TABLE noguild_reminder_blockers RENAME COLUMN 'user_id' TO 'user'; -- conv_db";

		db::sql << "ALTER TABLE tickets RENAME COLUMN 'user_id' TO 'user'; -- conv_db";
		db::sql << "ALTER TABLE tickets RENAME COLUMN 'channel_id' TO 'channel'; -- conv_db";
		db::sql << "ALTER TABLE tickets RENAME COLUMN 'dm_channel_id' TO 'dm_channel'; -- conv_db";

		db::sql << "ALTER TABLE temp_vcs RENAME COLUMN 'channel_id' TO 'channel'; -- conv_db";
		db::sql << "ALTER TABLE temp_vcs RENAME COLUMN 'guild_id' TO 'guild'; -- conv_db";
		db::sql << "ALTER TABLE temp_vcs RENAME COLUMN 'creator_id' TO 'creator'; -- conv_db";
		db::sql << "ALTER TABLE temp_vcs RENAME COLUMN 'parent_id' TO 'parent'; -- conv_db";
		[] {
			bool alter_failure = db::is_error(db::sql << "ALTER TABLE temp_vcs ADD name_timer BIGINT; -- conv_db");
			if (alter_failure) {
				return;
			}
			alter_failure = db::is_error(db::sql << "ALTER TABLE temp_vcs RENAME TO old_temp_vcs; -- conv_db");
			const bool create_failure = db::is_error(db::sql << "CREATE TABLE temp_vcs AS "
				"SELECT "
				"t1.channel, "
				"t1.guild, "
				"t1.creator, "
				"t1.parent, "
				"IFNULL(t2.timer, 0) AS name_timer "
				"FROM old_temp_vcs AS t1 "
				"LEFT JOIN channel_name_edit_timers AS t2 "
				"ON t1.channel = t2.channel_id; -- conv_db"
			);
			if (create_failure) {
				return;
			}
			db::sql << "DROP TABLE channel_name_edit_timers --conv_db";
			db::sql << "DROP TABLE old_temp_vcs; -- conv_db";
		}();
		db::errors_pending.erase("conv_db");
		std::cout << "Conversion done. Note that some errors aren't critical and can be ignored as they are caused by queries that try to achieve something that's already been done.\n";
#else
		std::cout << color::rize("The bot needs to be compiled with GL_USE_LOCAL_DB. To do so, pass -DUSE_LOCAL_DB=1 to the cmake command.\n", "Red");
#endif
	}},
	{"upload_db", [](const std::vector <std::string>&) {
#ifdef GL_USE_LOCAL_DB
		if (GL_ON_RAILS_TOKEN.empty()) {
			std::cout << "The GoR parameters have not been initialised! Fix the config!\n";
			return;
		}
		std::cout << "Uploading can take a while, please wait.\n";
		{
			std::vector <jtc_vc_t> resources;
			db::sql << "SELECT * FROM jtc_vcs; -- upload_db::jtc_vcs" >> [&resources](const db::BIGINT channel_id, const db::BIGINT guild_id, const std::string& name, const db::TINYINT limit, const db::SMALLINT bitrate) {
				resources.emplace_back(channel_id, guild_id, name, limit, bitrate);
			};
			db::upload <jtc_vc_t>(resources).sync_wait();
		}
		{
			std::vector <notification_channel_t> resources;
			db::sql << "SELECT * FROM notification_channels; -- upload_db::notification_channels" >> [&resources](const db::BIGINT channel_id, const db::BIGINT guild_id, const db::TINYINT source) {
				resources.emplace_back(channel_id, guild_id, cast <notification_source_t>(source));
			};
			db::upload <notification_channel_t>(resources).sync_wait();
		}
		{
			std::vector <temp_ping_blocker_t> resources;
			db::sql << "SELECT * FROM temp_ping_blockers; -- upload_db::temp_ping_blockers" >> [&resources](const db::BIGINT user_id) {
				resources.emplace_back(user_id);
			};
			db::upload <temp_ping_blocker_t>(resources).sync_wait();
		}
		{
			std::vector <topgg_guild_choice_t> resources;
			db::sql << "SELECT * FROM topgg_guild_choices; -- upload_db::topgg_guild_choices" >> [&resources](const db::BIGINT user_id, const db::BIGINT guild_id) {
				resources.emplace_back(user_id, guild_id);
			};
			db::upload <topgg_guild_choice_t>(resources).sync_wait();
		}
		{
			std::vector <topgg_guild_vote_amount_t> resources;
			db::sql << "SELECT * FROM topgg_guild_vote_amounts; -- upload_db::topgg_guild_vote_amounts" >> [&resources](const db::BIGINT guild_id, const int votes) {
				resources.emplace_back(guild_id, votes);
			};
			db::upload <topgg_guild_vote_amount_t>(resources).sync_wait();
		}
		{
			std::vector <noguild_reminder_blocker_t> resources;
			db::sql << "SELECT * FROM noguild_reminder_blockers; -- upload_db::noguild_reminder_blockers" >> [&resources](const db::BIGINT user_id) {
				resources.emplace_back(user_id);
			};
			db::upload <noguild_reminder_blocker_t>(resources).sync_wait();
		}
		{
			std::vector <ticket_t> resources;
			db::sql << "SELECT * FROM tickets; -- upload_db::tickets" >> [&resources](const db::BIGINT user_id, const db::BIGINT channel_id, const db::BIGINT dm_channel_id) {
				resources.emplace_back(user_id, channel_id, dm_channel_id);
			};
			db::upload <ticket_t>(resources).sync_wait();
		}
		{
			std::vector <temp_vc_t> resources;
			db::sql << "SELECT * FROM temp_vcs; -- upload_db::temp_vcs" >> [&resources](const db::BIGINT channel_id, const db::BIGINT guild_id, const db::BIGINT creator_id, const db::BIGINT parent_id, const time_t name_timer) {
				resources.emplace_back(channel_id, guild_id, creator_id, parent_id, name_timer);
			};
			const auto vector = db::upload <temp_vc_t>(resources).sync_wait();
		}
#else
		std::cout << color::rize("The bot needs to be compiled with GL_USE_LOCAL_DB. To do so, pass -DUSE_LOCAL_DB=1 to the cmake command.\n", "Red");
#endif
	}},
	{"select", [](const std::vector <std::string>& cmd) {
		const std::filesystem::path select_path(std::format("{0}/{1}", db::SELECT_LOCATION, MODE_NAME));
		if (!std::filesystem::exists(select_path)) {
			std::filesystem::create_directories(select_path);
		}
		std::string table_name;
		if (cmd.size() == 1) {
			read_until_provided(table_name);
			if (table_name.empty()) {
				return;
			}
		}
		else {
			table_name = cmd[1];
		}

		std::lock_guard L(cfg_values_mutex);

		if (field_lists.contains(table_name)) {
			const auto md = markdown_t::generate_md(table_name).sync_wait();
			if (md.is_valid()) {
				std::ofstream md_file(std::format("{0}/{1}/{2}.md", db::SELECT_LOCATION, MODE_NAME, table_name));
				md_file << md.content << std::endl;
				if (!md_file.bad()) {
					std::cout << color::rize(std::format("Generated a .md file in {0}/{1}/{2}.md\n",
						db::SELECT_LOCATION, MODE_NAME, table_name), "Green");
					return;
				}
			}
			std::cout << color::rize("An error occurred. Could not generate a .md file.\n", "Red");
		}
		std::cout << color::rize(std::format("The table {} doesn't exist.\n", table_name), "Red");
	}},
	{"globalcreate", [](const std::vector <std::string>& cmd) {
		if (!is_running()) {
			std::cout << color::rize(std::format("{} isn't running. Use the launch command to launch it.\n", bot_name()), "Red");
			return;
		}
		if (cmd.size() == 1) {
			std::cout << color::rize("You need to provide at least one slashcommand. See help globalcreate for more info.\n", "Red");
			return;
		}
		std::vector <dpp::slashcommand> created;
		std::vector <std::string> not_created;
		for (int i = 1; i < cast <int>(cmd.size()); i++) {
			if (cmd[i] == "...") {
				created.clear();
				not_created.clear();
				break;
			}
			bool exists{};
			for (const auto& [name, command] : slashcommands::list_global) {
				if (cmd[i] == name) {
					created.push_back(command);
					exists = true;
					break;
				}
			}
			if (!exists) {
				not_created.push_back(cmd[i]);
			}
		}
		if (created.empty() && not_created.empty()) {
			bot->global_bulk_command_create(map_values_to_vector(slashcommands::list_global), [](const dpp::confirmation_callback_t& callback) {
				if (!error_callback(callback)) {
					cfg::write_down_slashcommands();
				}
			});
		}
		else {
			if (!created.empty()) {
				bot->global_bulk_command_create(created, [](const dpp::confirmation_callback_t& callback) {
					if (!error_callback(callback)) {
						cfg::write_down_slashcommands();
					}
				});
			}
			for (const std::string& x : not_created) {
				std::cout << std::format("{}: global slashcommand has NOT been defined.\n", x);
			}
		}
	}},
	{"guildcreate", [](const std::vector <std::string>& cmd) {
		if (!is_running()) {
			std::cout << color::rize(std::format("{} isn't running. Use the launch command to launch it.\n", bot_name()), "Red");
			return;
		}
		if (cmd.size() == 1) {
			std::cout << color::rize("You need to provide at least one slashcommand. See help guildcreate for more info.\n", "Red");
			return;
		}
		std::vector <dpp::slashcommand> created;
		std::vector <std::string> not_created;
		for (int i = 1; i < cast <int>(cmd.size()); i++) {
			if (cmd[i] == "...") {
				created.clear();
				not_created.clear();
				break;
			}
			bool exists{};
			for (const auto& [name, command] : slashcommands::list_guild) {
				if (cmd[i] == name) {
					created.push_back(command);
					exists = true;
					break;
				}
			}
			if (!exists) {
				not_created.push_back(cmd[i]);
			}
		}
		if (created.empty() && not_created.empty()) {
			bot->guild_bulk_command_create(map_values_to_vector(slashcommands::list_guild), MY_PRIVATE_GUILD_ID, error_callback);
		}
		else {
			if (!created.empty()) {
				bot->guild_bulk_command_create(created, MY_PRIVATE_GUILD_ID, error_callback);
			}
			for (const std::string& x : not_created) {
				std::cout << color::rize(std::format("{}: guild slashcommand has NOT been defined.\n", x), "Red");
			}
		}
		cfg::write_down_slashcommands();
	}},
	{"cdelete", [](const std::vector <std::string>& cmd) {
		if (!is_running()) {
			std::cout << color::rize(std::format("{} isn't running. Use the launch command to launch it.\n", bot_name()), "Red");
			return;
		}
		cfg::write_down_slashcommands();
		if (cmd.size() == 1) {
			bot->global_bulk_command_delete(error_callback);
			bot->guild_bulk_command_delete(MY_PRIVATE_GUILD_ID, error_callback);
			slash::global_created.clear();
			slash::guild_created.clear();
		}
		else if (!slash::global_created.contains(cmd[1]) && !slash::guild_created.contains(cmd[1])) {
			std::cout << color::rize(std::format("{}: slashcommand hasn't been created or defined.\n", cmd[1]), "Red");
		}
		else {
			if (slash::global_created.contains(cmd[1])) {
				bot->global_command_delete(slash::global_created[cmd[1]].id, error_callback);
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
				std::cout << color::rize(bot_name(), MODE_NAME == "release" ? "Cyan" : "Light Yellow") + color::rize(" is already being launched.\n", "Yellow");
				return;
			}
			const bool to_start = confirmation(color::rize(std::format("This will launch {}. Proceed?", bot_name()), MODE_NAME == "release" ? "Light Cyan" : "Light Yellow"));
			if (to_start) {
				*bot_is_starting = true;
				bot->on_ready([](const dpp::ready_t&) {
					cfg::write_down_slashcommands();
				});
				std::thread start([] {
					bot->start();
				});
				start.detach();
			}
		}
		else {
			std::cout << color::rize(std::format("{} is already running.\n", bot_name()), "Green");
		}
	}},
	{"topgg_update", [](const std::vector <std::string>& cmd) {
		std::string type;
		if (cmd.size() == 1) {
			read_until_provided(type);
			if (type.empty()) {
				return;
			}
		}
		else {
			type = cmd[1];
		}
		if (type == "slashcommands") {
			std::scoped_lock L(slashcommands::list_mutex, cfg_values_mutex);
			if (TOPGG_BOT_TOKEN.empty()) {
				std::cout << color::rize("No top.gg token was provided in the config!\n", "Light Red");
				return;
			}
			std::cout << color::rize("Updating the slashcommand list on top.gg.\n", "Blue");
			if (slash::global_vector.empty()) {
				std::cout << color::rize("The local slashcommand list is empty. Launch the bot to get a list of those that exist.\n", "Red");
				return;
			}
			dpptgg::poker poker(TOPGG_BOT_TOKEN);
			poker.update_discord_bot_commands(slash::global_vector, [&poker](dpptgg::v1::request_completion_t const& callback) {
				poker.shutdown();
				uint16_t const status = callback.request.status;
				if (callback.is_error()) {
					std::cout << color::rize("Error! Status: ", "Red") << status;
					if (!callback.error.detail.empty()) {
						std::cout << color::rize(", detail: ", "Red") << callback.error.detail;
					}
					std::cout << '\n';
				}
				else {
					std::cout << color::rize("Success!\n", "Green");
				}
			});
			poker.start();
		}
		else {
			std::cout << "Unsupported type. See help topgg_update for more info.\n";
		}
	}},
	{"list", [](const std::vector <std::string>& cmd) {
		std::string type;
		if (cmd.size() == 1) {
			read_until_provided(type);
			if (type.empty()) {
				return;
			}
		}
		else {
			type = cmd[1];
		}
		if (type == "logs") {
			std::lock_guard L(cfg_values_mutex);
			std::cout << color::rize(std::format("Log files are located in logging/<interface>/{}/<file_name>.log\n<interface> is either bot or cli. Here are the possible values for <file_name>:\n", MODE_NAME), "Blue");
			for (const std::string& x : logs::list) {
				std::cout << x;
				if (x != *logs::list.rbegin()) {
					std::cout << ", ";
				}
			}
			std::cout << '\n';
		}
		else if (type == "slashcommands") {
			std::cout << color::rize("Slashcommands are defined in src/lib/commands.cpp.\nHere is a list of them:\n", "Blue");
			std::cout << "Global slashcommands: ";
			std::lock_guard L(slashcommands::list_mutex);
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
			std::cout << color::rize("SQL tables are defined in their respective resource structs; those are defined in their respective header files under include/guiding_light.\nHere is a list of said tables:\n", "Blue");
			int vis{};
			for (const std::string_view x : field_lists | std::views::keys) {
				++vis;
				std::cout << x;
				if (vis != field_lists.size()) {
					std::cout << ", ";
				}
			}
			std::cout << '\n';
		}
		else {
			std::cout << std::format("{}: unknown type.\n", type);
		}
	}},
};

std::vector <std::string> cli::tokenise(const std::string_view line) {
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
			token += cast <char>(tolower(x));
		}
	}
	if (!token.empty()) {
		tokens.push_back(token);
	}
	return tokens;
}

void cli::exec_command(const std::vector <std::string>& cmd) {
	if (!commands.contains(cmd[0])) {
		std::cout << color::rize(std::format("Unknown command: {}. Try help to see the list of available commands.\n", cmd[0]), "Red");
		return;
	}
	commands.at(cmd[0])(cmd);
}

void cli::init() {
	cfg::read_config("../Guiding_Light_Config/config.hfg");
	slashcommands::init();
	const std::filesystem::path history_path(HISTORY_PATH);
	if (!std::filesystem::exists(history_path)) {
		std::filesystem::create_directory("cli");
	}
	completions = {
		{"help", concat_vectors <std::string>( {{"..."}, map_keys_to_vector(manual)} )},
		{"quit", {"smoking"}},
		{"switch", {"release", "dev"}},
		{"init_db", {}},
		{"conv_db", {}},
		{"upload_db", {}},
		{"select", map_keys_to_vector <std::string>(field_lists)},
		{"globalcreate", concat_vectors <std::string>( {{"..."}, map_keys_to_vector(slashcommands::list_global)} )},
		{"guildcreate", concat_vectors <std::string>( {{"..."}, map_keys_to_vector(slashcommands::list_guild)} )},
		{"cdelete", concat_vectors <std::string>( {map_keys_to_vector(slashcommands::list_global), map_keys_to_vector(slashcommands::list_global)} )},
		{"launch", {}},
		{"topgg_update", {"slashcommands"}},
		{"list", {"logs", "slashcommands", "tables"}},
	};
	linenoise::SetCompletionCallback([](const char* edit_buffer, std::vector <std::string>& completion) {
		std::string line = edit_buffer;
		const std::vector <std::string> cmd = tokenise(line);
		if (cmd.empty() || *line.rbegin() == ' ') {
			return;
		}
		std::vector <std::string> valid_completions;
		if (completions.contains(cmd[0])) {
			line = "";
			for (auto it = cmd.begin(); it != cmd.end() - 1; ++it) {
				line += *it;
			}
			for (const std::string& x : completions.at(cmd[0])) {
				if (x.starts_with(*cmd.rbegin())) {
					valid_completions.push_back(x);
				}
			}
			if (!valid_completions.empty()) {
				completion = {line + ' ' + valid_completions[0]};
			}
		}
		else {
			for (const std::string& x : map_keys_to_vector(completions)) {
				if (x.starts_with(cmd[0])) {
					valid_completions.push_back(x);
				}
			}
			if (!valid_completions.empty()) {
				completion = {valid_completions[0] + ' '};
			}
		}
		if (valid_completions.size() > 1) {
			completion = {edit_buffer};
			linenoise::linenoiseWipeLine(); // Wipe the line so that the tips appear without any other text.
			for (const std::string& x : valid_completions) {
				std::cout << x << ' ';
			}
			std::cout << '\n';
		}
	});
	linenoise::LoadHistory(HISTORY_PATH);
}

void cli::enter() {
	init();
	if (!db::test_conn(std::format("../database/{}.db", MODE_NAME))) {
		std::cout << std::format("{0} Failed to connect to the database in the {1} mode.\n", color::rize("Warning:", "Yellow"), MODE_NAME);
	}
	while (!ready_to_explode) {
		std::string line;
		const bool quit = linenoise::Readline(
			std::format("{} ",
				(!IS_DEV ? color::rize("guidingLight", "Cyan") : color::rize("curiousLight", "Light Yellow"))
				+ color::rize(">", is_running() ? "Light Green" : *bot_is_starting ? "Yellow" : "Red")).c_str(),
		line);
		const std::vector <std::string> command = tokenise(line);
		if (!command.empty()) {
			exec_command(command);
		}
		if (quit) {
			break;
		}
		linenoise::AddHistory(line.c_str());
	}
	commands.at("quit")({});
}
