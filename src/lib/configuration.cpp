#include <configuration.h>

using json = nlohmann::json;

void configuration::read_config() {
	json config;
	std::ifstream config_file_stream("../Guiding_Light_Config/config.json");
	config_file_stream >> config;

	BOT_DM_LOGS = config["BOT_DM_LOGS_ID"];
	MY_ID = config["MY_ID"];
	MY_GUILD_ID = config["MY_GUILD_ID"];
	MY_PRIVATE_GUILD_ID = config["MY_PRIVATE_GUILD_ID"];
	TOPGG_WEBHOOK_CHANNEL_ID = config["TOPGG_WEBHOOK_CHANNEL_ID"];
	TICKETS_GUILD_ID = config["TICKETS_GUILD_ID"];

	BOT_TOKEN = config["BOT_TOKEN"];
	BOT_TOKEN_DEV = config["BOT_TOKEN_DEV"];

	MODE_NAME = IS_DEV ? "dev" : "release";

	logs_directory = fmt::format("../logging/{}", IS_CLI ? "cli" : "bot");
}

void configuration::init_logs() {
	const std::filesystem::path path_release{fmt::format("{}/release", logs_directory)};
	const std::filesystem::path path_dev{fmt::format("{}/dev", logs_directory)};
	if (!std::filesystem::exists(path_release)) {
		std::filesystem::create_directories(fmt::format("{}/release", logs_directory));
	}
	if (!std::filesystem::exists(path_dev)) {
		std::filesystem::create_directories(fmt::format("{}/dev", logs_directory));
	}
	my_logs_release = std::ofstream(fmt::format("{}/release/my_logs.log", logs_directory));
	my_logs_dev = std::ofstream(fmt::format("{}/dev/my_logs.log", logs_directory));
	guild_logs_release = std::ofstream(fmt::format("{}/release/guild_logs.log", logs_directory));
	guild_logs_dev = std::ofstream(fmt::format("{}/dev/guild_logs.log", logs_directory));
	other_logs_release = std::ofstream(fmt::format("{}/release/other_logs.log", logs_directory));
	other_logs_dev = std::ofstream(fmt::format("{}/dev/other_logs.log", logs_directory));
	sql_logs_release = std::ofstream(fmt::format("{}/release/sql_logs.log", logs_directory));
	sql_logs_dev = std::ofstream(fmt::format("{}/dev/sql_logs.log", logs_directory));
}

void configuration::pray() { // I'll pray that when this function starts executing we have all the cache because Discord doesn't let me know whether all the cache I've received at a certain point is everything or there's more and there's no better way to do this I promise
	db::sql << "SELECT * FROM jtc_vcs;" >> [](const db::BIGINT& channel_id, const db::BIGINT& guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			jtc_vcs[channel_id] = guild_id;
			++jtc_vc_amount[guild_id];
		}
		else {
			db::sql << "DELETE FROM jtc_vcs WHERE channel_id=?;" << channel_id;
		}
	};
	db::sql << "SELECT * FROM temp_vc_notifications;" >> [](const db::BIGINT& channel_id, const db::BIGINT& guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			temp_vc_notifications[guild_id] = channel_id;
		}
		else {
			db::sql << "DELETE FROM temp_vc_notifications WHERE guild_id=?;" << guild_id;
		}
	};

	db::sql << "SELECT * FROM jtc_default_values;" >> [](const db::BIGINT& channel_id, const std::string& name, const db::TINYINT& limit, const db::MEDIUMINT& bitrate) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			jtc_default_values[channel_id] = {channel_id, name, (int8_t)limit, (int16_t)bitrate};
		}
		else {
			db::sql << "DELETE FROM jtc_default_values WHERE channel_id=?;" << channel_id;
		}
	};

	db::sql << "SELECT * FROM no_temp_ping;" >> [](const db::BIGINT& user_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			no_temp_ping[user_id] = true;
		}
		else {
			db::sql << "DELETE FROM no_temp_ping WHERE user_id=?;" << user_id;
		}
	};
	
	db::sql << "SELECT * FROM topgg_guild_choices;" >> [](const db::BIGINT& user_id, const db::BIGINT& guild_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			topgg::guild_choices[user_id] = guild_id;
		}
		else {
			db::sql << "DELETE FROM topgg_guild_choices WHERE user_id=?;" << user_id;
		}
	};
	
	db::sql << "SELECT * FROM topgg_guild_votes_amount;" >> [](const db::BIGINT& guild_id, const int& votes) {
		const dpp::guild* guild = dpp::find_guild(guild_id);
		if (guild != nullptr) {
			topgg::guild_votes_amount[guild_id] = votes;
		}
		else {
			db::sql << "DELETE FROM topgg_guild_votes_amount WHERE guild_id=?;" << guild_id;
		}
	};
	
	db::sql << "SELECT * FROM no_noguild_reminder;" >> [](const db::BIGINT& user_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			topgg::no_noguild_reminder[user_id] = true;
		}
		else {
			db::sql << "DELETE FROM no_noguild_reminder WHERE user_id=?;" << user_id;
		}
	};
	
	db::sql << "SELECT * FROM topgg_notifications;" >> [](const db::BIGINT& channel_id, const db::BIGINT& guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			topgg_notifications[guild_id] = channel_id;
		}
		else {
			db::sql << "DELETE FROM topgg_notifications WHERE guild_id=?;" << guild_id;
		}
	};
	
	db::sql << "SELECT * FROM tickets;" >> [](const db::BIGINT user_id, const db::BIGINT channel_id) {
		const dpp::user* user = dpp::find_user(user_id);
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (user != nullptr && channel != nullptr) {
			tickets[user_id] = channel_id;
			ck_tickets[channel_id] = user_id;
		}
		else {
			db::sql << "DELETE FROM tickets WHERE user_id=?;" << user_id;
		}
	};
	
	db::sql << "SELECT * FROM temp_vcs;" >> [](const db::BIGINT& channel_id, const db::BIGINT& guild_id, const db::BIGINT& creator_id, const db::BIGINT& parent_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			++temp_vc_amount[guild_id];
			temp_vcs[channel_id] = {channel_id, guild_id, creator_id, parent_id};
			for (const auto& x : channel->permission_overwrites) {
				if (x.deny.can(dpp::p_view_channel)) {
					banned[channel->id].insert(x.id);
				}
			}
		}
		else {
			db::sql << "DELETE FROM temp_vcs WHERE channel_id=?;" << channel_id;
		}
	};
	
	slash::enabled = true;
}

