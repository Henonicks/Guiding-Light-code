#include "guiding_light/guiding_light.hpp"
#include "guiding_light/ticket_handler.hpp"
#include "guiding_light/temp_vc_handler.hpp"
#include "guiding_light/cli.hpp"
#include "guiding_light/responses.hpp"
#include "guiding_light/slash_funcs.hpp"
#include "guiding_light/signal.hpp"

int main(const int argc, char** argv) {
	cfg::check_sqlite3();
	// Check if we have sqlite3 installed
	if (!exec_subcommands(argc, argv)) {
		// Execute any subcommands provided with <path>/guidingLight [subcommands].
		std::cerr << "Failed to execute some subcommands. Quitting.\n";
		exit(1);
	}
	cfg::read_config();
	// Write down the values from the config into variables.

	const auto bot_intents =
		IS_CLI ? dpp::i_default_intents :
		         dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content |
		         	dpp::i_direct_message_typing | dpp::i_guild_message_typing | dpp::i_guild_webhooks | dpp::i_guild_messages;
	bot_release = new dpp::cluster(BOT_TOKEN, bot_intents);
	bot_dev = new dpp::cluster(BOT_TOKEN_DEV, bot_intents);
	// In the CLI mode we can switch between the release and the dev modes and then launch the bots.
	// To switch between them, we're gonna write their addresses to respective pointers.

	bot = get_bot();
	// Write down the right release/dev address into the bot pointer.

	cfg::init_logs();
	// Open the log files. Create them if needed.

	if (TO_DUMP) {
		std::cout << "Dumping and exiting.\n";
		bot->start(dpp::st_return);
		dump_data(f_success).sync_wait();
	}

	if (IS_CLI) {
		bot_is_starting = &(!IS_DEV ? bot_release_is_starting : bot_dev_is_starting);
		// also a pointer, it's only used in the CLI mode.
	}

	bot_release->on_log([](const dpp::log_t& log) -> void {
		bot_log(log);
	});

	bot_dev->on_log([](const dpp::log_t& log) -> void {
		bot_log(log);
	});

	if (IS_CLI) {
		cli::enter();
		return 0;
	}

	if (!db::connection_successful()) {
		std::cerr << fmt::format("{0} connection to DB failed! imma js crash ts g 💔🥀\n{1} have you imported your database as database/{2}.db or initialised the database with init_db?", color::rize("ERROR:", "Red"), color::rize("HINT:", "Yellow"), MODE_NAME) << std::endl;
		return 0;
	}

	cfg::init_db_data();

	bot->on_ready([](const dpp::ready_t&) -> void {
		if (dpp::run_once <struct initialise_bot>()) {
			cfg::init_bot();
		}
	});

	bot->on_typing_start([](const dpp::typing_start_t& event) {
		const dpp::snowflake typing_channel_id = event.typing_channel.id;
		const dpp::snowflake typing_user_id = event.user_id;
		const dpp::snowflake guild_id = event.typing_guild.id;
		if (guild_id == 0 && tickets.contains(typing_user_id)) {
			bot->channel_typing(tickets[typing_user_id].channel_id, error_callback);
		}
		else if (guild_id == TICKETS_GUILD_ID && ck_tickets.contains(typing_channel_id)) {
			bot->channel_typing(ck_tickets[typing_channel_id].dm_channel_id, error_callback);
		}
	});

	bot->on_button_click([](const dpp::button_click_t& event) {
		get_lang();
		const std::string_view button_id = event.custom_id;
		if (button_id == "temp_ping_toggle") {
			const dpp::snowflake user_id = event.command.usr.id;
			std::lock_guard L(temp_vc_mutex);
			const bool allowing_pings = !no_temp_ping[user_id];
			if (allowing_pings) {
				db::sql << "DELETE FROM no_temp_ping WHERE user_id=?;" << user_id.str();
				no_temp_ping.erase(user_id);
			}
			else {
				db::sql << "INSERT INTO no_temp_ping VALUES (?);" << user_id.str();
				no_temp_ping[user_id] = allowing_pings;
			}
			event.reply(response_fmtemsg(NEXT_TIME_THE_PING_WILL_BE, lang,
				{!allowing_pings ? response(OFF, lang) : response(ON, lang)})
					.set_channel_id(event.command.channel_id), error_callback);
		}
		else if (button_id.starts_with("help")) {
			if (isdigit(button_id[4])) {
				event.reply(dpp::ir_update_message, cfg::help_message(lang, button_id[4] - '0'), error_callback);
			}
			else {
				const dpp::message& msg = event.command.msg;
				const uint8_t curr_page = msg.components[0].components[2].custom_id[4] - '1';
				dpp::interaction_modal_response search_modal("help_search_modal", response(SELECT_THE_HELP_PAGE_YOU_WANT_TO_VISIT, lang));
				const std::vector <std::string> pages = get_help_command_page_names(lang);
				dpp::component search_select_menu = dpp::component()
					.set_label(response(SELECT_A_HELP_PAGE, lang))
					.set_placeholder(response(HELP_PAGE, lang))
					.set_id("help_search_modal_number")
					.set_type(dpp::cot_selectmenu);
				for (uint8_t i = 0; i < pages.size(); i++) {
					if (curr_page != i) {
						search_select_menu.add_select_option(
							dpp::select_option()
								.set_label(fmt::format("{0}. {1}", i + 1, pages[i]))
								.set_value(fmt::format("help{}", i))
						);
					}
				}
				search_modal.add_component(search_select_menu);
				event.dialog(search_modal);
			}
		}
		else {
			event.reply(response_emsg(UNDEFINED_COMMAND, lang), error_callback);
		}
	});

	bot->on_form_submit([](const dpp::form_submit_t& event) {
		get_lang();
		if (event.custom_id == "help_search_modal") {
			event.reply();
			event.edit_original_response(
				cfg::help_message(lang,
					std::get <std::string>(event.components[0].value)[4] - '0'),
				error_callback
			);
		}
	});

	bot->on_message_create([](const dpp::message_create_t& event) -> dpp::task <> {
		const dpp::snowflake user_id = event.msg.author.id;
		// We don't want to reply to any of our own messages.
		if (user_id == bot->me.id) {
			co_return;
		}
		const std::string& msg = event.msg.content;
		const dpp::snowflake channel_id = event.msg.channel_id;
		if (channel_id == TOPGG_WEBHOOK_CHANNEL_ID) {
			const dpp::snowflake voted_user_id = msg.substr(2, msg.size() - bot->me.id.str().size() - 10);
			const int8_t weight = msg[2 + voted_user_id.str().size() + 2] - '0';
			// The vote messages are formatted like this: <@${user_id}> ${weight}>
			// Where ${user_id} is the ID of the user who voted,
			// ${weight} is the amount of points the vote is worth

			const bool failure = co_await topgg::vote(voted_user_id, weight);
			std::lock_guard L(topgg::mutex);
			if (failure && !topgg::no_noguild_reminder[voted_user_id]) {
				// If there was a failure in granting a guild a vote point
				// and the user has not been notified about that before,
				// notify them.
				bot->direct_message_create(voted_user_id, dpp::message("You have just voted and missed out on the chance to vote in favor of a guild! Choosing a guild with `/guild set` and voting for me on top.gg grants it guild points which can then be turned into JTC VCs!"), error_callback);
				topgg::no_noguild_reminder[voted_user_id] = true;
				db::sql << "INSERT INTO no_noguild_reminder VALUES (?);" << voted_user_id.str();
			}
			co_return;
		}
		const dpp::snowflake guild_id = event.msg.guild_id;
		if (event.msg.is_dm()) {
			handle_dm_in(event);
		}
		else if (event.msg.content.find(fmt::format("<@{}>", bot->me.id)) != std::string::npos) {
			event.reply(co_await random_response(user_id), true, error_callback);
		}
		std::lock_guard L2(ticket_mutex);
		if (guild_id == TICKETS_GUILD_ID) {
			handle_dm_out(event);
		}
	});

	bot->on_channel_update([](const dpp::channel_update_t& event) -> void {
		std::lock_guard L(temp_vc_mutex);
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
		const dpp::channel_type type = event.deleted.get_type();
		const dpp::snowflake channel_id = event.deleted.id;
		const dpp::snowflake guild_id = event.deleted.guild_id;
		wait_for_guild_readiness(guild_id);
		if (type == dpp::channel_type::CHANNEL_VOICE) {
			std::scoped_lock L(jtc_mutex, temp_vc_mutex, restriction_mutex);
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
				muted.erase(channel_id);
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
			std::scoped_lock L(notification_mutex, ticket_mutex);
			if (!temp_vc_notifications[guild_id].empty()) {
				db::sql << "DELETE FROM temp_vc_notifications WHERE guild_id=?;" << guild_id.str();
				temp_vc_notifications.erase(event.deleted.guild_id);
			}
			if (!topgg_notifications[guild_id].empty()) {
				db::sql << "DELETE FROM topgg_notifications WHERE guild_id=?;" << guild_id.str();
				topgg_notifications.erase(event.deleted.guild_id);
			}
			if (ck_tickets.contains(channel_id)) {
				db::sql << "DELETE FROM tickets WHERE user_id=?;" << ck_tickets[channel_id].user_id.str();
				tickets.erase(ck_tickets[channel_id].user_id);
				ck_tickets.erase(channel_id);
			}
		}
	});

	bot->on_guild_create([](const dpp::guild_create_t& event) -> void {
		bot->queue_work(event.created.id, [event] {
			cfg::init_guild_channels(event.created.id, event.created.channels);
			ready_guilds.insert(event.created.id);
			guild_readiness_cv.notify_all();
		});
		guild_log(fmt::format("I have joined a guild. These are its stats:\n"
			"Name: `{0}`\nID: `{1}`\nMember count: `{2}`\n Channel count: `{3}`"
			, event.created.name, event.created.id, event.created.member_count, event.created.channels.size()
		));
	});
	bot->on_guild_delete([](const dpp::guild_delete_t& event) -> void {
		if (IS_CLI) {
			return;
		}
		guild_log(fmt::format("I have left a guild. These are its stats:\n"
			"Name: `{0}`\nID: `{1}`\nMember count: `{2}`"
			, event.deleted.name, event.deleted.id, event.deleted.member_count
		));
	});

	bot->on_voice_state_update([](const dpp::voice_state_update_t& event) -> dpp::task <> {
		const dpp::snowflake user_id = event.state.user_id;
		const dpp::snowflake guild_id = event.state.guild_id;
		wait_for_guild_readiness(guild_id);
		std::unique_lock L(temp_vc_mutex);
		dpp::snowflake channel_id = vc_statuses[user_id][guild_id];
		const temp_vc curr_temp_vc = temp_vcs[channel_id];
		L.unlock();
		if (curr_temp_vc.exists()) {
			if ((co_await lookup_channel(channel_id)).get_voice_members().empty()) {
				bot->queue_work(curr_temp_vc.id, std::bind_front([](const dpp::snowflake channel_id) -> dpp::job {
					co_await temp_vc_delete_with_msg(channel_id);
				}, channel_id));
			}
		}
		else {
			std::lock_guard L(temp_vc_mutex);
			temp_vcs.erase(channel_id);
		}
		if (!event.state.channel_id.empty()) {
			std::lock_guard L(temp_vc_mutex);
			vc_statuses[user_id][guild_id] = event.state.channel_id;
		}
		else {
			std::lock_guard L(temp_vc_mutex);
			vc_statuses[user_id].erase(guild_id);
		}
		channel_id = event.state.channel_id;
		if (!channel_id.empty()) {
			std::unique_lock jtc_lock(jtc_mutex);
			if (!jtc_vcs[channel_id].empty()) {
				jtc_lock.unlock();
				co_await temp_vc_create(event);
			}
			else {
				jtc_vcs.erase(channel_id);
			}
		}
	});

	bot->on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task <> {
		get_lang();
		const dpp::snowflake guild_id = event.command.guild_id;
		const dpp::snowflake user_id = event.command.usr.id;
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
			std::string file_name = cmd.options[0].name;
			file_name = file_name == "dpp" ? "other_logs.log" : file_name == "mine" ? "my_logs.log" : file_name == "guild" ? "guild_logs.log" : "sql_logs.log";
			std::lock_guard L(cfg_values_mutex);
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
			std::lock_guard L(cfg_values_mutex);
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
			const std::string& subcommand = cmd.options[0].name;
			if (subcommand == "get") {
				co_await slash::topgg::guild_get(event);
			}
			else if (subcommand == "set") {
				slash::topgg::guild_set(event);
			}
		}
		else if (cmd_name == "get") {
			slash::topgg::get_progress(event);
		}
		else if (cmd_name == "tempvc") {
			const std::string& subcommand = cmd.options[0].name;
			if (subcommand == "set") {
				co_await slash::tempvc::set(event);
			}
			else if (subcommand == "blocklist" || subcommand == "mutelist") {
				const std::string& suboption = cmd.options[0].options[0].name;
				restrictions_types rest_type;
				if (subcommand == "blocklist") {
					rest_type = RRT_BLOCKLIST;
				}
				else {
					rest_type = RRT_MUTELIST;
				}
				if (suboption == "add") {
					co_await slash::tempvc::list::add(event, rest_type);
				}
				if (suboption == "remove") {
					co_await slash::tempvc::list::remove(event, rest_type);
				}
				if (suboption == "status") {
					co_await slash::tempvc::list::status(event, rest_type);
				}
			}
		}
		else if (cmd_name == "set") {
			co_await slash::set::default_values(event);
		}
		else if (cmd_name == "setup") {
			std::shared_lock L1(slash::in_progress_mutex);
			const bool creation_status = slash::in_progress[cmd_name].contains(guild_id);
			if (creation_status) {
				event.reply(response_emsg(A_CHANNEL_IS_ALREADY_BEING_SET_UP, lang), error_callback);
				co_return;
			}
			L1.unlock();
			std::unique_lock L2(slash::in_progress_mutex);
			slash::in_progress[cmd_name].insert(guild_id);
			L2.unlock();
			wait_for_guild_readiness(guild_id);
			co_await slash::setup(event);
			std::lock_guard L3(slash::in_progress_mutex);
			slash::in_progress[cmd_name].erase(guild_id);
		}
		else if (cmd_name == "ticket") {
			std::shared_lock L1(slash::in_progress_mutex);
			const bool management_status = slash::in_progress[cmd_name].contains(user_id);
			if (management_status) {
				event.reply(response_emsg(A_TICKET_IS_ALREADY_BEING_SET_UP, lang), error_callback);
				co_return;
			}
			L1.unlock();
			std::unique_lock L2(slash::in_progress_mutex);
			slash::in_progress[cmd_name].insert(user_id);
			L2.unlock();
			wait_for_guild_readiness(TICKETS_GUILD_ID);
			const std::string& subcommand = cmd.options[0].name;
			if (subcommand == "create") {
				co_await slash::ticket::create(event);
			}
			else /*if (subcommand == "close")*/ {
				slash::ticket::close(event);
			}
			std::unique_lock L3(slash::in_progress_mutex);
			slash::in_progress[cmd_name].erase(user_id);
		}
		else if (cmd_name == "reload") {
			log("Started reloading...");
			cfg::read_config();
			for (const dpp::guild* guild : dpp::get_guild_cache()->get_container() | std::views::values) {
				cfg::init_guild_channels(guild->id, guild->channels);
			}
			cfg::init_db_data();
			cfg::write_down_slashcommands();
			if (!db::connection_successful()) {
				event.reply(dpp::message("COULDN'T CONNECT TO THE DATABASE! THIS IS A DISASTER! RUN WHILE YOU CAN!").set_flags(dpp::m_ephemeral), error_callback);
				log("Reload: COULDN'T CONNECT TO THE DATABASE! THIS IS A DISASTER! RUN WHILE YOU CAN!");
				co_return;
			}
			event.reply(dpp::message("Reloaded").set_flags(dpp::m_ephemeral), error_callback);
			log("Finished reloading.");
		}
		else if (cmd_name == "dumpq") {
			std::queue <temp_vc_query> copy;
			{
				std::lock_guard L(temp_vc_mutex);
				copy = temp_vcs_queue;
			}
			std::stringstream response;
			response << fmt::format("dumping the {} queries\n", copy.size());
			while (!copy.empty()) {
				const temp_vc_query& q = copy.front();
				response << q.id << " {\n"
				         << "channel ID: " << q.channel_id << '\n'
				         << "guild ID: " << q.guild_id << '\n'
				         << "user ID: " << q.user_id << "\n}\n";
				copy.pop();
			}
			event.reply(response.str());
		}
		else {
			event.reply(response_emsg(UNDEFINED_COMMAND, lang), error_callback);
		}
	});

	std::signal(SIGINT, [](const int code) -> void {
		last_signal = code;
		bomb_cv.notify_all();
	});

	std::signal(SIGTERM, [](const int code) -> void {
		last_signal = code;
		bomb_cv.notify_all();
	});

	std::signal(SIGSEGV, [](const int code) -> void {
		last_signal = code;
		bomb_cv.notify_all();
	});

	if (!TO_DUMP) {
		std::cout << "Launching the bot.\n";
		log("Launching the bot.");
		bot->start(dpp::st_return);
		std::unique_lock L(bomb_mutex);
		bomb_cv.wait(L, [] { return last_signal != 0 || ready_to_explode; });
		handle_signal(last_signal);
		log("Goodnight!");
		std::cout << "Goodnight!\n";
	}
	else {
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		std::cout << "It's been a second and there is still no dump. Exiting now.\n";
	}
	return exec_verdict;
}
