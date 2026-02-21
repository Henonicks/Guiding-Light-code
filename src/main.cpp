#include "guiding_light/guiding_light.hpp"
#include "guiding_light/ticket_handler.hpp"
#include "guiding_light/temp_vc_handler.hpp"
#include "guiding_light/launch_options.hpp"
#include "guiding_light/cli.hpp"
#include "guiding_light/responses.hpp"

int main(const int argc, char** argv) {
	std::cout << "Launching the bot.\n";
	log("Launching the bot.");
	cfg::check_sqlite3();
	// Check if we have sqlite3 installed
	if (!exec_subcommands(argc, argv)) {
		// Execute any subcommands provided with <path>/guidingLight [subcommands].
		std::cerr << "Failed to execute some subcommands. Quitting.\n";
		exit(1);
	}
	cfg::read_config();
	// Write down the values from the config into variables.

	bot_release = new dpp::cluster(BOT_TOKEN,
		IS_CLI ? dpp::i_default_intents :
		               dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content | dpp::i_guild_webhooks | dpp::i_guild_messages);
	bot_dev = new dpp::cluster(BOT_TOKEN_DEV,
		IS_CLI ? dpp::i_default_intents :
		               dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content | dpp::i_guild_webhooks | dpp::i_guild_messages);

	// In the CLI mode we can switch between the release and the dev modes and then launch the bots.
	// To switch between them, we're gonna write their addresses to respective pointers.

	bot = get_bot();
	// Write down the right release/dev address into the bot pointer.

	cfg::init_logs();
	// Open the log files. Create them if needed.

	if (TO_DUMP) {
		std::cout << "Dumping and exiting.\n";
		bot->start(dpp::st_return);
		dump_data(0);
	}

	bot_is_starting = &(!IS_DEV ? bot_release_is_starting : bot_dev_is_starting);
	// also a pointer, it's only used in the CLI mode.

	bot_release->on_log([](const dpp::log_t& log) -> void {
		bot_log(log);
	});

	bot_dev->on_log([](const dpp::log_t& log) -> void {
		bot_log(log);
	});

	if (IS_CLI) {
		cli::enter();
	}

	bot->on_button_click([](const dpp::button_click_t& event) -> void {
		if (IS_CLI || BOT_RETURN) {
			return;
		}
		get_lang();
		if (!slash::enabled) {
			event.reply(response_emsg(IM_PREPARING, lang), error_callback);
			return;
		}
		const std::string_view button_id = event.custom_id;
		// We don't want to handle a button press twice, do we?
		if (button_id == "temp_ping_toggle") {
			const dpp::snowflake& user_id = event.command.usr.id;
			if (no_temp_ping[user_id]) {
				db::sql << "DELETE FROM no_temp_ping WHERE user_id=?;" << user_id.str();
			}
			else {
				db::sql << "INSERT INTO no_temp_ping VALUES (?);" << user_id.str();
			}
			const bool new_tp_rule = !no_temp_ping[user_id];
			if (!new_tp_rule) {
				no_temp_ping.erase(user_id);
			}
			else {
				no_temp_ping[user_id] = new_tp_rule;
			}
			event.reply(response_fmtemsg(NEXT_TIME_THE_PING_WILL_BE, lang,
				{new_tp_rule == true ? response(OFF, lang) : response(ON, lang)})
				.set_channel_id(event.command.channel_id), error_callback);
		}
		else if (button_id.starts_with("help")) {
			event.reply(dpp::ir_update_message, cfg::help_message(lang, button_id[4] - '0'), error_callback);
		}
		else {
			event.reply(response_emsg(UNDEFINED_COMMAND, lang), error_callback);
		}
	});

	bot->on_message_create([](const dpp::message_create_t& event) -> void {
		if (IS_CLI || BOT_RETURN) {
			return;
		}
		if (IS_DEV && event.msg.content == "!dumpq") {
			std::queue <temp_vc_query> copy = temp_vcs_queue;
			std::cout << fmt::format("dumping the {} queries\n", copy.size());
			while (!copy.empty()) {
				const temp_vc_query& q = copy.front();
				std::cout << "channel ID: " << q.channel_id << '\n'
				          << "guild ID: " << q.guild_id << '\n'
				          << "user ID: " << q.usr->id << '\n';
				copy.pop();
			}
		}
		const dpp::snowflake& user_id = event.msg.author.id;
		// Same as with a button click.
		if (user_id == bot->me.id) {
			return;
		}
		const std::string& msg = event.msg.content;
		const dpp::snowflake& channel_id = event.msg.channel_id;
		// We don't want to reply to any of our own messages.
		if (channel_id == TOPGG_WEBHOOK_CHANNEL_ID) {
			const dpp::snowflake voted_user_id = msg.substr(2, msg.size() - bot->me.id.str().size() - 10);
			const int8_t weight = msg[2 + voted_user_id.str().size() + 2] - '0';
			// The vote messages are formatted like this: <@${user_id}> ${weight}>
			// Where ${user_id} is the ID of the user who voted,
			// ${weight} is the amount of points the vote is worth
			const bool failure = topgg::vote(voted_user_id, weight);
			if (failure && !topgg::no_noguild_reminder[voted_user_id]) {
				// If there was a failure in granting a guild a vote point
				// and the user has not been notified about that before,
				// notify them.
				bot->direct_message_create(voted_user_id, dpp::message("You have just voted and missed out on the chance to vote in favor of a guild! Choosing a guild with `/guild set` and voting for me on top.gg grants it guild points which can then be turned into JTC VCs!"), error_callback);
				topgg::no_noguild_reminder[voted_user_id] = true;
				db::sql << "INSERT INTO no_noguild_reminder VALUES (?);" << voted_user_id.str();
			}
			return;
		}
		const dpp::snowflake& guild_id = event.msg.guild_id;
		if (event.msg.is_dm()) {
			handle_dm_in(event);
		}
		else if (event.msg.content.find(fmt::format("<@{}>", bot->me.id)) != std::string::npos) {
			event.reply(random_response(user_id), true, error_callback);
		}
		if (guild_id == TICKETS_GUILD_ID) {
			handle_dm_out(event);
		}
	});

	bot->on_channel_update([](const dpp::channel_update_t& event) -> void {
		if (IS_CLI || BOT_RETURN) {
			return;
		}
		if (!temp_vcs[event.updated.id].channel_id.empty()) {
			bool bans{}, mutes{};
			if (blocklist_updated(event.updated)) {
				bans = true;
			}
			if (mutelist_updated(event.updated)) {
				mutes = true;
			}
			std::string content;
			if (bans && mutes) {
				content = "The blocklist and mutelist have been updated.";
			}
			else if (bans) {
				content += "The blocklist has been updated.";
			}
			else if (mutes) {
				content += "The mutelist has been updated.";
			}
			if (!content.empty()) {
				bot->message_create(dpp::message(event.updated.id, content), error_callback);
			}
		}
	});

	bot->on_channel_delete([](const dpp::channel_delete_t& event) -> void {
		if (IS_CLI || BOT_RETURN) {
			return;
		}
		const dpp::channel_type type = event.deleted.get_type();
		const dpp::snowflake& channel_id = event.deleted.id;
		const dpp::snowflake& guild_id = event.deleted.guild_id;
		if (type == dpp::channel_type::CHANNEL_VOICE) {
			if (!jtc_vcs[channel_id].empty()) {
				jtc_default_values.erase(channel_id);
				jtc_vcs.erase(channel_id);
				if (jtc_vc_amount[guild_id] == 1) {
					jtc_vc_amount.erase(guild_id);
				}
				else {
					--jtc_vc_amount[guild_id];
				}
				db::sql << "DELETE FROM jtc_vcs WHERE channel_id=?;" << channel_id.str();
				db::sql << "DELETE FROM jtc_default_values WHERE channel_id=?;" << channel_id.str();
			}
			if (!temp_vcs[channel_id].channel_id.empty()) {
				banned.erase(channel_id);
				if (temp_vc_amount[guild_id] == 1) {
					temp_vc_amount.erase(guild_id);
				}
				else {
					--temp_vc_amount[guild_id];
				}
				temp_vcs.erase(channel_id);
				db::sql << "DELETE FROM temp_vcs WHERE channel_id=?;" << channel_id.str();
			}
		}
		else {
			if (!temp_vc_notifications[guild_id].empty()) {
				db::sql << "DELETE FROM temp_vc_notifications WHERE guild_id=?;" << guild_id.str();
				temp_vc_notifications.erase(event.deleted.guild_id);
			}
			if (!topgg_notifications[guild_id].empty()) {
				db::sql << "DELETE FROM topgg_notifications WHERE guild_id=?;" << guild_id.str();
				topgg_notifications.erase(event.deleted.guild_id);
			}
			if (!ck_tickets[channel_id].empty()) {
				db::sql << "DELETE FROM tickets WHERE user_id=?;" << ck_tickets[channel_id].str();
				tickets.erase(ck_tickets[channel_id]);
				ck_tickets.erase(channel_id);
			}
		}
	});

	bot->on_guild_create([](const dpp::guild_create_t& event) -> void {
		if (IS_CLI || BOT_RETURN) {
			return;
		}
		guild_log(fmt::format("I have joined a guild. These are its stats:\n"
			"Name: `{0}`\nID: `{1}`\nMembers count: `{2}`"
			, event.created.name, event.created.id, event.created.member_count
		));
	});
	bot->on_guild_delete([](const dpp::guild_delete_t& event) {
		if (IS_CLI || BOT_RETURN) {
			return;
		}
		guild_log(fmt::format("I have left a guild. These are its stats:\n"
			"Name: `{0}`\nID: `{1}`\nMembers count: `{2}`"
			, event.deleted.name, event.deleted.id, event.deleted.member_count
		));
	});

	bot->on_voice_state_update([](const dpp::voice_state_update_t& event) {
		if (IS_CLI || BOT_RETURN) {
			return;
		}
		dpp::snowflake channel_id = event.state.channel_id;
		const dpp::snowflake& user_id = event.state.user_id;
		dpp::user* user = dpp::find_user(user_id);
		const dpp::snowflake& guild_id = event.state.guild_id;
		if (!channel_id.empty()) {
			if (!jtc_vcs[channel_id].empty()) {
				const temp_vc_query q = {user, channel_id, guild_id};
				temp_vcs_queue.push(q);
				temp_vc_create(q);
			}
			else {
				jtc_vcs.erase(channel_id);
			}
		}
		channel_id = vc_statuses[user_id];
		if (!temp_vcs[channel_id].channel_id.empty()) {
			if (dpp::find_channel(channel_id)->get_voice_members().empty()) {
				const dpp::channel* channel = dpp::find_channel(channel_id);
				temp_vc_delete_msg(*user, channel);
			}
		}
		else {
			temp_vcs.erase(channel_id);
		}
		if (!event.state.channel_id.empty()) {
			vc_statuses[user_id] = event.state.channel_id;
		}
		else {
			vc_statuses.erase(user_id);
		}
	});

	bot->on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task <> {
		if (IS_CLI || BOT_RETURN) {
			co_return;
		}
		get_lang();
		if (!slash::enabled) {
			event.reply(response_emsg(IM_PREPARING, lang), error_callback);
			co_return;
		}
		const dpp::snowflake& guild_id = event.command.guild_id;
		const dpp::snowflake& user_id = event.command.usr.id;
		const std::string cmd_name = event.command.get_command_name();
		const dpp::command_interaction cmd = event.command.get_command_interaction();
		if (cmd_name == "help") {
			log(fmt::format("Provide the user {} with some help, NOW!", user_id));
			event.reply(cfg::help_message(lang), error_callback);
		}
		else if (cmd_name == "logs") {
			if (user_id != MY_ID) {
				error_log(fmt::format("User {} is checking the logs! Check your perms!", user_id));
			}
			std::string_view file_name = cmd.options[0].name == "dpp" ? "other_logs.log" : cmd.options[0].name == "mine" ? "my_logs.log" : cmd.options[0].name == "guild" ? "guild_logs.log" : "sql_logs.log";
			const dpp::message message = dpp::message().add_file(file_name, dpp::utility::read_file(fmt::format("{0}/{1}/{2}", logs_directory, MODE_NAME, file_name))).set_flags(dpp::m_ephemeral);
			event.reply(message, error_callback);
		}
		else if (cmd_name == "select") {
			if (user_id != MY_ID) {
				error_log(fmt::format("User {} is selecting! Check your perms!", user_id));
			}
			std::string table_name = cmd.options[0].name;
			for (char& x : table_name) {
				if (x == '-') {
					x = '_';
				}
			}
			const std::filesystem::path select_path(fmt::format("{0}/{1}", db::SELECT_LOCATION, MODE_NAME));
			if (!std::filesystem::exists(select_path)) {
				std::filesystem::create_directories(select_path);
			}
			system(fmt::format(R"(sqlite3 ../database/{0}.db '.mode markdown' ".output ../database/select/{0}/{1}.md" "SELECT * FROM {1}";)", MODE_NAME, table_name).c_str());
			const dpp::message message = dpp::message().add_file("db.md", dpp::utility::read_file(fmt::format("../database/select/{0}/{1}.md", MODE_NAME, table_name))).set_flags(dpp::m_ephemeral);
			event.reply(message, error_callback);
		}
		else if (cmd_name == "vote") {
			log(fmt::format("User {} wants to vote, it seems!", user_id));
			event.reply(response_fmtemsg(VOTE_HERE, lang, {bot->me.id.str(), slash::get_mention("help")}), error_callback);
			co_return;
		}
		else if (cmd_name == "guild") {
			if (cmd.options[0].name == "get") {
				slash::topgg::guild_get(event);
			}
			else if (cmd.options[0].name == "set") {
				slash::topgg::guild_set(event);
			}
		}
		else if (cmd_name == "get") {
			slash::topgg::get_progress(event);
		}
		else if (cmd_name == "set") {
			if (cmd.options[0].name == "default") {
				co_await slash::set::default_values(event);
			}
			else {
				co_await slash::set::current(event);
			}
		}
		else if (cmd_name == "setup") {
			bool& creation_status = slash::in_progress[cmd_name][guild_id];
			if (creation_status) {
				event.reply(response_emsg(A_CHANNEL_IS_ALREADY_BEING_SET_UP, lang), error_callback);
				co_return;
			}
			creation_status = true;
			co_await slash::setup(event);
			slash::in_progress[cmd_name].erase(guild_id);
		}
		else if (cmd_name == "blocklist" || cmd_name == "mutelist") {
			restrictions_types rest_type;
			if (cmd_name == "blocklist") {
				rest_type = RRT_BLOCKLIST;
			}
			else {
				rest_type = RRT_MUTELIST;
			}
			if (cmd.options[0].name == "add") {
				co_await slash::list::add(event, rest_type);
			}
			if (cmd.options[0].name == "remove") {
				co_await slash::list::remove(event, rest_type);
			}
			if (cmd.options[0].name == "status") {
				slash::list::status(event, rest_type);
			}
		}
		else if (cmd_name == "ticket") {
			bool& creation_status = slash::in_progress[cmd_name][user_id];
			if (creation_status) {
				event.reply(response_emsg(A_TICKET_IS_ALREADY_BEING_SET_UP, lang), error_callback);
				co_return;
			}
			if (cmd.options[0].name == "create") {
				creation_status = true;
				co_await slash::ticket::create(event);
				creation_status = false;
			}
			if (cmd.options[0].name == "close") {
				slash::ticket::close(event);
			}
		}
		else if (cmd_name == "reload") {
			log("Started reloading...");
			cfg::read_config();
			cfg::pray();
			if (!db::connection_successful()) {
				event.reply(dpp::message("COULDN'T CONNECT TO THE DATABASE! THIS IS A DISASTER! RUN WHILE YOU CAN!").set_flags(dpp::m_ephemeral), error_callback);
				log("Reload: COULDN'T CONNECT TO THE DATABASE! THIS IS A DISASTER! RUN WHILE YOU CAN!");
				co_return;
			}
			event.reply(dpp::message("Reloaded").set_flags(dpp::m_ephemeral), error_callback);
			log("Finished reloading.");
		}
		else {
			event.reply(response_emsg(UNDEFINED_COMMAND, lang), error_callback);
		}
	});

	std::signal(SIGINT, [](const int code) -> void {
		if (!IS_CLI) {
			log("Ну, все, я пішов спати, бувай, добраніч.");
			std::cout << "Ну, все, я пішов спати, бувай, добраніч.\n";
			// Ukrainian for "Well, that's it, I'm going to bed, bye, goodnight."
			dump_data(code);
		}
	});

	std::signal(SIGTERM, [](const int code) -> void {
		if (!IS_CLI) {
			log("I have been told to kill myself, doing it now.");
			std::cout << "I have been told to kill myself, doing it now.\n";
			dump_data(code);
		}
	});

	std::signal(SIGSEGV, [](const int code) -> void {
		if (!IS_CLI) {
			log("I am not entitled to this memory.");
			std::cout << "I am not entitled to this memory.\n";
			dump_data(code);
		}
	});

	if (!TO_DUMP) {
		bot->start();
	}
	else {
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		std::cout << "It's been a second and there is still no dump. Exiting now.\n";
	}
	return 0;
}
