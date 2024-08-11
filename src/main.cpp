#include "guidingLight/guiding_light.h"
#include <random>
#include <csignal>
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "temp_vc.h"
#include "slash_funcs.h"
#include "file_namespace.h"
#include "notification_channel.h"
#include "configuration.h"
#include "logging.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
std::unordered_map <dpp::snowflake, dpp::guild> all_bot_guilds;

int main(int argc, char** argv) {
	std::cout << "starting the bot" << std::endl;

	std::set <std::string> command_list =
			{"./guidingLight", "--return", "--noreturn", "--dev"};
	std::set <std::string> slashcommand_list =
			{"--chelp", "--csetup", "--cset"};
	command_list.insert(slashcommand_list.begin(), slashcommand_list.end());
	std::vector <std::string> commands;

	bool bot_return = false;
	bool is_dev = false;

	for (int i = 0; i < argc; i++) {
		if (!command_list.count(std::string(argv[i]))) {
			std::cout << "Unknown command: " << argv[i] << '\n';
		}
		commands.emplace_back(argv[i]);
		if (strcmp(argv[i], "--return") == 0) {
			bot_return = true;
		}
		else if (strcmp(argv[i], "--dev") == 0) {
			is_dev = true;
		}
	}

	configuration::configure_bot(is_dev);
	dpp::cluster bot(BOT_TOKEN, dpp::i_guilds | dpp::i_guild_members | dpp::i_guild_voice_states | dpp::i_direct_messages | dpp::i_message_content);
	bot.on_log([&bot](const dpp::log_t& log) -> void {
		bot_log(log, bot);
	});

	auto error_callback = [&bot](const dpp::confirmation_callback_t& callback) -> void {
		if (callback.is_error()) {
			bot.log(dpp::loglevel::ll_error, callback.get_error().message);
		}
	};

	/* Register slash commands here in on_ready */
	bot.on_ready([&bot, commands, error_callback](const dpp::ready_t& event) -> void {
		if (dpp::run_once <struct register_bot_commands>()) {
			bot.start_timer([&bot](const dpp::timer& h) -> void {
				bot.set_presence(dpp::presence(dpp::ps_idle, dpp::activity(dpp::activity_type::at_watching, "VCs in " + std::to_string(all_bot_guilds.size()) + " guilds", "", "")));
			}, 180);

			dpp::slashcommand help("help", "See what I can do!", bot.me.id);
			dpp::slashcommand setup("setup", "Set up a part of JTC feature", bot.me.id);
			dpp::slashcommand set("set", "Edit an attribute of the temp VC you are in (or of a JTC).", bot.me.id);

			std::vector <dpp::slashcommand> slashcommands_to_create;

			set.add_option(
					dpp::command_option(dpp::co_sub_command, "name", "Change the VC name").
							add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VC to have", true))
			);
			set.add_option(
					dpp::command_option(dpp::co_sub_command, "limit", "Change the member count limit").
							add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VC to have", true))
			);
			set.add_option(
					dpp::command_option(dpp::co_sub_command, "bitrate", "Change the bitrate of the VC").
							add_option(dpp::command_option(dpp::co_integer, "bitrate", "The bitrate you want the VC to have", true))
			);

			//---------------------------------------------------
			dpp::command_option sub_cmd_group_default = dpp::command_option(dpp::co_sub_command_group, "default", "Change a default attribute of temp VCs");

			dpp::command_option name_sub_cmd = dpp::command_option(dpp::co_sub_command, "name", "Change default name of temp VCs");
			name_sub_cmd.add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VCs to have", true));
			name_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed", true));
			sub_cmd_group_default.add_option(name_sub_cmd);

			dpp::command_option limit_sub_cmd = dpp::command_option(dpp::co_sub_command, "limit", "Change default limit of temp VCs");
			limit_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VCs to have", true));
			limit_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed", true));
			sub_cmd_group_default.add_option(limit_sub_cmd);

			dpp::command_option bitrate_sub_cmd = dpp::command_option(dpp::co_sub_command, "bitrate", "Change default name of temp VCs");
			bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "bitrate","The bitrate you want the VCs to have", true));
			bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed", true));
			sub_cmd_group_default.add_option(bitrate_sub_cmd);
			//---------------------------------------------------

			set.add_option(sub_cmd_group_default);

			setup.add_option(
					dpp::command_option(dpp::co_sub_command, "jtc", "Setup a JTC voice channel").
							add_option(dpp::command_option(dpp::co_string, "maxmembers", "The max number of members in temporary VCs created from this one", true))
			);
			setup.add_option(
					dpp::command_option(dpp::co_sub_command_group, "notification", "Setup a notification channel").
							add_option(dpp::command_option(dpp::co_sub_command, "channel", "Setup a notification channel"))
			);

			for (const std::string& s : commands) {
				if (s == "--chelp") {
					slashcommands_to_create.push_back(help);
				}
				if (s == "--csetup") {
					slashcommands_to_create.push_back(setup);
				}
				if (s == "--cset") {
					slashcommands_to_create.push_back(set);
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
		std::string msg = event.msg.content;
		std::string prefix;
		dpp::snowflake userid = event.msg.author.id;
		const std::string username = event.msg.author.username;
		const std::string usertag = '#' + std::to_string(event.msg.author.discriminator);
		const std::string usernt = username + usertag;
		const std::string str_userid = "<@" + std::to_string(userid) + '>';
		if (event.msg.is_dm()) {
			if (userid != dpp::snowflake(bot.me.id)) {
				bot.message_create(dpp::message(bot_dm_logs, "<@" + std::to_string(userid) + "> (" + event.msg.author.format_username() + ") [to me] " + event.msg.content), [&bot, userid](const dpp::confirmation_callback_t& callback) -> void {
					bot.direct_message_create(userid, dpp::message("Message is received!"), [&bot](const dpp::confirmation_callback_t& callback) {
						if (callback.is_error()) {
							bot.log(dpp::loglevel::ll_error, callback.http_info.body);
							return;
						}
						auto msg = std::get <dpp::message>(callback.value);
						bot.start_timer([&bot, msg](dpp::timer timer) -> void {
							bot.message_delete(msg.id, msg.channel_id);
							bot.stop_timer(timer);
						}, 2);
					});
				});
			}
		}
		if (msg.find("!dm ") == 0 && event.msg.author.id == my_id) {
			std::string dm_who_str;
			int i;
			for (i = 4; i < msg.size(); i++) {
				if (msg[i] == ' ') {
					break;
				}
				dm_who_str += msg[i];
			}
			auto dm_who = dpp::snowflake(dm_who_str);
			dpp::user* user = dpp::find_user(dm_who);
			if (user == nullptr) {
				event.reply("Screw you, the user is unknown!");
				return;
			}
			std::string content;
			for (; i < msg.size(); i++) {
				content += msg[i];
			}
			if (content.empty()) {
				std::getline(std::cin >> std::ws, content);
			}
			bot.direct_message_create(dm_who, dpp::message(content));
			bot.message_create(dpp::message(bot_dm_logs, fmt::format("[me to] <@{0}> (``{1}``) \'{2}\'", dm_who_str, user->username, content)));
			bot.log(dpp::loglevel::ll_info, fmt::format("dming {0} \'{1}\'", dm_who, content));
		}
		if (msg.size() > 4) {
			for (int i = 1; i <= 4; i++) {
				prefix += msg[i];
			}
		}
	});

	bot.on_channel_update([](const dpp::channel_update_t& event) -> void {
		if (jtc_channels_map[event.updated->id] != dpp::channel{}) {
			jtc_channels_map[event.updated->id] = *event.updated;
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
				const std::string jtc_line = std::to_string(to_erase_jtc.channelid) + ' ' + std::to_string(to_erase_jtc.guildid);
				file::delete_line_once(jtc_line, file::jtc_vcs);
				const std::string jtc_defs_line = file::getline(std::to_string(jtc_defs_to_erase.channelid), file::jtc_default_values, -1);
				file::delete_line_once(jtc_defs_line, file::jtc_default_values);
			}
			if (!to_erase_temp.channelid.empty()) {
				temp_vcs.erase(channelid);
			}
		}
		else {
			bool is_ntf = !ntif_chnls[guildid].channelid.empty();
			if (is_ntf) {
				std::string to_remove = file::getline(std::to_string(event.deleted.guild_id), file::temp_vc_notifications, 0);
				file::delete_line_once(to_remove, file::temp_vc_notifications);
				ntif_chnls.erase(event.deleted.guild_id);
			}
		}
	});

	bot.on_guild_create([](const dpp::guild_create_t& event) -> void {
		all_bot_guilds[event.created->id] = *event.created;
		guild_log("I have joined a guild. These are its stats:\nName: `" + event.created->name + "`\nID: `" + std::to_string(event.created->id) + "`");
	});
	bot.on_guild_delete([](const dpp::guild_delete_t& event) {
		all_bot_guilds.erase(event.deleted.id);
		guild_log("I have left a guild. These are its stats:\nName: `" + event.deleted.name + "`\nID: `" + std::to_string(event.deleted.id) + "`");
	});

	bot.on_voice_state_update([&bot](const dpp::voice_state_update_t& event) -> void {
		dpp::snowflake userid = event.state.user_id;
		dpp::user* ptr = dpp::find_user(userid);
		dpp::snowflake channelid = event.state.channel_id;
		if (ptr == nullptr) {
			log(fmt::format("User {0} not found. Channel: {1}", userid, channelid));
			if (!channelid.empty()) {
				bot.message_create(dpp::message(channelid, fmt::format("<@{}> I could not find your user information. Leave and try again in a few seconds.", userid)).set_allowed_mentions(true));
			}
			return;
		}
		dpp::user user = *ptr;
		dpp::snowflake guildid = event.state.guild_id;
		if (!channelid.empty()) {
			bool is_jtc = !jtc_vcs[channelid].channelid.empty();
			if (is_jtc) {
				bool to_return = false;
				uint64_t current_time = bot.uptime().to_secs();
				if (current_time - join_time[userid] <= 5) {
					bot.direct_message_create(userid, dpp::message("You have to wait more before joining the voice channel. To create a temporary VC as soon as possible, disconnect and try again in 5 seconds."));
					to_return = true;
				}
				join_time[userid] = current_time;
				if (to_return) {
					return;
				}
				std::string username = user.username;
				std::string new_name;
				dpp::channel new_channel;
				new_channel.set_type(dpp::channel_type::CHANNEL_VOICE);
				jtc_defaults defs = jtc_default_values[channelid];
				for (int i = 0; i < defs.name.size(); i++) {
					if (defs.name[i] == '{') {
						if (defs.name.size() - i >= 10) { // text {username}
							std::string temp_string;	  // 0123456789	14
							for (int j = i; j < i + 10; j++) {
								temp_string += defs.name[j];
							}
							if (temp_string == "{username}") {
								new_name += username;
								i += 9;
								continue;
							}
						}
					}
					else if (defs.name[i] == '_') {
						new_name += ' ';
						continue;
					}
					new_name += defs.name[i];
				}
				int limit = (int)defs.limit;
				new_channel.set_name(new_name);
				new_channel.set_guild_id(guildid);
				new_channel.set_bitrate(defs.bitrate);
				dpp::channel current = jtc_channels_map[channelid];
				new_channel.set_parent_id(current.parent_id);
				if (limit == 100) {
					limit = 0;
				}
				new_channel.set_user_limit(limit);
				bot.channel_create(new_channel,[&bot, current, userid, user, guildid, event](const dpp::confirmation_callback_t& callback) -> void {
					auto newchannel = std::get <dpp::channel>(callback.value);
					dpp::snowflake channelid = newchannel.id;
					if (!no_temp_ping[userid]) {
						bot.message_create(dpp::message(channelid, user.get_mention()).set_allowed_mentions(true), [&bot](const dpp::confirmation_callback_t& callback) -> void {
							auto new_message = callback.get <dpp::message>();
							bot.message_delete(new_message.id, new_message.channel_id);
						});
					}
					dpp::embed temp_ping_embed = dpp::embed()
						.set_color(dpp::colors::sti_blue)
						.set_title(fmt::format("Welcome to {0}!", newchannel.get_mention()))
						.set_author(fmt::format("This VC belongs to {}.", user.username), user.get_url(), user.get_avatar_url())
						.add_field(
							"You're able to edit the channel!",
							"Use a subcommand of the `/set` command to change the name, limit, or bitrate of your channel to whatever value your soul desires. See `/help` (not to be confused with \"seek help\") for more information."
						)
						.set_footer(
							dpp::embed_footer()
							.set_text("Use the button bellow to toggle the temporary VC creationg ping on/off. Have fun!")
					);
					dpp::message message = dpp::message(channelid, temp_ping_embed).add_component(
						dpp::component().add_component(
							dpp::component()
								.set_type(dpp::cot_button)
								.set_emoji("ping", 1271923808739000431)
								.set_style(dpp::cos_danger)
								.set_id("temp_ping_toggle")
						)
					);
					bot.message_create(message);
				   	temp_vcs[newchannel.id] = {newchannel.id, newchannel.guild_id, userid};
				   	bot.guild_member_move(newchannel.id, newchannel.guild_id, userid, [&bot, channelid, userid](const dpp::confirmation_callback_t& callback) -> void {
					   bot.start_timer([&bot, callback, userid, channelid](dpp::timer timer) {
						   if (callback.is_error() || vc_statuses[userid] != channelid) {
							   bot.channel_delete(channelid, [&bot](const dpp::confirmation_callback_t& callback) -> void {
								   if (callback.is_error()) {
									   bot.log(dpp::ll_error, callback.get_error().message);
								   }
							   });
						   }
						   bot.stop_timer(timer);
					   }, 5);
				   });
				   if (!ntif_chnls[guildid].guildid.empty()) {
					   std::string description = "A new temporary channel has been created ";
					   description += (!newchannel.parent_id.empty() ?
							   "in the <#" +
							   std::to_string(newchannel.parent_id) + "> category"
							   : "outside the categories");
					   description += ". Join the channel, **" + newchannel.name + "** (<#" +
									  std::to_string(newchannel.id) + ">)!";
					   dpp::embed temp_vc_create_embed = dpp::embed().
							   set_color(dpp::colors::greenish_blue).
							   set_description(description);
					   bot.message_create(dpp::message(ntif_chnls[guildid].channelid, temp_vc_create_embed));

				   }
				   if (callback.is_error()) {
					   std::cout << callback.http_info.body << '\n';
				   }
			   });
			}
		}
		channelid = vc_statuses[userid];
		bool is_temp = !temp_vcs[channelid].channelid.empty();
		if (is_temp && dpp::find_channel(channelid)->get_voice_members().empty()) {
			dpp::channel* newchannel = dpp::find_channel(channelid);
			log(fmt::format("{0} joined a JTC. Guild ID: {1}, channel ID: {2}, channel name: `{3}`, notification channel ID: {4}",
							user.format_username(), guildid, channelid,
							dpp::find_channel(channelid)->name, ntif_chnls[guildid].channelid));
			bot.channel_delete(channelid, [](const dpp::confirmation_callback_t& callback) -> void {
				if (callback.is_error()) {
					std::cout << callback.http_info.body << std::endl;
				}
			});
			dpp::snowflake ntif_channelid = ntif_chnls[guildid].channelid;
			if (!ntif_channelid.empty()) {
				std::string description = "A temporary channel **" + newchannel->name + "**";
				if (!newchannel->parent_id.empty()) {
					dpp::channel category = *dpp::find_channel(newchannel->parent_id);
					description += " in the **" + category.name + "** category";
				}
				description += " is deleted.";
				dpp::embed temp_vc_delete_message = dpp::embed().
						set_color(dpp::colors::blood_night).
						set_description(description);
				bot.message_create(dpp::message(ntif_channelid, temp_vc_delete_message));
			}
		}
		vc_statuses[userid] = event.state.channel_id;
		if (vc_statuses[userid].empty()) {
			vc_statuses[userid] = 1;
		}
	});

	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) -> dpp::task <void> {
		dpp::guild guild;
		dpp::command_interaction cmd = event.command.get_command_interaction();
		if (event.command.get_command_name() == "help") {
			event.reply(dpp::message(event.command.channel_id, slash::help_embed).set_flags(dpp::m_ephemeral));
			co_return;
		}
		if (event.command.get_command_name() == "set") {
			if (cmd.options[0].name == "default") {
				co_await slash::set::default_values(bot, event);
			}
			else {
				slash::set::current(bot, event);
			}
		}
		if (event.command.get_command_name() == "setup") {
			co_await slash::setup(bot, event);
		}
	});

	signal(SIGINT, [](int code) -> void {
		std::cout << std::endl << "Ну, все, я пішов спати, бувай, добраніч." << std::endl;
		system("killall guidingLight");
	});

	bot.start(bot_return);
	return 0;
}

#pragma clang diagnostic pop
