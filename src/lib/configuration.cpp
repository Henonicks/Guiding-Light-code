#include <configuration.h>

using json = nlohmann::json;

std::string_view logs_directory = "../logging/";
std::string BOT_TOKEN, logs_suffix;
dpp::snowflake bot_dm_logs, my_id, TOPGG_WEBHOOK_CHANNEL_ID, MY_GUILD_ID;
std::ofstream my_logs, guild_logs, other_logs;
dpp::start_type bot_return = dpp::st_wait;
bool is_dev = false;
int delay = 5;

void configuration::configure_bot(const bool& is_dev) {
    json config;
    std::ifstream config_file_stream("../Guiding_Light_Config/config.json");
    config_file_stream >> config;

    bot_dm_logs = config["BOT_DM_LOGS_ID"];
    my_id = config["MY_ID"];
	MY_GUILD_ID = config["MY_GUILD_ID"];
	TOPGG_WEBHOOK_CHANNEL_ID = config["TOPGG_WEBHOOK_CHANNEL_ID"];

    BOT_TOKEN = (is_dev ? config["BOT_TOKEN_DEV"] : config["BOT_TOKEN"]);

	logs_suffix = (is_dev ? "dev" : "release");

    my_logs.open(fmt::format("{0}{1}/my_logs.log", logs_directory, logs_suffix));
    guild_logs.open(fmt::format("{0}{1}/guild_logs.log", logs_directory, logs_suffix));
    other_logs.open(fmt::format("{0}{1}/other_logs.log", logs_directory, logs_suffix));

	file::temp_vc_notifications = fmt::format("../src/{}/temp_vc_notifications.txt", logs_suffix);
	file::jtc_vcs = fmt::format("../src/{}/jtc_vcs.txt", logs_suffix);
	file::temp = fmt::format("../src/{}/temp.txt", logs_suffix);
	file::jtc_default_values = fmt::format("../src/{}/jtc_default_values.txt", logs_suffix);
	file::no_temp_ping = fmt::format("../src/{}/no_temp_ping.txt", logs_suffix);
	file::topgg_guild_votes_amount = fmt::format("../src/{}/topgg_guild_votes_amount.txt", logs_suffix);
	file::topgg_guild_choices = fmt::format("../src/{}/topgg_guild_choices.txt", logs_suffix);
	file::no_noguild_reminder = fmt::format("../src/{}/no_noguild_reminder.txt", logs_suffix);
	file::topgg_notifications = fmt::format("../src/{}/topgg_notifications.txt", logs_suffix);
}

void configuration::pray(dpp::cluster& bot) { // I'll pray that when this function starts executing we have all the cache because Discord doesn't let me know whether all the cache I've received at a certain point is everything or there's more and there's no better way to do this I promise
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
			notification_chnl current = get_ntf_chnl(line);
			ntif_chnls[current.guildid] = channel->id;
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

	std::ifstream topgg_guild_choices_file;
	topgg_guild_choices_file.open(file::topgg_guild_choices);

	while (std::getline(topgg_guild_choices_file, line)) {
		topgg::guild_choice gc = topgg::get_guild_choice(line);
		dpp::user* user = dpp::find_user(gc.user_id);
		if (user != nullptr) {
			topgg::guild_choices[user->id] = gc.guild_id;
		}
		else {
			file::delete_line_once(line, file::topgg_guild_choices);
		}
	}

	topgg_guild_choices_file.close();

	std::ifstream topgg_guild_votes_amount_file;
	topgg_guild_votes_amount_file.open(file::topgg_guild_votes_amount);

	while (std::getline(topgg_guild_votes_amount_file, line)) {
		topgg::guild_votes_amount gva = topgg::get_guild_votes_amount(line);
		dpp::guild* guild = dpp::find_guild(gva.guild_id);
		if (guild != nullptr) {
			topgg::guild_list[guild->id] = gva.votes;
		}
		else {
			file::delete_line_once(line, file::topgg_guild_votes_amount);
		}
	}

	topgg_guild_choices_file.close();

	std::ifstream no_noguild_reminder_file;
	no_noguild_reminder_file.open(file::no_noguild_reminder);

	while (std::getline(no_noguild_reminder_file, line)) {
		dpp::user* user = dpp::find_user(line);
		if (user != nullptr) {
			topgg::noguild_reminders[user->id] = true;
		}
		else {
			file::delete_line_once(line, file::no_noguild_reminder);
		}
	}

	no_noguild_reminder_file.close();

	std::ifstream topgg_notifications_file;
	topgg_notifications_file.open(file::topgg_notifications);

	while (std::getline(topgg_notifications_file, line)) {
		notification_chnl ntif_chnl = get_ntf_chnl(line);
		dpp::channel* channel = dpp::find_channel(ntif_chnl.channelid);
		if (channel != nullptr) {
			topgg_ntif_chnls[channel->guild_id] = channel->id;
		}
		else {
			file::delete_line_once(line, file::topgg_notifications);
		}
	}

	topgg_notifications_file.close();

	slash::enabled = true;

	std::cout << "Amen.\n";
}

