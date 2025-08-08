#include "guidingLight/guiding_light.h"
#include <random>
#include "configuration.h"
#include "ticket_handler.h"
#include "temp_vc_handler.h"
#include "logging.h"
#include "database.h"
#include "commands.h"

std::unordered_map <dpp::snowflake, dpp::guild> all_bot_guilds;

int main(int argc, char** argv) {
	std::vector <std::string> commands;

	exec_subcommands(argc, argv);
	configuration::read_config();
	configuration::init_logs();

	dpp::cluster _bot_release(BOT_TOKEN, IS_CLI ? dpp::i_default_intents : dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content | dpp::i_guild_webhooks | dpp::i_guild_messages);
	bot_release = &_bot_release;
	dpp::cluster _bot_dev(BOT_TOKEN_DEV, IS_CLI ? dpp::i_default_intents : dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content | dpp::i_guild_webhooks | dpp::i_guild_messages);
	bot_dev = &_bot_dev;

	get_bot(bot);
	bot_is_starting = &(!IS_DEV ? bot_release_is_starting : bot_dev_is_starting);
	
	_bot_release.on_log([](const dpp::log_t& log) -> void {
		bot_log(log);
	});

	_bot_dev.on_log([](const dpp::log_t& log) -> void {
		bot_log(log);
	});

	auto error_callback = [](const dpp::confirmation_callback_t& callback) -> void {
		if (callback.is_error()) {
			if (!callback.get_error().errors.empty()) {
				bot->log(dpp::loglevel::ll_error, fmt::format("FIELD: {0} REASON: {1}", callback.get_error().errors[0].field, callback.get_error().errors[0].reason));
			}
			else {
				bot->log(dpp::loglevel::ll_error, callback.get_error().message);
			}
		}
	};

	if (IS_CLI) {
		enter_cli();
	}

	/* Register slash commands here in on_ready */
	bot->on_ready([commands, error_callback](const dpp::ready_t&) -> void {
		if (dpp::run_once <struct register_bot_commands>()) {
			bot->start_timer([](const dpp::timer&) -> void {
				bot->set_presence(dpp::presence(dpp::ps_idle, dpp::activity(dpp::activity_type::at_watching, "VCs in " + std::to_string(all_bot_guilds.size()) + " guilds", "", "")));
			}, 180);
		}
	});

	bot->on_button_click([](const dpp::button_click_t& event) -> void {
		if (IS_CLI) {
			return;
		}
		if (event.custom_id == "temp_ping_toggle") {
			const dpp::snowflake& user_id = event.command.usr.id;
			if (no_temp_ping[user_id]) {
				db::sql << "DELETE FROM no_temp_ping WHERE user_id=?;" << user_id.str();
			}
			else {
				db::sql << "INSERT INTO no_temp_ping VALUES (?);" << user_id.str();
			}
			no_temp_ping[user_id] = !no_temp_ping[user_id];
			event.reply(dpp::message(event.command.channel_id, (fmt::format("Next time the ping will be: **{}**.", no_temp_ping[user_id] == true ? "off" : "on"))).set_flags(dpp::m_ephemeral));
		}
	});

	bot->on_message_create([](const dpp::message_create_t& event) -> void {
		if (IS_CLI) {
			return;
		}
		const dpp::snowflake& user_id = event.msg.author.id;
		if (user_id == bot->me.id) {
			return;
		}
		const dpp::snowflake& channel_id = event.msg.channel_id;
		const std::string& msg = event.msg.content;
		const dpp::snowflake& guild_id = event.msg.guild_id;
		if (event.msg.is_dm()) {
			handle_dm_in(user_id, event);
		}
		if (guild_id == TICKETS_GUILD_ID) {
			handle_dm_out(event);
		}
		if (channel_id == TOPGG_WEBHOOK_CHANNEL_ID) {
			const dpp::snowflake user_id = msg.substr(2, msg.size() - bot->me.id.str().size() - 10);
			bool weekend = msg[2 + user_id.str().size() + 2] == 't';
			bool failure = topgg::vote(user_id, weekend);
			if (failure && !topgg::no_noguild_reminder[user_id]) {
				bot->direct_message_create(user_id, dpp::message("You have just voted and missed out on the chance to vote in favor of a guild! Choosing a guild with `/guild set` and voting for me on top.gg grants it guild points which can then be turned into JTC VCs!"));
				topgg::no_noguild_reminder[user_id] = true;
				db::sql << "INSERT INTO no_noguild_reminder VALUES (?);" << user_id.str();
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
			const bool is_ntf = !temp_vc_notifications[guild_id].empty();
			if (is_ntf) {
				db::sql << "DELETE FROM temp_vc_notifications WHERE guild_id=?;" << guild_id.str();
				temp_vc_notifications.erase(event.deleted.guild_id);
			}
			const bool is_topgg_ntf = !topgg_notifications[guild_id].empty();
			if (is_topgg_ntf) {
				db::sql << "DELETE FROM topgg_notifications WHERE guild_id=?;" << guild_id.str();
				topgg_notifications.erase(event.deleted.guild_id);
			}
			const bool is_ticket = !ck_tickets[channel_id].empty();
			if (is_ticket) {
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

	bot->on_voice_state_update([error_callback](const dpp::voice_state_update_t& event) {
		if (IS_CLI) {
			return;
		}
		dpp::snowflake channel_id = event.state.channel_id;
		const dpp::snowflake& user_id = event.state.user_id;
		dpp::user* ptr = dpp::find_user(user_id);
		const dpp::user user = *ptr;
		const dpp::snowflake& guild_id = event.state.guild_id;
		if (!channel_id.empty()) {
			const bool is_jtc = !jtc_vcs[channel_id].empty();
			if (is_jtc) {
				const temp_vc_query q = {ptr, channel_id, guild_id};
				temp_vcs_queue.push(q);
				temp_vc_create(q);
			}
		}
		channel_id = vc_statuses[user_id];
		const bool is_temp = !temp_vcs[channel_id].channel_id.empty();
		if (is_temp && dpp::find_channel(channel_id)->get_voice_members().empty()) {
			dpp::channel* channel = dpp::find_channel(channel_id);
			temp_vc_delete_msg(user, channel);
		}
		vc_statuses[user_id] = event.state.channel_id;
	});

	bot->on_slashcommand([](const dpp::slashcommand_t& event) -> dpp::task <void> {
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
		dpp::command_interaction cmd = event.command.get_command_interaction();
		if (cmd_name == "help") {
			event.reply(dpp::message(event.command.channel_id, slash::help_embed_1).add_embed(slash::help_embed_2).set_flags(dpp::m_ephemeral));
			co_return;
		}
		if (cmd_name == "logs") {
			if (event.command.usr.id != MY_ID) {
				bot->direct_message_create(MY_ID, dpp::message(fmt::format("Ayo {} checking logs wht", event.command.usr.id)));
			}
			std::string_view file_name = (cmd.options[0].name == "dpp" ? "other_logs.log" : cmd.options[0].name == "mine" ? "my_logs.log" : cmd.options[0].name == "guild" ? "guild_logs.log" : "sql_logs.log");
			const dpp::message message = dpp::message().add_file(file_name, dpp::utility::read_file(fmt::format("{0}{1}/{2}", logs_directory, MODE_NAME, file_name))).set_flags(dpp::m_ephemeral);
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
			system(fmt::format("sqlite3 ../database/{0}.db '.mode markdown' \".output ../database/select/{0}/{1}.md\" \"SELECT * FROM {1}\";", MODE_NAME, table_name).c_str());
			const dpp::message message = dpp::message().add_file("db.md", dpp::utility::read_file(fmt::format("../database/select/{0}/{1}.md", MODE_NAME, table_name))).set_flags(dpp::m_ephemeral);
			event.reply(message);
		}
		if (cmd_name == "vote") {
			event.reply(dpp::message("Vote [here](https://top.gg/bot/1101159652315627551/vote) and earn JTC points for a chosen guild! See `/help` for more information.").set_flags(dpp::m_ephemeral));
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
			bool& status = slash::in_progress[cmd_name][guild_id];
			if (status) {
				event.reply(dpp::message("A channel is already being set up! Try again when it's done.").set_flags(dpp::m_ephemeral));
				co_return;
			}
			status = true;
			co_await slash::setup(event);
			status = false;
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
			auto& status = slash::in_progress[cmd_name][user_id];
			if (status) {
				event.reply(dpp::message("A ticket is already being created! Try again when it's done.").set_flags(dpp::m_ephemeral));
				co_return;
			}
			if (cmd.options[0].name == "create") {
				status = true;
				co_await slash::ticket::create(event);
				status = false;
			}
			if (cmd.options[0].name == "close") {
				slash::ticket::close(event);
			}
		}
	});

	signal(SIGINT, [](int) -> void {
		if (!IS_CLI) {
			log("Ну, все, я пішов спати, бувай, добраніч.");
			std::cout << "Ну, все, я пішов спати, бувай, добраніч." << '\n';
		}
		exit(0);
	});

	bot->start(BOT_RETURN);
	return 0;
}
