#include "guiding_light/cfg.hpp"

#include <dpp/unicode_emoji.h>

#include "guiding_light/responses.hpp"

void cfg::check_sqlite3() {
	if (!command_exists("sqlite3")) {
		std::cerr << fmt::format("{} The sqlite3 executable hasn't been found. Please install sqlite3.\n", color::rize("ERROR:", "Red"));
		log("ERROR: The sqlite3 executable hasn't been found. Please install sqlite3.");
		std::exit(1);
	}
}

void cfg::read_config() {
	config.open("../Guiding_Light_Config/config.hfg");
	responses.open("../additional/responses.hfg");
	slashcommands.open("../additional/slashcommands.hfg");

	BOT_DM_LOGS = config["BOT_DM_LOGS_ID"];
	MY_ID = config["MY_ID"];
	MY_GUILD_ID = config["MY_GUILD_ID"];
	MY_PRIVATE_GUILD_ID = config["MY_PRIVATE_GUILD_ID"];
	TOPGG_WEBHOOK_CHANNEL_ID = config["TOPGG_WEBHOOK_CHANNEL_ID"];
	TICKETS_GUILD_ID = config["TICKETS_GUILD_ID"];
	LOGS_CHANNEL_ID = config["LOGS_CHANNEL_ID"];

	BOT_TOKEN = config["BOT_TOKEN"].get <std::string>();
	BOT_TOKEN_DEV = config["BOT_TOKEN_DEV"].get <std::string>();

	MODE_NAME = IS_DEV ? "dev" : "release";

	logs_directory = fmt::format("../logging/{}", IS_CLI ? "cli" : "bot");

	PING_RESPONSES = responses["PING_RESPONSES"].get <henifig::value_array>();
}

void cfg::init_logs() {
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

void cfg::pray() { // I'll pray that when this function starts executing we have all the cache because Discord doesn't let me know whether all the cache I've received at a certain point is everything or there's more and there's no better way to do this I promise
	db::sql << "SELECT * FROM jtc_vcs;" + db::line_comment("pray::jtc_vcs") >> [](const db::BIGINT channel_id, const db::BIGINT guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			jtc_vcs[channel_id] = guild_id;
			++jtc_vc_amount[guild_id];
		}
		else {
			db::sql << "DELETE FROM jtc_vcs WHERE channel_id=?;" << channel_id;
			error_log(fmt::format("Couldn't find the JTC VC {0} in the guild {1}. Deleting.", channel_id, guild_id));
		}
	};
	db::sql << "SELECT * FROM temp_vc_notifications;" + db::line_comment("pray::temp_vc_notifications") >> [](const db::BIGINT channel_id, const db::BIGINT guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			temp_vc_notifications[guild_id] = channel_id;
		}
		else {
			db::sql << "DELETE FROM temp_vc_notifications WHERE guild_id=?;" << guild_id;
			error_log(fmt::format("Couldn't find the temp notification VC {0} in the guild {1}. Deleting.", channel_id, guild_id));
		}
	};

	db::sql << "SELECT * FROM jtc_default_values;" + db::line_comment("pray::jtc_default_values") >> [](const db::BIGINT channel_id, const std::string& name, const db::TINYINT limit, const db::MEDIUMINT bitrate) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			jtc_default_values[channel_id] = {channel_id, name, cast <int8_t>(limit), cast <int16_t>(bitrate)};
		}
		else {
			db::sql << "DELETE FROM jtc_default_values WHERE channel_id=?;" << channel_id;
			error_log(fmt::format("Couldn't find the default values for the JTC VC {0}, with the name `{1}`, limit {2}, bitrate {3}. Deleting.", channel_id, name, limit, bitrate));
		}
	};

	db::sql << "SELECT * FROM no_temp_ping;" + db::line_comment("pray::no_temp_ping") >> [](const db::BIGINT user_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			no_temp_ping[user_id] = true;
		}
		else {
			db::sql << "DELETE FROM no_temp_ping WHERE user_id=?;" << user_id;
			error_log(fmt::format("While searching for temp ping blockers: couldn't find the user {}. Deleting.", user_id));
		}
	};
	
	db::sql << "SELECT * FROM topgg_guild_choices;" + db::line_comment("pray::jtc_vcs") >> [](const db::BIGINT user_id, const db::BIGINT guild_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			topgg::guild_choices[user_id] = guild_id;
		}
		else {
			db::sql << "DELETE FROM topgg_guild_choices WHERE user_id=?;" << user_id;
			error_log(fmt::format("Couldn't find the user {0} to get the top.gg choice {1} from. Deleting.", user_id, guild_id));
		}
	};
	
	db::sql << "SELECT * FROM topgg_guild_votes_amount;" + db::line_comment("pray::guild_votes_amount") >> [](const db::BIGINT guild_id, const int votes) {
		const dpp::guild* guild = dpp::find_guild(guild_id);
		if (guild != nullptr) {
			topgg::guild_votes_amount[guild_id] = votes;
		}
		else {
			db::sql << "DELETE FROM topgg_guild_votes_amount WHERE guild_id=?;" << guild_id;
			error_log(fmt::format("Couldn't find the guild {0} to get {1} votes from. Deleting.", guild_id, votes));
		}
	};
	
	db::sql << "SELECT * FROM no_noguild_reminder;" + db::line_comment("pray::no_noguild_reminder") >> [](const db::BIGINT user_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			topgg::no_noguild_reminder[user_id] = true;
		}
		else {
			db::sql << "DELETE FROM no_noguild_reminder WHERE user_id=?;" << user_id;
			error_log(fmt::format("While searching for no guild reminder blockers: couldn't find the user {}. Deleting.", user_id));
		}
	};
	
	db::sql << "SELECT * FROM topgg_notifications;" + db::line_comment("pray::topgg_notifications") >> [](const db::BIGINT channel_id, const db::BIGINT guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			topgg_notifications[guild_id] = channel_id;
		}
		else {
			db::sql << "DELETE FROM topgg_notifications WHERE guild_id=?;" << guild_id;
			error_log(fmt::format("Couldn't find the top.gg notifications channel {0} in the guild {1}. Deleting.", channel_id, guild_id));
		}
	};
	
	db::sql << "SELECT * FROM tickets;" + db::line_comment("pray::tickets") >> [](const db::BIGINT user_id, const db::BIGINT channel_id) {
		const dpp::user* user = dpp::find_user(user_id);
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (user != nullptr && channel != nullptr) {
			tickets[user_id] = channel_id;
			ck_tickets[channel_id] = user_id;
		}
		else {
			db::sql << "DELETE FROM tickets WHERE user_id=?;" << user_id;
			error_log(fmt::format("Couldn't find the user {0} with the ticket channel {1}. Deleting.", user_id, channel_id));
		}
	};
	
	db::sql << "SELECT * FROM temp_vcs;" + db::line_comment("pray::temp_vcs") >> [](const db::BIGINT channel_id, const db::BIGINT guild_id, const db::BIGINT& creator_id, const db::BIGINT parent_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			++temp_vc_amount[guild_id];
			temp_vcs[channel_id] = {channel_id, guild_id, creator_id, parent_id};
			for (const dpp::permission_overwrite& x : channel->permission_overwrites) {
				if (x.deny.can(dpp::p_view_channel)) {
					banned[channel->id].insert(x.id);
				}
			}
		}
		else {
			db::sql << "DELETE FROM temp_vcs WHERE channel_id=?;" << channel_id;
			error_log(fmt::format("Couldn't find the temp VC {0} in the guild {1} whose creator is {2} with the parent being {3}. Deleting.", channel_id, guild_id, creator_id, parent_id));
		}
	};

	db::sql << "SELECT * FROM channel_name_edit_timers;" + db::line_comment("pray::name_edit_timers") >> [](const db::BIGINT channel_id, const time_t timer) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			const auto current_time = cast <time_t>(dpp::utility::time_f());
			if (current_time < timer) {
				channel_edits[cet_name][channel_id] = true;
				channel_edit_timers[cet_name][channel_id] = timer;
				remove_channel_edit(channel_id, cet_name, timer - current_time);
			}
			else {
				db::sql << "DELETE FROM channel_name_edit_timers WHERE channel_id=?;" << channel_id;
			}
		}
		else {
			db::sql << "DELETE FROM channel_name_edit_timers WHERE channel_id=?;" << channel_id;
			error_log(fmt::format("Couldn't find the temp VC {0} which is on cooldown from being renamed until {1}. Deleting.", channel_id, timer));
		}
	};

	slash::enabled = true;
}