void configuration::write_down_slashcommands(dpp::cluster& bot) {
	bot.global_commands_get([&bot](const dpp::confirmation_callback_t& callback) -> void {
		auto map = callback.get <dpp::slashcommand_map>();
		for (const auto& x : map) {
			slash::created_slashcommands[x.second.name] = x.second;
		}
		slash::help_embed = dpp::embed().
			set_color(dpp::colors::sti_blue).
			set_author("Here is what I can do!\n", bot.me.get_url(), bot.me.get_avatar_url()).
			set_description("## /help\n"
							"I don\'t know, I guess you\'ve just issued this command?\n"
				"# JTC-related\n"

				"A Join-To-Create Voice Channel, or, as referred to, a JTC VC (or simply JTC), is a voice channel which you can join so that a temporary voice is created. "
				"When you do so, you\'re moved to a newly created temporary channel which you can edit with the `/set` command. "
				"A moderator can also edit the default settings of the temporary channels, created with a certain JTC by running the `/set` command with a sub-command of `default`. "
				"When everyone leaves a temporary channel, it gets automatically deleted, hence the \"temporary\" in the name. Then there are notification channels. "
				"Every time a temporary channel is created/deleted, a message about this is sent in such channel. Notification channels are text-only; "
				"if the guild they\'re set up in has community enabled, the channel will be an announcement channel, otherwise it\'s a normal text channel with sending messages disabled for the `@everyone` role.\n"

				"## /setup\n"

				"Create a JTC VC or a notification channel. If the limit wasn't hit.\n"

				"## /set\n"

				"This command has two types of subcommands: set current and set default.\n"

				"__**set current**__ sets a current value (currently supports the channel name/limit/bitrate) to a user-defined one. Some limitations apply. "
				"You cannot change the values of a channel that doesn\'t belong to you (was requested by someone else)\n"

				"__**set default**__ sets a default value (currently supports the channel name/limit/bitrate) to a user-defined one. Some limitations apply. "
				"You cannot use this if you don\'t have the permissions to edit a channel.\n"

				"# top.gg-related\n"

				"Voting for the bot is done on the top.gg website. Your vote counts as two if you vote on a weekend and therefore you get more points. "
				"You can only get points from voting if you\'ve selected a guild with `/guild set`. Voting for the bot while a guild `X` is selected will be referred to as \"Voting in favour of the guild `X`\". "
				"Voting in favour of a guild will get it points which add up from all of its members. Points are not subtracted when a member leaves, not even if they've voted. "
				"When a guild reaches a certain level, a new JTC is unlocked. There can be up to 10 JTCs while on the 1st level there\'s only 1.\n"

				"## /vote\n"

				"Issuing this command will show the link to the page where you can vote for me. "
				"Voting in favour of a certain guild a certain amount of times lets there be more JTCs and therefore more different default settings at once.\n"

				"## /guild\n"

				"This command\'s purpose is to set/get the guild you want to vote in favour of, "
				"`set` if you know which guild you want to vote in favour of and `get` if you forgot.\n"

				"__**set**__ sets the guild you\'re issuing the command in. This means that if you issue this sub-command in the guild `X`, "
				"your next vote will be made in favour of this guild unless you change the guild to `Y`. "
				"Then the vote will be made in favour of `Y`.\n"

				"__**get**__ gets the guild you\'re currently voting in favour of. "
				"If you\'ve issued the `set` sub-command in the gulid `X`, that\'ll be the guild returned.\n"

				"## /blocklist\n"

				"A blocklist is a list of users who aren\'t allowed to join your voice channel.\n"

				"__**add**__ adds a user to the blocklist of the channel if a user is provided as the parameter.\n"

				"__**remove**__ removes a user from the blocklist of the channel if a user is provided as the parameter.\n"

				"__**status**__ tells you whether the requested user is in the blocklist or not.\n"

				"\n"

				"__***TIP:***__ when setting the default name for a JTC, you can put \"{username}\" (without the quotes) so it can be replaced with the username of the person requesting a channel. For example, if the default name is \"VC for {username}\" and a person with the username \"henonicks\" requests a channel, the name of the temporary VC will be set to \"VC for henonicks\".\n"

				"\n"

				"__***NOTE:***__ If I shut down while you\'re in a temporary VC, the channel will never be auto-deleted since temporary VCs are not stored in the files. This is to be handled by a moderator. Deleting *all* of the temporary VCs out there would take a lot of API requests which could get the bot rate-limited which is, in Layman\'s terms, terrible.\n"
				"If you vote but don\'t have a guild set to vote in favour of, the point is wasted. The first time you do that, you get a direct message from the bot if your DMs are open."
			)
			.set_footer(dpp::embed_footer()
				.set_text("You can always ask the creator a question in direct messages or on the support server!")
				.set_icon(bot.me.get_avatar_url())
			);
	});
}
