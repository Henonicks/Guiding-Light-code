#include <configuration.h>

using json = nlohmann::json;

std::string BOT_TOKEN;
dpp::snowflake bot_dm_logs, my_id;
std::ofstream my_logs, guild_logs, other_logs;

void configuration::configure_bot(const bool& is_dev) {
    json config;
    std::ifstream config_file_stream("../config.json");
    config_file_stream >> config;

    bot_dm_logs = config["BOT_DM_LOGS_ID"];
    my_id = config["MY_ID"];

    BOT_TOKEN = (is_dev ? config["BOT_TOKEN_DEV"] : config["BOT_TOKEN"]);

	std::string_view logs_suffix = (is_dev ? "dev" : "release");

    my_logs.open(fmt::format("../logging/{}/my_logs.log", logs_suffix));
    guild_logs.open(fmt::format("../logging/{}/guild_logs.log", logs_suffix));
    other_logs.open(fmt::format("../logging/{}/other_logs.log", logs_suffix));

	file::temp_vc_notifications = fmt::format("../src/{}/temp_vc_notifications.txt", logs_suffix);
	file::jtc_vcs = fmt::format("../src/{}/jtc_vcs.txt", logs_suffix);
	file::temp = fmt::format("../src/{}/temp.txt", logs_suffix);
	file::jtc_default_values = fmt::format("../src/{}/jtc_default_values.txt", logs_suffix);
	file::no_temp_ping = fmt::format("../src/{}/no_temp_ping.txt", logs_suffix);
}

void configuration::configure_channels() {
    std::string line;

    std::ifstream last_jtc_vcs;
    last_jtc_vcs.open(file::jtc_vcs);
    while (std::getline(last_jtc_vcs, line)) {
		dpp::channel* channel = dpp::find_channel(get_jtc_vc(line).channelid);
		if (channel != nullptr) {
			jtc_vc current = get_jtc_vc(line);
			jtc_vcs[current.channelid] = current;
			jtc_channels_map[current.channelid] = *channel;
		}
		else {
			file::delete_line_once(line, file::jtc_vcs);
		}
    }

    last_jtc_vcs.close();
    std::ifstream last_temp_vc_notifications;
    last_temp_vc_notifications.open(file::temp_vc_notifications);

    while (std::getline(last_temp_vc_notifications, line)) {
		dpp::channel* channel = dpp::find_channel(get_ntf_chnl(line).channelid);
		if (channel != nullptr) {
			vc_notification_chnl current = get_ntf_chnl(line);
			ntif_chnls[current.guildid] = current;
		}
        else {
			file::delete_line_once(line, file::temp_vc_notifications);
        }
    }

	last_temp_vc_notifications.close();

    std::ifstream jtc_default_values_file;
    jtc_default_values_file.open(file::jtc_default_values);

    while (std::getline(jtc_default_values_file, line)) {
		dpp::channel* channel = dpp::find_channel(get_jtc_defs(line).channelid);
		if (channel != nullptr) {
			jtc_defaults current = get_jtc_defs(line);
			jtc_default_values[current.channelid] = current;
		}
        else {
			file::delete_line_once(line, file::jtc_default_values);
        }
    }

    jtc_default_values_file.close();

	std::ifstream no_temp_ping_file;
	no_temp_ping_file.open(file::no_temp_ping);

	while (std::getline(no_temp_ping_file, line)) {
		dpp::user* user = dpp::find_user((dpp::snowflake)line);
		if (user != nullptr) {
			no_temp_ping[user->id] = true;
		}
		else {
			file::delete_line_once(line, file::no_temp_ping);
		}
	}

	no_temp_ping_file.close();

}

void configuration::write_down_slashcommands(dpp::cluster& bot) {
	bot.global_commands_get([&bot](const dpp::confirmation_callback_t& callback) -> void {
		auto map = callback.get <dpp::slashcommand_map>();
		for (const auto& x : map) {
			for (const auto& y : x.second.options) {

				slash::created_slashcommands[x.second.name] = x.second;
			}
		}
		slash::help_embed = dpp::embed().
			set_color(dpp::colors::sti_blue).
			set_author("Here is what I can do!\n", bot.me.get_url(), bot.me.get_avatar_url()).
			set_description("# /help\n"
							"I don\'t know, I guess you\'ve just issued this command?\n"
				"# /setup\n"
				"Create a join-to-create (JTC) voice channel or a notification channel. As soon as you join a JTC, a temporary one is created, and you get moved to it, unless you\'ve disconnected. A notification channel is used to notify others when a temporary channel is created.\n"
				"# /set\n"
				"This command has two types of subcommands: set current and set default.\n"
				"__**set current**__ sets a current value (currently supports the channel name/limit/bitrate) to a user-defined one. Some limitations apply. You cannot change the values of a channel that doesn\'t belong to you (was requested by someone else)\n"
				"__**set default**__ sets a default value (currently supports the channel name/limit/bitrate) to a user-defined one. Some limitations apply. You cannot use this if you don\'t have the permissions to edit a channel.\n"
				"\n"
				"__***TIP:***__ you can put \"{username}\" (without the quotes) so it can be replaced with the username of the person requesting a channel. For example, if the default name is \"VC for {username}\" and a person with the username \"henonicks\" requests a channel, the name of the temporary VC will be set to \"VC for henonicks\".\n"
				"\n"
				"__***NOTE:***__ If I shut down while you\'re in a temporary VC, the channel will never be auto-deleted since temporary VCs are not stored in the files. This is to be handled by a moderator. Deleting *all* of the temporary VCs out there would take a lot of API requests which could get the bot rate-limited which is, in Layman\'s terms, terrible."
			)
			.set_footer(dpp::embed_footer()
				.set_text("You can always ask me a question!")
				.set_icon(bot.me.get_avatar_url())
			);
	});
}