void cfg::write_down_slashcommands() {
	bot->global_commands_get([](const dpp::confirmation_callback_t& callback) -> void {
		const auto& map = callback.get <dpp::slashcommand_map>();
		for (const auto& cmd : map | std::views::values) {
			slash::global_created[cmd.name] = cmd;
		}
		bot->guild_commands_get(MY_PRIVATE_GUILD_ID, [](const dpp::confirmation_callback_t& callback) -> void {
			if (error_callback(callback)) {
				return;
			}
			const auto& map = callback.get <dpp::slashcommand_map>();
			for (const auto& cmd : map | std::views::values) {
				slash::guild_created[cmd.name] = cmd;
			}
		});
	});
}

dpp::message cfg::help_message(const std::string_view lang, const uint8_t page) {
	const henifig::value_map& help_components = responses["LOCALISATION"]["HELP_EMBEDS"];
	const henifig::value_t& rp = response(DESCRIPTIONS, lang, help_components);
	if (!rp.is <henifig::array_t>()) {
		return dpp::message(rp.get <std::string>()).set_flags(dpp::m_ephemeral);
	}
	const std::vector <std::string> descriptions = get_arr <std::string>(rp);
	const henifig::value_array& help_cmd_mentions = responses["HELP_COMMAND_MENTIONS"];
	dpp::message res;
	res.add_embed(
		dpp::embed()
		.set_color(dpp::colors::sti_blue)
		.set_description(format_if_filled(descriptions[page], {slash::get_mention(get_arr <std::string>(help_cmd_mentions[page]))}))
	);
	res.embeds[0].set_author(response(AUTHOR, lang, help_components), bot->me.get_url(), bot->me.get_avatar_url());
	res.embeds[0].set_footer(dpp::embed_footer()
		.set_text(response(FOOTER, lang, help_components))
		.set_icon(bot->me.get_avatar_url())
	);
	dpp::component buttons;
	buttons.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(fmt::format("help{}", page - 1))
		.set_label(dpp::unicode_emoji::arrow_left)
	);
	if (page == 0) {
		buttons.components[0].set_disabled(true);
	}
	buttons.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(fmt::format("help{}", page + 1))
		.set_label(dpp::unicode_emoji::arrow_right)
	);
	if (page == descriptions.size() - 1) {
		buttons.components[1].set_disabled(true);
	}
	return res.add_component(buttons).set_flags(dpp::m_ephemeral);
}

std::string format_if_filled(const std::string_view base, const std::vector <std::string>& values) {
	if (values.empty()) {
		return base.data();
	}
	return fmt::vformat(base, vec_to_fmt(values));
}
