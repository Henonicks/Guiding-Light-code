#include "guidingLight/guiding_light.h"
#include <random>
#include <csignal>
#include "file_namespace.h"
#include "configuration.h"
#include "ticket_handler.h"
#include "temp_vc_handler.h"
#include "logging.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
std::unordered_map <dpp::snowflake, dpp::guild> all_bot_guilds;

int main(int argc, char** argv) {
	std::set <std::string> command_list =
			{"./guidingLight", "--return", "--noreturn", "--dev"};
	std::set <std::string> slashcommand_list =
			{"--chelp", "--csetup", "--cset", "--cguild", "--cget", "--cvote", "--cblocklist", "--clogs", "--cticket", "--call"};
	command_list.insert(slashcommand_list.begin(), slashcommand_list.end());
	std::vector <std::string> commands;

	for (int i = 0; i < argc; i++) {
		if (!command_list.count(std::string(argv[i]))) {
			std::cout << "Unknown command: " << argv[i] << '\n';
		}
		commands.emplace_back(argv[i]);
		if (strcmp(argv[i], "--return") == 0) {
			bot_return = dpp::st_return;
		}
		else if (strcmp(argv[i], "--dev") == 0) {
			is_dev = true;
		}
	}

	configuration::configure_bot(is_dev);
	dpp::cluster bot(BOT_TOKEN, dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content | dpp::i_guild_webhooks | dpp::i_guild_messages);
	bot.on_log([&bot](const dpp::log_t& log) -> void {
		bot_log(log, bot);
	});

	auto error_callback = [&bot](const dpp::confirmation_callback_t& callback) -> void {
		if (callback.is_error()) {
			if (!callback.get_error().errors.empty()) {
				bot.log(dpp::loglevel::ll_error, fmt::format("FIELD: {0} REASON: {1}", callback.get_error().errors[0].field, callback.get_error().errors[0].reason));
			}
			else {
				bot.log(dpp::loglevel::ll_error, callback.get_error().message);
			}
		}
	};

	/* Register slash commands here in on_ready */
	bot.on_ready([&bot, commands, error_callback](const dpp::ready_t& event) -> void {
		if (dpp::run_once <struct register_bot_commands>()) {
			bot.start_timer([&bot](const dpp::timer& h) -> void {
				bot.set_presence(dpp::presence(dpp::ps_idle, dpp::activity(dpp::activity_type::at_watching, "VCs in " + std::to_string(all_bot_guilds.size()) + " guilds", "", "")));
			}, 180);

			dpp::slashcommand help("help", "See what I can do!", bot.me.id);
			dpp::slashcommand setup("setup", "Set up a part of JTC feature.", bot.me.id);
			dpp::slashcommand set("set", "Edit an attribute of the temp VC you are in (or of a JTC).", bot.me.id);
			dpp::slashcommand guild("guild", "Get/set the guild you're going to vote in favor of.", bot.me.id);
			dpp::slashcommand get("get", "Get the voting progress of a guild.", bot.me.id);
			dpp::slashcommand vote("vote", "Show the top.gg vote link.", bot.me.id);
			dpp::slashcommand logs("logs", "Drop the logs of choice.", bot.me.id);
			dpp::slashcommand blocklist("blocklist", "Add/Remove a user from your channel's blocklist", bot.me.id);
			dpp::slashcommand ticket("ticket", "Create/Delete a ticket.", bot.me.id);

			std::vector <dpp::slashcommand> slashcommands_to_create;

			set.add_option(
				dpp::command_option(dpp::co_sub_command, "name", "Change the VC name.").
					add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VC to have.", true))
			);
			set.add_option(
				dpp::command_option(dpp::co_sub_command, "limit", "Change the member count limit.").
					add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VC to have.", true))
			);
			set.add_option(
				dpp::command_option(dpp::co_sub_command, "bitrate", "Change the bitrate of the VC.").
					add_option(dpp::command_option(dpp::co_integer, "bitrate", "The bitrate you want the VC to have.", true))
			);

			//---------------------------------------------------
			dpp::command_option sub_cmd_group_default = dpp::command_option(dpp::co_sub_command_group, "default", "Change a default attribute of temp VCs.");

			dpp::command_option name_sub_cmd = dpp::command_option(dpp::co_sub_command, "name", "Change default name of temp VCs.");
			name_sub_cmd.add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VCs to have.", true));
			name_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
			sub_cmd_group_default.add_option(name_sub_cmd);

			dpp::command_option limit_sub_cmd = dpp::command_option(dpp::co_sub_command, "limit", "Change default limit of temp VCs.");
			limit_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VCs to have.", true));
			limit_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
			sub_cmd_group_default.add_option(limit_sub_cmd);

			dpp::command_option bitrate_sub_cmd = dpp::command_option(dpp::co_sub_command, "bitrate", "Change default name of temp VCs.");
			bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "bitrate","The bitrate you want the VCs to have.", true));
			bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
			sub_cmd_group_default.add_option(bitrate_sub_cmd);
			//---------------------------------------------------

			set.add_option(sub_cmd_group_default);

			setup.add_option(
				dpp::command_option(dpp::co_sub_command, "jtc", "Setup a JTC voice channel.").
					add_option(dpp::command_option(dpp::co_string, "maxmembers", "The max number of members in temporary VCs created from this one.", true))
			);

			dpp::command_option notifications_subcommands = dpp::command_option(dpp::co_sub_command_group, "notifications", "Setup a notification channel.");
			notifications_subcommands.add_option(dpp::command_option(dpp::co_sub_command, "jtc", "Setup a notification channel for JTCs."));
			notifications_subcommands.add_option(dpp::command_option(dpp::co_sub_command, "topgg", "Setup a notification channel for top.gg votes."));

			setup.add_option(notifications_subcommands);

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

			logs.add_option(dpp::command_option(dpp::co_sub_command, "dpp", "D++ logs, sent by bot.on_log()."));
			logs.add_option(dpp::command_option(dpp::co_sub_command, "mine", "Logs, written by me."));
			logs.add_option(dpp::command_option(dpp::co_sub_command, "guild", "Guild logs."));
			logs.set_default_permissions(dpp::permissions::p_administrator);

			ticket.add_option(dpp::command_option(dpp::co_sub_command, "create", "Create a support ticket."));
			ticket.add_option(dpp::command_option(dpp::co_sub_command, "close", "Delete a support ticket."));
			ticket.set_dm_permission(true);

			for (const std::string& s : commands) {
				if (s == "--call") {
					slashcommands_to_create.clear();
					slashcommands_to_create.push_back(help);
					slashcommands_to_create.push_back(setup);
					slashcommands_to_create.push_back(set);
					slashcommands_to_create.push_back(guild);
					slashcommands_to_create.push_back(get);
					slashcommands_to_create.push_back(vote);
					slashcommands_to_create.push_back(blocklist);
					slashcommands_to_create.push_back(ticket);
				}
				if (s == "--chelp") {
					slashcommands_to_create.push_back(help);
				}
				if (s == "--csetup") {
					slashcommands_to_create.push_back(setup);
				}
				if (s == "--cset") {
					slashcommands_to_create.push_back(set);
				}
				if (s == "--cguild") {
					slashcommands_to_create.push_back(guild);
				}
				if (s == "--cget") {
					slashcommands_to_create.push_back(get);
				}
				if (s == "--cvote") {
					slashcommands_to_create.push_back(vote);
				}
				if (s == "--cblocklist") {
					slashcommands_to_create.push_back(blocklist);
				}
				if (s == "--clogs" || s == "--call") {
					bot.guild_command_create(logs, MY_GUILD_ID, error_callback);
				}
				if (s == "--cticket") {
					slashcommands_to_create.push_back(ticket);
				}
				if (s == "--call") {
					break;
				}
			}
			if (!slashcommands_to_create.empty()) {
				bot.global_bulk_command_create(slashcommands_to_create, error_callback);
			}
		}
	});

	bot.on_button_click([&bot](const dpp::button_click_t& event) -> void {
		if (event.custom_id == "temp_ping_toggle") {
			const dpp::snowflake& userid = event.command.usr.id;
			if (no_temp_ping[userid]) {
				file::delete_line_once(userid.str(), file::no_temp_ping);
			}
			else {
				file::line_append(userid.str(), file::no_temp_ping);
			}
			no_temp_ping[userid] = !no_temp_ping[userid];
			event.reply(dpp::message(event.command.channel_id, (fmt::format("Next time the ping will be: **{}**.", no_temp_ping[userid] == true ? "off" : "on"))).set_flags(dpp::m_ephemeral));
		}
	});

	bot.on_message_create([&bot](const dpp::message_create_t& event) -> void {
		const dpp::snowflake& userid = event.msg.author.id;
		if (userid == bot.me.id) {
			return;
		}
		const dpp::snowflake& channel_id = event.msg.channel_id;
		const std::string& msg = event.msg.content;
		const dpp::snowflake& guild_id = event.msg.guild_id;
		if (event.msg.is_dm()) {
			handle_dm_in(userid, event);
		}
		if (guild_id == TICKETS_GUILD_ID) {
			handle_dm_out(event);
		}
		if (channel_id == TOPGG_WEBHOOK_CHANNEL_ID) {
			const dpp::snowflake user_id = msg.substr(2, msg.size() - bot.me.id.str().size() - 10);
			bool weekend = msg[2 + user_id.str().size() + 2] == 't';
			bool failure = topgg::vote(user_id, weekend, bot);
			if (failure && !topgg::noguild_reminders[user_id]) {
				bot.direct_message_create(user_id, dpp::message("You have just voted and missed out on the chance to vote in favor of a guild! Choosing a guild with `/guild set` and voting for me on top.gg grants it guild points which can then be turned into JTC VCs!"));
				topgg::noguild_reminders[user_id] = true;
				file::line_append(user_id.str(), file::no_noguild_reminder);
			}
		}
	});

	bot.on_channel_update([&bot](const dpp::channel_update_t& event) -> void {
		if (jtc_channels_map[event.updated.id] != dpp::channel{}) {
			jtc_channels_map[event.updated.id] = event.updated;
		}
		if (!temp_vcs[event.updated.id].channelid.empty()) {
			auto unbanned = banned[event.updated.id];
			bool flag{};
			for (const auto& x : event.updated.permission_overwrites) {
				if (banned[event.updated.id].count(x.id) && (x.allow.can(dpp::p_view_channel) || !x.deny.can(dpp::p_view_channel))) {
					flag = true;
					banned[event.updated.id].erase(x.id);
				}
				if (x.deny.can(dpp::p_view_channel)) {
					flag = true;
					banned[event.updated.id].insert(x.id);
				}
				if (unbanned.count(x.id)) {
					unbanned.erase(x.id);
				}
			}
			for (const auto& x : unbanned) {
				flag = true;
				if (banned[event.updated.id].count(x)) {
					banned[event.updated.id].erase(x);
				}
			}
			if (flag) {
				bot.message_create(dpp::message(event.updated.id, "The blocklist of this channel has been updated."));
			}
		}
	});

	bot.on_channel_delete([](const dpp::channel_delete_t& event) -> void {
		dpp::channel_type type = event.deleted.get_type();
		dpp::snowflake channelid = event.deleted.id;
		dpp::snowflake guildid = event.deleted.guild_id;
		if (type == dpp::channel_type::CHANNEL_VOICE) {
			jtc_vc to_erase_jtc;
			to_erase_jtc.channelid = 0;
			temp_vc to_erase_temp;
			to_erase_temp.channelid = 0;
			to_erase_jtc = jtc_vcs[channelid];
			to_erase_temp = temp_vcs[channelid];
			if (!to_erase_jtc.channelid.empty()) {
				jtc_defaults jtc_defs_to_erase;
				jtc_defs_to_erase = jtc_default_values[channelid];
				jtc_default_values.erase(channelid);
				jtc_channels_map.erase(channelid);
				jtc_vcs.erase(channelid);
				const std::string jtc_line = file::getline(std::to_string(channelid), file::jtc_vcs, -1);
				file::delete_line_once(jtc_line, file::jtc_vcs);
				const std::string jtc_defs_line = file::getline(std::to_string(jtc_defs_to_erase.channelid), file::jtc_default_values, -1);
				file::delete_line_once(jtc_defs_line, file::jtc_default_values);
			}
			if (!to_erase_temp.channelid.empty()) {
				--temp_vc_amount[guildid];
				temp_vcs.erase(channelid);
			}
		}
		else {
			const bool is_ntf = !ntif_chnls[guildid].empty();
			if (is_ntf) {
				const std::string to_remove = channelid.str() + ' ' + guildid.str();
				file::delete_line_once(to_remove, file::temp_vc_notifications);
				ntif_chnls.erase(event.deleted.guild_id);
			}
			const bool is_topgg_ntf = !topgg_ntif_chnls[guildid].empty();
			if (is_topgg_ntf) {
				const std::string to_remove = channelid.str() + ' ' + guildid.str();
				file::delete_line_once(to_remove, file::topgg_notifications);
				topgg_ntif_chnls.erase(event.deleted.guild_id);
			}
			const bool is_ticket = !ck_tickets[channelid].empty();
			if (is_ticket) {
				const std::string to_remove = channelid.str() + ' ' + guildid.str();
				file::delete_line_once(to_remove, file::tickets);
				tickets.erase(ck_tickets[channelid]);
				ck_tickets.erase(channelid);
			}
		}
	});

	bot.on_guild_create([](const dpp::guild_create_t& event) -> void {
		all_bot_guilds[event.created.id] = event.created;
		guild_log("I have joined a guild. These are its stats:\nName: `" + event.created.name + "`\nID: `" + event.created.id.str() + "`\nMembers amount: `" + std::to_string(event.created.member_count) + "`");
	});
	bot.on_guild_delete([](const dpp::guild_delete_t& event) {
		all_bot_guilds.erase(event.deleted.id);
		guild_log("I have left a guild. These are its stats:\nName: `" + event.deleted.name + "`\nID: `" + event.deleted.id.str() + "`\nMembers amount: `" + std::to_string(event.deleted.member_count) + "`");
	});

	bot.on_voice_state_update([&bot, error_callback](const dpp::voice_state_update_t& event) {
		dpp::snowflake channel_id = event.state.channel_id;
		dpp::snowflake user_id = event.state.user_id;
		dpp::user* ptr = dpp::find_user(user_id);
		dpp::user user = *ptr;
		dpp::snowflake guild_id = event.state.guild_id;
		if (!channel_id.empty()) {
			const bool is_jtc = !jtc_vcs[channel_id].channelid.empty();
			if (is_jtc) {
				const temp_vc_query q = {ptr, channel_id, guild_id};
				temp_vcs_queue.push(q);
				temp_vc_create(&bot, q);
			}
		}
		channel_id = vc_statuses[user_id];
		const bool is_temp = !temp_vcs[channel_id].channelid.empty();
		if (is_temp && dpp::find_channel(channel_id)->get_voice_members().empty()) {
			dpp::channel* channel = dpp::find_channel(channel_id);
			temp_vc_delete_msg(&bot, user, channel);
		}
		vc_statuses[user_id] = event.state.channel_id;
	});

	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) -> dpp::task <void> {
		if (!slash::enabled) {
			event.reply(dpp::message("Patience, I'm preparing! Wait about 5-10 seconds and try again.").set_flags(dpp::m_ephemeral));
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
			if (event.command.usr.id != my_id) {
				bot.direct_message_create(my_id, dpp::message(fmt::format("Ayo {} checking logs wht", event.command.usr.id)));
			}
			std::string_view file_name = (cmd.options[0].name == "dpp" ? "other_logs.log" : (cmd.options[0].name == "mine" ? "my_logs.log" : "guild_logs.log"));
			dpp::message message = dpp::message().add_file(file_name, dpp::utility::read_file(fmt::format("{0}{1}/{2}", logs_directory, logs_suffix, file_name))).set_flags(dpp::m_ephemeral);
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
				co_await slash::set::default_values(bot, event);
			}
			else {
				slash::set::current(bot, event);
			}
		}
		if (cmd_name == "setup") {
			auto& status = slash::in_progress[cmd_name][guild_id];
			if (status) {
				event.reply(dpp::message("A channel is already being set up! Try again when it's done.").set_flags(dpp::m_ephemeral));
				co_return;
			}
			status = true;
			co_await slash::setup(bot, event);
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

	signal(SIGINT, [](int code) -> void {
		log("Ну, все, я пішов спати, бувай, добраніч.");
		system("killall guidingLight");
	});

	bot.start(bot_return);
	return 0;
}

#pragma clang diagnostic pop
