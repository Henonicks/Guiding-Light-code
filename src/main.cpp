#include "guiding_light/guiding_light.hpp"
#include "guiding_light/handlers/ticket_handler.hpp"
#include "guiding_light/handlers/temp_vc_handler.hpp"
#include "guiding_light/resources/jtc_vc.hpp"
#include "guiding_light/resources/channel.hpp"
#include "guiding_light/internal/cli.hpp"
#include "guiding_light/internal/cfg.hpp"
#include "guiding_light/handlers/responses.hpp"
#include "guiding_light/handlers/slash_funcs.hpp"
#include "guiding_light/internal/signal.hpp"
#include "guiding_light/resources/temp_ping_blocker.hpp"
#include "guiding_light/internal/launch_options.hpp"
#include "guiding_light/internal/config_values.hpp"
#include "guiding_light/database/gl_on_rails.hpp"
#include "guiding_light/resources/noguild_reminder_blocker.hpp"
#include "guiding_light/handlers/topgg.hpp"
#include "guiding_light/database/database.hpp"
#include "guiding_light/database/markdown.hpp"
#include "guiding_light/generic/table_mapper.hpp"

int main(const int argc, char** argv) {
	map_field_lists();
	// Check if we have sqlite3 installed
	if (!exec_subcommands(argc, argv)) {
		// Execute any subcommands provided with <path>/GL_EXE_NAME [subcommands].
		std::cerr << color::rize("Failed to execute some subcommands. Quitting.\n", "Red");
		exit(1);
	}
	cfg::read_config("../Guiding_Light_Config/config.hfg");
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

	if (!TO_DUMP) {
		dpp::cluster conn_test_cluster("", 0, dpp::NO_SHARDS);
		conn_test_cluster.start(dpp::st_return);
		gor::poker temp_poker(GL_ON_RAILS_TOKEN, &conn_test_cluster);
		gor_client = &temp_poker;

		if (IS_CLI) {
			cli::enter();
			return 0;
		}
		if (!db::test_conn(std::format("../database/{}.db", MODE_NAME))) {
			std::cerr << std::format(
				"{0} connection to DB failed! imma js crash ts g 💔🥀", color::rize("ERROR:", "Red")
			) << std::endl;
			return f_failure;
		}

		gor_client = new gor::poker(GL_ON_RAILS_TOKEN, bot);
	}

	bot->on_ready([](const dpp::ready_t&) -> void {
		if (dpp::run_once <struct initialise_bot>()) {
			cfg::init_bot();
		}
	});

	bot->on_typing_start([](const dpp::typing_start_t& event)-> dpp::task <> {
		const dpp::snowflake typing_channel_id = event.typing_channel.id;
		const dpp::snowflake typing_user_id = event.user_id;
		const dpp::snowflake guild_id = event.typing_guild.id;
		if (guild_id.empty()) {
			const dpp::snowflake send_channel_id = (co_await db::get <ticket_t, "", ticket_t::where_user>(typing_user_id)).channel_id;
			if (!send_channel_id.empty()) {
				bot->channel_typing(send_channel_id, error_callback);
			}
		}
		else if (guild_id == TICKETS_GUILD_ID) {
			const dpp::snowflake send_channel_id = (co_await db::get <ticket_t, "", ticket_t::where_channel>(typing_channel_id)).channel_id;
			if (!send_channel_id.empty()) {
				bot->channel_typing(send_channel_id, error_callback);
			}
		}
	});

	bot->on_button_click([](const dpp::button_click_t& event) -> dpp::task <> {
		get_lang();
		const std::string_view button_id = event.custom_id;
		if (button_id == "temp_ping_toggle") {
			const dpp::snowflake user_id = event.command.usr.id;
			const auto blocker = co_await db::get <temp_ping_blocker_t>(user_id);
			if (!blocker.is_valid()) {
				event.reply(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
				co_return;
			}
			bool disallowing_pings = blocker.exists();
			if (disallowing_pings) {
				co_await db::destroy <temp_ping_blocker_t>(user_id);
			}
			else {
				co_await db::create <temp_ping_blocker_t>({user_id});
			}
			disallowing_pings = !disallowing_pings;
			event.reply(response_fmtemsg(NEXT_TIME_THE_PING_WILL_BE, lang,
				disallowing_pings ? response(OFF, lang) : response(ON, lang))
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
								.set_label(std::format("{0}. {1}", i + 1, pages[i]))
								.set_value(std::format("help{}", i))
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
			if (!ENABLE_TOPGG) {
				const dpp::snowflake voted_user_id = msg.substr(2, msg.size() - bot->me.id.str().size() - 10);
				const int8_t weight = msg[2 + voted_user_id.str().size() + 2] - '0';
				// The vote messages are formatted like this: <@${user_id}> ${weight}>
				// Where ${user_id} is the ID of the user who voted,
				// ${weight} is the amount of points the vote is worth

				co_await topgg::vote(voted_user_id, weight);
			}
			co_return;
		}
		const dpp::snowflake guild_id = event.msg.guild_id;
		if (event.msg.is_dm()) {
			co_await handle_dm_in(event);
		}
		else if (event.msg.content.find(std::format("<@{}>", bot->me.id)) != std::string::npos) {
			event.reply(co_await random_response(user_id), true, error_callback);
		}
		if (guild_id == TICKETS_GUILD_ID) {
			co_await handle_dm_out(event);
		}
	});

	bot->on_channel_update([](const dpp::channel_update_t& event) -> dpp::task <> {
		if ((co_await db::get <temp_vc_t>(event.updated.id)).exists()) {
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

	bot->on_channel_delete([](const dpp::channel_delete_t& event) -> dpp::task <> {
		const dpp::channel_type type = event.deleted.get_type();
		const dpp::snowflake channel_id = event.deleted.id;
		co_await db::destroy <channel_t>(channel_id);
		if (type == dpp::channel_type::CHANNEL_VOICE) {
			std::lock_guard L(restriction_mutex);
			banned.erase(channel_id);
			muted.erase(channel_id);
		}
	});

	bot->on_guild_create([](const dpp::guild_create_t& event) -> void {
		bot->queue_work(event.created.id, std::bind_front([](const dpp::guild_create_t& event) -> dpp::job {
			co_await cfg::init_restriction_lists(event.created.channels);
			std::lock_guard L(ready_mutex);
			ready_guilds.insert(event.created.id);
			guild_readiness_cv.notify_all();
		}, event));
		guild_log(std::format("I have joined a guild. These are its stats:\n"
			"Name: `{0}`\nID: `{1}`\nMember count: `{2}`\n Channel count: `{3}`"
			, event.created.name, event.created.id, event.created.member_count, event.created.channels.size()
		));
	});
	bot->on_guild_delete([](const dpp::guild_delete_t& event) -> void {
		if (IS_CLI) {
			return;
		}
		guild_log(std::format("I have left a guild. These are its stats:\n"
			"Name: `{0}`\nID: `{1}`\nMember count: `{2}`"
			, event.deleted.name, event.deleted.id, event.deleted.member_count
		));
		{
			std::scoped_lock L(
				jtc_vc_t::keep_mutex,
				temp_vc_t::keep_mutex,
				notification_channel_t::keep_mutex
			);
			jtc_vc_t::kept_guilds.erase(event.deleted.id);
			temp_vc_t::kept_guilds.erase(event.deleted.id);
			notification_channel_t::kept_guilds.erase(event.deleted.id);
		}
	});

	bot->on_voice_state_update([](const dpp::voice_state_update_t& event) -> dpp::task <> {
		const dpp::snowflake user_id = event.state.user_id;
		const dpp::snowflake guild_id = event.state.guild_id;
		wait_for_guild_readiness(guild_id);
		std::unique_lock L(temp_vc_mutex);
		dpp::snowflake user_channel_id = vc_statuses[user_id][guild_id];
		// The user was in this channel before they switched to another one.
		L.unlock();
		if (!event.state.channel_id.empty()) {
			std::lock_guard L(temp_vc_mutex);
			vc_statuses[user_id][guild_id] = event.state.channel_id;
		}
		else {
			const auto curr_temp_vc = co_await db::get <temp_vc_t>(user_channel_id);
			if (curr_temp_vc.exists()) {
				if ((co_await lookup_channel(user_channel_id)).get_voice_members().empty()) {
					bot->queue_work(create_temp_vc_id(), std::bind_front([](const dpp::snowflake channel_id) -> dpp::job {
						co_await temp_vc_delete_with_msg(channel_id);
					}, user_channel_id));
				}
			}
			std::lock_guard L(temp_vc_mutex);
			vc_statuses[user_id].erase(guild_id);
		}
		user_channel_id = event.state.channel_id;
		if (!user_channel_id.empty()) {
			const jtc_vc_t jtc_vc = co_await db::get <jtc_vc_t>(user_channel_id);
			if (jtc_vc.exists()) {
				log(std::format("DEBUG: entering temp_vc_create for user {}.", user_id));
				co_await temp_vc_create(event);
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
			log(std::format("Provide the user {} with some help, NOW!", user_id));
			event.reply(cfg::help_message(lang), error_callback);
		}
		else if (cmd_name == "logs") {
			if (user_id != MY_ID) {
				error_log(std::format("User {} is checking the logs! Check your perms!", user_id));
			}
			std::string file_name = cmd.options[0].name;
			file_name = file_name == "dpp" ? "other_logs.log" : file_name == "mine" ? "my_logs.log" : file_name == "guild" ? "guild_logs.log" : "sql_logs.log";
			std::lock_guard L(cfg_values_mutex);
			const dpp::message message = dpp::message().add_file(file_name, dpp::utility::read_file(std::format("{0}/{1}/{2}", logs_directory, MODE_NAME, file_name))).set_flags(dpp::m_ephemeral);
			event.reply(message, error_callback);
		}
		else if (cmd_name == "select") {
			const std::string table_name = cmd.options[0].name;
			const auto markdown = co_await markdown_t::generate_md(table_name);
			if (!markdown.is_valid()) {
				event.reply(dpp::message("Error").set_flags(dpp::m_ephemeral));
				co_return;
			}
			event.reply(dpp::message().add_file(std::format("{}.md", table_name), markdown.content).set_flags(dpp::m_ephemeral));
		}
		else if (cmd_name == "vote") {
			log(std::format("User {} wants to vote, it seems!", user_id));
			event.reply(response_fmtemsg(VOTE_HERE, lang, bot->me.id.str(), slash::get_mention("help")), error_callback);
			co_return;
		}
		else if (cmd_name == "guild") {
			const std::string& subcommand = cmd.options[0].name;
			if (subcommand == "get") {
				co_await slash::topgg::guild_get(event);
			}
			else if (subcommand == "set") {
				co_await slash::topgg::guild_set(event);
			}
		}
		else if (cmd_name == "get") {
			co_await slash::topgg::get_progress(event);
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
			const bool currently_creating = slash::in_progress[cmd_name].contains(guild_id);
			if (currently_creating) {
				event.reply(response_emsg(A_CHANNEL_IS_ALREADY_BEING_SET_UP, lang), error_callback);
				co_return;
			}
			slash::in_progress[cmd_name].insert(guild_id);
			L1.unlock();
			try {
				event.thinking(true);
				wait_for_guild_readiness(guild_id);
				co_await slash::setup(event);
			}
			catch (const std::exception& e) {
				event.edit_original_response(response_emsg(SOMETHING_WENT_TERRIBLY_WRONG, lang), error_callback);
				error_log(std::format("Uncaught exception while setting up a JTC-related channel: {}", e.what()));
			}
			std::lock_guard L2(slash::in_progress_mutex);
			slash::in_progress[cmd_name].erase(guild_id);
		}
		else if (cmd_name == "ticket") {
			std::shared_lock L1(slash::in_progress_mutex);
			const bool currently_managing = slash::in_progress[cmd_name].contains(user_id);
			if (currently_managing) {
				event.reply(response_emsg(A_TICKET_IS_ALREADY_BEING_SET_UP, lang), error_callback);
				co_return;
			}
			slash::in_progress[cmd_name].insert(user_id);
			L1.unlock();
			try {
				const std::string& subcommand = cmd.options[0].name;
				if (subcommand == "create") {
					co_await slash::ticket::create(event);
				}
				else /*if (subcommand == "close")*/ {
					co_await slash::ticket::close(event);
				}
			}
			catch (const std::exception& e) {
				event.edit_original_response(response_emsg(SOMETHING_WENT_TERRIBLY_WRONG, lang), error_callback);
				error_log(std::format("Uncaught exception while managing a ticket: {}", e.what()));
			}
			std::unique_lock L2(slash::in_progress_mutex);
			slash::in_progress[cmd_name].erase(user_id);
		}
		else if (cmd_name == "reload") {
			log("Started reloading...");
			cfg::read_config("../Guiding_Light_Config/config.hfg");
			cfg::write_down_slashcommands();
			if (!db::test_conn(std::format("../database/{}.db", MODE_NAME))) {
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
				copy = temp_vc_queue;
			}
			std::stringstream response;
			response << std::format("dumping the {} queries\n", copy.size());
			while (!copy.empty()) {
				const temp_vc_query& q = copy.front();
				response << q.id - LOWEST_QUERY_ID << " {\n"
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
	return exec_verdict;
}
