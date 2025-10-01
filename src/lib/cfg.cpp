#include "guiding_light/cfg.hpp"

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

	BOT_DM_LOGS = config["BOT_DM_LOGS_ID"];
	MY_ID = config["MY_ID"];
	MY_GUILD_ID = config["MY_GUILD_ID"];
	MY_PRIVATE_GUILD_ID = config["MY_PRIVATE_GUILD_ID"];
	TOPGG_WEBHOOK_CHANNEL_ID = config["TOPGG_WEBHOOK_CHANNEL_ID"];
	TICKETS_GUILD_ID = config["TICKETS_GUILD_ID"];

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
	db::sql << "SELECT * FROM jtc_vcs;" + db::line_comment("pray::jtc_vcs") >> [](const db::BIGINT& channel_id, const db::BIGINT& guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			jtc_vcs[channel_id] = guild_id;
			++jtc_vc_amount[guild_id];
		}
		else {
			db::sql << "DELETE FROM jtc_vcs WHERE channel_id=?;" << channel_id;
		}
	};
	db::sql << "SELECT * FROM temp_vc_notifications;" + db::line_comment("pray::temp_vc_notifications") >> [](const db::BIGINT& channel_id, const db::BIGINT& guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			temp_vc_notifications[guild_id] = channel_id;
		}
		else {
			db::sql << "DELETE FROM temp_vc_notifications WHERE guild_id=?;" << guild_id;
		}
	};

	db::sql << "SELECT * FROM jtc_default_values;" + db::line_comment("pray::jtc_default_values") >> [](const db::BIGINT& channel_id, const std::string& name, const db::TINYINT& limit, const db::MEDIUMINT& bitrate) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			jtc_default_values[channel_id] = {channel_id, name, (int8_t)limit, (int16_t)bitrate};
		}
		else {
			db::sql << "DELETE FROM jtc_default_values WHERE channel_id=?;" << channel_id;
		}
	};

	db::sql << "SELECT * FROM no_temp_ping;" + db::line_comment("pray::no_temp_ping") >> [](const db::BIGINT& user_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			no_temp_ping[user_id] = true;
		}
		else {
			db::sql << "DELETE FROM no_temp_ping WHERE user_id=?;" << user_id;
		}
	};
	
	db::sql << "SELECT * FROM topgg_guild_choices;" + db::line_comment("pray::jtc_vcs") >> [](const db::BIGINT& user_id, const db::BIGINT& guild_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			topgg::guild_choices[user_id] = guild_id;
		}
		else {
			db::sql << "DELETE FROM topgg_guild_choices WHERE user_id=?;" << user_id;
		}
	};
	
	db::sql << "SELECT * FROM topgg_guild_votes_amount;" + db::line_comment("pray::guild_votes_amount") >> [](const db::BIGINT& guild_id, const int& votes) {
		const dpp::guild* guild = dpp::find_guild(guild_id);
		if (guild != nullptr) {
			topgg::guild_votes_amount[guild_id] = votes;
		}
		else {
			db::sql << "DELETE FROM topgg_guild_votes_amount WHERE guild_id=?;" << guild_id;
		}
	};
	
	db::sql << "SELECT * FROM no_noguild_reminder;" + db::line_comment("pray::no_noguild_reminder") >> [](const db::BIGINT& user_id) {
		const dpp::user* user = dpp::find_user(user_id);
		if (user != nullptr) {
			topgg::no_noguild_reminder[user_id] = true;
		}
		else {
			db::sql << "DELETE FROM no_noguild_reminder WHERE user_id=?;" << user_id;
		}
	};
	
	db::sql << "SELECT * FROM topgg_notifications;" + db::line_comment("pray::topgg_notifications") >> [](const db::BIGINT& channel_id, const db::BIGINT& guild_id) {
		const dpp::channel* channel = dpp::find_channel(channel_id);
		if (channel != nullptr) {
			topgg_notifications[guild_id] = channel_id;
		}
		else {
			db::sql << "DELETE FROM topgg_notifications WHERE guild_id=?;" << guild_id;
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
		}
	};
	
	db::sql << "SELECT * FROM temp_vcs;" + db::line_comment("pray::temp_vcs") >> [](const db::BIGINT& channel_id, const db::BIGINT& guild_id, const db::BIGINT& creator_id, const db::BIGINT& parent_id) {
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

void cfg::write_down_slashcommands() {
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

		const henifig::value_map& help_components = responses["HELP_EMBEDS"];
		const henifig::value_array& descriptions = help_components.at("DESCRIPTIONS");
		slash::help_embeds.resize(descriptions.size());
		for (size_t i = 0; i < descriptions.size(); i++) {
			fmt::dynamic_format_arg_store <fmt::format_context> desc_args;
			const henifig::value_array& desc = descriptions[i];
			const std::string& desc_str = desc[0];
			for (size_t j = 1; j < desc.size(); j++) {
				desc_args.push_back(slash::get_mention(desc[j].get <std::string>()));
			}
			slash::help_embeds[i] = dpp::embed()
				.set_color(dpp::colors::sti_blue)
				.set_description(
					desc.size() > 1 ? fmt::vformat(desc_str, desc_args) : desc_str
				);
		}
		slash::help_embeds[0].set_author(help_components.at("AUTHOR").get <std::string>(), bot->me.get_url(), bot->me.get_avatar_url());
		slash::help_embeds.rbegin()->set_footer(dpp::embed_footer()
			.set_text(help_components.at("FOOTER").get <std::string>())
			.set_icon(bot->me.get_avatar_url())
		);
	});
}