void configuration::write_down_slashcommands() {
	bot->global_commands_get([](const dpp::confirmation_callback_t& callback) -> void {
		const auto& map = callback.get <dpp::slashcommand_map>();
		for (const auto& x : map) {
			slash::global_created[x.second.name] = x.second;
		}
		bot->guild_commands_get(MY_PRIVATE_GUILD_ID, [](const dpp::confirmation_callback_t& callback) -> void {
			const auto& map = callback.get <dpp::slashcommand_map>();
			for (const auto& x : map) {
				slash::guild_created[x.second.name] = x.second;
			}
		});
		slash::help_embed_1 = dpp::embed().
			set_color(dpp::colors::sti_blue).
			set_author("Here is what I can do!\n", bot->me.get_url(), bot->me.get_avatar_url()).
			set_description("## /help\n"
							"I guess you\'ve just issued this command!\n"
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

				"### Note: this command is in beta. If something goes wrong, please report it by creating a ticket.\n"

				"A blocklist is a list of users who aren\'t allowed to join your voice channel.\n"

				"__**add**__ adds a user to the blocklist of the channel if a user is provided as the parameter.\n"

				"__**remove**__ removes a user from the blocklist of the channel if a user is provided as the parameter.\n"

				"__**status**__ tells you whether the requested user is in the blocklist or not.\n"

				"# Ticket-related\n"

				"Whenever you find something that works incorrectly (at least, in your opinion) you can contact the creator of the bot by DMing it. "
				"But first, you need to open a ticket.\n"

				"## /ticket\n"

				"### Note: this command is in beta. If something goes wrong... well, this is the tickets command, so you're on your own. Unless you're in the Discord server.\n"

				"__**create**__ allows the creator of the bot to talk to you through the bot's DMs.\n"
				"__**close**__ closes a ticket, ending any conversation between the creator of the bot and you."
			);
		slash::help_embed_2 = dpp::embed().
			set_color(dpp::colors::sti_blue).
			set_description(
				"__***TIP:***__ when setting the default name for a JTC, you can put \"{username}\" (without the quotes) so it can be replaced with the username of the person requesting a channel. For example, if the default name is \"VC for {username}\" and a person with the username \"henonicks\" requests a channel, the name of the temporary VC will be set to \"VC for henonicks\".\n"

				"\n"

				"__***NOTE:***__ When the bot reboots, it won\'t know that you (as well as anyone else) are/is in a channel. A temporary VC needs to be \"touched\" to be auto-deleted. So to delete your previously owned channel you need to re-join it and then leave it. Although if you decide to keep it, you can!\n"
				"If you vote but don\'t have a guild set to vote in favour of, the point is wasted. The first time you do that, you get a direct message from the bot if your DMs are open."
			).set_footer(dpp::embed_footer()
				.set_text("You can always ask the creator of the bot a question in direct messages or on the support server!")
				.set_icon(bot->me.get_avatar_url())
			);
	});
}
