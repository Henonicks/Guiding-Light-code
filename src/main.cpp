#include "guiding_light/guiding_light.hpp"
#include "guiding_light/ticket_handler.hpp"
#include "guiding_light/temp_vc_handler.hpp"
#include "guiding_light/launch_options.hpp"
#include "guiding_light/cli.hpp"
#include "guiding_light/ping.hpp"

std::unordered_map <dpp::snowflake, dpp::guild> all_bot_guilds;

int main(const int argc, char** argv) {
	std::vector <std::string> commands;
	// Is this used?

	configuration::check_sqlite3();
	// Check if we have sqlite3 installed
	exec_subcommands(argc, argv);
	// Execute any subcommands provided with ./guidingLight <subcommands>.
	configuration::read_config();
	// Write down the values from the config into variables.
	configuration::init_logs();
	// Open the log files. Create them if needed.

	dpp::cluster _bot_release(BOT_TOKEN, IS_CLI ? dpp::i_default_intents : dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content | dpp::i_guild_webhooks | dpp::i_guild_messages);
	bot_release = &_bot_release;
	dpp::cluster _bot_dev(BOT_TOKEN_DEV, IS_CLI ? dpp::i_default_intents : dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content | dpp::i_guild_webhooks | dpp::i_guild_messages);
	bot_dev = &_bot_dev;

	// In the CLI mode we can switch between the release and the dev modes and then launch the bots.
	// To switch between them, we're gonna write down their addresses in pointers.

	bot = get_bot();
	// Write down the right release/dev address into the bot pointer.
	bot_is_starting = &(!IS_DEV ? bot_release_is_starting : bot_dev_is_starting);
	// also a pointer, it's only used in the CLI mode.
	
	_bot_release.on_log([](const dpp::log_t& log) -> void {
		bot_log(log);
	});

	_bot_dev.on_log([](const dpp::log_t& log) -> void {
		bot_log(log);
	});

	if (IS_CLI) {
		cli::enter();
	}

	/* Register slash commands here in on_ready */
	bot->on_ready([](const dpp::ready_t&) -> void {
		if (IS_CLI) {
			return;
		}
		if (dpp::run_once <struct register_bot_commands>()) {
			bot->start_timer([](const dpp::timer&) -> void {
				bot->set_presence(dpp::presence(dpp::ps_idle, dpp::activity(dpp::activity_type::at_watching, "VCs in " + std::to_string(all_bot_guilds.size()) + " guilds", "", "")));
			}, 180);
			// Keep on setting the presence to update the guild count on it.
		}
	});

	bot->on_button_click([](const dpp::button_click_t& event) -> void {
		if (IS_CLI) {
			return;
		}
		// We don't want to handle a button press twice, do we?
		if (event.custom_id == "temp_ping_toggle") {
			const dpp::snowflake& user_id = event.command.usr.id;
			if (no_temp_ping[user_id]) {
				db::sql << "DELETE FROM no_temp_ping WHERE user_id=?;" << user_id.str();
			}
			else {
				db::sql << "INSERT INTO no_temp_ping VALUES (?);" << user_id.str();
			}
			no_temp_ping[user_id] = !no_temp_ping[user_id];
			event.reply(dpp::message(event.command.channel_id, fmt::format("Next time the ping will be: **{}**.", no_temp_ping[user_id] == true ? "off" : "on")).set_flags(dpp::m_ephemeral));
		}
	});

	bot->on_message_create([](const dpp::message_create_t& event) -> void {
		if (IS_CLI) {
			return;
		}
		// Same as with a button click.
		if (event.msg.author.id == bot->me.id) {
			return;
		}
		// We don't want to reply to any of our own messages.
		const dpp::snowflake& user_id = event.msg.author.id;
		const dpp::snowflake& channel_id = event.msg.channel_id;
		const std::string& msg = event.msg.content;
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
		if (channel_id == TOPGG_WEBHOOK_CHANNEL_ID) {
			const dpp::snowflake voted_user_id = msg.substr(2, msg.size() - bot->me.id.str().size() - 10);
			const bool weekend = msg[2 + voted_user_id.str().size() + 2] == 't';
			// The vote messages are formatted like this: <@${user_id}> ${weekend}> <@${bot_id}>
			// Where ${user_id} is the ID of the user who voted,
			// ${weekend} is a boolean stating whether it's a weekend or not
			// And ${bot_id} is the ID of the bot which was voted for.
			const bool failure = topgg::vote(voted_user_id, weekend);
			if (failure && !topgg::no_noguild_reminder[voted_user_id]) {
				// If there was a failure in granting a guild a vote point
				// and the user has not been notified about that before,
				// notify them.
				bot->direct_message_create(voted_user_id, dpp::message("You have just voted and missed out on the chance to vote in favor of a guild! Choosing a guild with `/guild set` and voting for me on top.gg grants it guild points which can then be turned into JTC VCs!"));
				topgg::no_noguild_reminder[voted_user_id] = true;
				db::sql << "INSERT INTO no_noguild_reminder VALUES (?);" << voted_user_id.str();
			}
		}
	});

	bot->on_channel_update([](const dpp::channel_update_t& event) -> void {
		if (IS_CLI) {
			return;
		}
		if (!temp_vcs[event.updated.id].channel_id.empty()) {
			if (blocklist_updated(event.updated)) {
				bot->message_create(dpp::message(event.updated.id, "The blocklist of this channel has been updated."));
			}
		}
	});

	bot->on_channel_delete([](const dpp::channel_delete_t& event) -> void {
		if (IS_CLI) {
			return;
		}
		const dpp::channel_type type = event.deleted.get_type();
		const dpp::snowflake& channel_id = event.deleted.id;
		const dpp::snowflake& guild_id = event.deleted.guild_id;
		if (type == dpp::channel_type::CHANNEL_VOICE) {
			if (!jtc_vcs[channel_id].empty()) {
				jtc_default_values.erase(channel_id);
				jtc_vcs.erase(channel_id);
				--jtc_vc_amount[guild_id];
				db::sql << "DELETE FROM jtc_vcs WHERE channel_id=?;" << channel_id.str();
				db::sql << "DELETE FROM jtc_default_values WHERE channel_id=?;" << channel_id.str();
			}
			if (!temp_vcs[channel_id].channel_id.empty()) {
				banned.erase(channel_id);
				--temp_vc_amount[guild_id];
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
		if (IS_CLI) {
			return;
		}
		all_bot_guilds[event.created.id] = event.created;
		guild_log("I have joined a guild. These are its stats:\nName: `" + event.created.name + "`\nID: `" + event.created.id.str() + "`\nMembers amount: `" + std::to_string(event.created.member_count) + "`");
	});
	bot->on_guild_delete([](const dpp::guild_delete_t& event) {
		if (IS_CLI) {
			return;
		}
		all_bot_guilds.erase(event.deleted.id);
		guild_log("I have left a guild. These are its stats:\nName: `" + event.deleted.name + "`\nID: `" + event.deleted.id.str() + "`\nMembers amount: `" + std::to_string(event.deleted.member_count) + "`");
	});

	bot->on_voice_state_update([](const dpp::voice_state_update_t& event) {
		if (IS_CLI) {
			return;
		}
		dpp::snowflake channel_id = event.state.channel_id;
		const dpp::snowflake& user_id = event.state.user_id;
		dpp::user* ptr = dpp::find_user(user_id);
		const dpp::user user = *ptr;
		const dpp::snowflake& guild_id = event.state.guild_id;
		if (!channel_id.empty()) {
			if (!jtc_vcs[channel_id].empty()) {
				const temp_vc_query q = {ptr, channel_id, guild_id};
				temp_vcs_queue.push(q);
				temp_vc_create(q);
			}
		}
		channel_id = vc_statuses[user_id];
		if (!temp_vcs[channel_id].channel_id.empty() && dpp::find_channel(channel_id)->get_voice_members().empty()) {
			dpp::channel* channel = dpp::find_channel(channel_id);
			temp_vc_delete_msg(user, channel);
		}
		vc_statuses[user_id] = event.state.channel_id;
	});

	bot->on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task <> {
		if (IS_CLI) {
			co_return;
		}
		if (!slash::enabled) {
			event.reply(dpp::message("I'm preparing! Please wait about 5-10 seconds and try again.").set_flags(dpp::m_ephemeral));
			co_return;
		}
		const dpp::snowflake& guild_id = event.command.guild_id;
		const dpp::snowflake& user_id = event.command.usr.id;
		const std::string cmd_name = event.command.get_command_name();
		const dpp::command_interaction cmd = event.command.get_command_interaction();
		if (cmd_name == "help") {
			event.reply(dpp::message(event.command.channel_id, slash::help_embed_1).add_embed(slash::help_embed_2).set_flags(dpp::m_ephemeral));
			co_return;
		}
		if (cmd_name == "logs") {
			if (event.command.usr.id != MY_ID) {
				bot->direct_message_create(MY_ID, dpp::message(fmt::format("Ayo {} checking logs wht", event.command.usr.id)));
			}
			std::string_view file_name = cmd.options[0].name == "dpp" ? "other_logs.log" : cmd.options[0].name == "mine" ? "my_logs.log" : cmd.options[0].name == "guild" ? "guild_logs.log" : "sql_logs.log";
			const dpp::message message = dpp::message().add_file(file_name, dpp::utility::read_file(fmt::format("{0}/{1}/{2}", logs_directory, MODE_NAME, file_name))).set_flags(dpp::m_ephemeral);
			event.reply(message);
		}
		if (cmd_name == "select") {
			if (event.command.usr.id != MY_ID) {
				bot->direct_message_create(MY_ID, dpp::message(fmt::format("Ayo {} selecting wht", event.command.usr.id)));
			}
			std::string table_name = cmd.options[0].name;
			for (char& x : table_name) {
				if (x == '-') {
					x = '_';
				}
			}
			system(fmt::format(R"(sqlite3 ../database/{0}.db '.mode markdown' ".output ../database/select/{0}/{1}.md" "SELECT * FROM {1}";)", MODE_NAME, table_name).c_str());
			const dpp::message message = dpp::message().add_file("db.md", dpp::utility::read_file(fmt::format("../database/select/{0}/{1}.md", MODE_NAME, table_name))).set_flags(dpp::m_ephemeral);
			event.reply(message);
		}
		if (cmd_name == "vote") {
			event.reply(dpp::message(fmt::format("Vote [here](https://top.gg/bot/{}/vote) and earn JTC points for a chosen guild! See `/help` for more information.", bot->me.id)).set_flags(dpp::m_ephemeral));
			co_return;
		}
		if (cmd_name == "guild") {
			if (cmd.options[0].name == "get") {
				slash::topgg::guild_get(event);
			}
			else if (cmd.options[0].name == "set") {
				slash::topgg::guild_set(event);
			}
		}
		if (cmd_name == "get") {
			slash::topgg::get_progress(event);
		}
		if (cmd_name == "set") {
			if (cmd.options[0].name == "default") {
				co_await slash::set::default_values(event);
			}
			else {
				co_await slash::set::current(event);
			}
		}
		if (cmd_name == "setup") {
			bool& creation_status = slash::in_progress[cmd_name][guild_id];
			if (creation_status) {
				event.reply(dpp::message("A channel is already being set up! Try again when it's done.").set_flags(dpp::m_ephemeral));
				co_return;
			}
			creation_status = true;
			co_await slash::setup(event);
			creation_status = false;
		}
		if (cmd_name == "blocklist") {
			if (cmd.options[0].name == "add") {
				co_await slash::blocklist::add(event);
			}
			if (cmd.options[0].name == "remove") {
				co_await slash::blocklist::remove(event);
			}
			if (cmd.options[0].name == "status") {
				slash::blocklist::status(event);
			}
		}
		if (cmd_name == "ticket") {
			auto& creation_status = slash::in_progress[cmd_name][user_id];
			if (creation_status) {
				event.reply(dpp::message("A ticket is already being created! Try again when it's done.").set_flags(dpp::m_ephemeral));
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
	});

	signal(SIGINT, [](const int) -> void {
		if (!IS_CLI) {
			log("Ну, все, я пішов спати, бувай, добраніч.");
			std::cout << "Ну, все, я пішов спати, бувай, добраніч." << '\n';
			// Ukrainian for "Well, that's it, I'm going to bed, bye, goodnight."
		}
		exit(0);
	});

	bot->start(BOT_RETURN);
	return 0;
}
