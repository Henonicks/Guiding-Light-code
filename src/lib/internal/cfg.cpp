#include "guiding_light/internal/cfg.hpp"

#ifdef GL_BOT_ENVIRONMENT

#include <dpp/unicode_emoji.h>

#include "dpptgg/topgg_poker.hpp"

#include "guiding_light/handlers/responses.hpp"
#include "guiding_light/handlers/temp_vc_handler.hpp"
#include "guiding_light/handlers/slash_funcs.hpp"
#include "guiding_light/resources/temp_ping_blocker.hpp"
#include "guiding_light/internal/config_values.hpp"
#include "guiding_light/database/database.hpp"
#include "guiding_light/handlers/topgg.hpp"

#include "guiding_light/helpers/format.tpp"

#include "http_server/topgg_listener.hpp"

dpp::emoji build_emoji(const henifig::value_map& config) {
	const std::string_view name = config.at("name");
	const auto id = config.contains("id") ? cast <dpp::snowflake>(config.at("id")) : dpp::snowflake();
	const bool is_animated = config.contains("animated") ? cast <bool>(config.at("animated")) : false;
	return {name, id, cast <uint8_t>(dpp::e_animated * is_animated)};
}

#endif

void cfg::read_config(const std::string_view path) {
	std::scoped_lock L(config_mutex, cfg_values_mutex);

	config.open(path);
#ifdef GL_BOT_ENVIRONMENT
	responses.open("../additional/responses.hfg");
	slashcommands.open("../additional/slashcommands.hfg");

	BOT_DM_LOGS = config["BOT_DM_LOGS_ID"].get <unsigned long long>();
	MY_ID = config["MY_ID"].get <unsigned long long>();
	MY_GUILD_ID = config["MY_GUILD_ID"].get <unsigned long long>();
	MY_PRIVATE_GUILD_ID = config["MY_PRIVATE_GUILD_ID"].get <unsigned long long>();
	TICKETS_GUILD_ID = config["TICKETS_GUILD_ID"].get <unsigned long long>();
	LOGS_CHANNEL_ID = config["LOGS_CHANNEL_ID"].get <unsigned long long>();

	PING_EMOJI = build_emoji(config["PING_EMOJI"]);
#endif
	try {
		TOPGG_WEBHOOK_CHANNEL_ID = config["TOPGG_WEBHOOK_CHANNEL_ID"].get <unsigned long long>();
#ifdef GL_BOT_ENVIRONMENT
		TOPGG_BOT_TOKEN = config["TOPGG_BOT_TOKEN"].get <std::string>();
		topgg::votes_leveling = cast <decltype(topgg::votes_leveling)>(config["TOPGG_VOTES_LEVELING"]);
#endif
	}
	catch (const henifig::retrieval_exception&) {}

#ifdef GL_USE_LOCAL_DB
	try
#endif
	{
		GL_ON_RAILS_API_URL = config["GL_ON_RAILS_API_URL"].get <std::string>();
		GL_ON_RAILS_API_URL_DEV = config["GL_ON_RAILS_API_URL_DEV"].get <std::string>();
		if (GL_ON_RAILS_API_URL.back() != '/') {
			GL_ON_RAILS_API_URL += '/';
		}
		if (GL_ON_RAILS_API_URL_DEV.back() != '/') {
			GL_ON_RAILS_API_URL_DEV += '/';
		}
		gor::v1::BASE_API_URL = GL_ON_RAILS_API_URL + "v1/";
		gor::v1::BASE_API_URL_DEV = GL_ON_RAILS_API_URL_DEV + "v1/";
		GL_ON_RAILS_TOKEN = config["GL_ON_RAILS_TOKEN"].get <std::string>();
	}
#ifdef GL_USE_LOCAL_DB
	catch (...) {}
#endif

#ifdef GL_BOT_ENVIRONMENT
	BOT_TOKEN = config["BOT_TOKEN"].get <std::string>();
	BOT_TOKEN_DEV = config["BOT_TOKEN_DEV"].get <std::string>();
#endif

	MODE_NAME = IS_DEV ? "dev" : "release";

	logs_directory = std::format("../logging/{}", IS_CLI ? "cli" : "bot");

#ifdef GL_BOT_ENVIRONMENT
	PING_RESPONSES = responses["PING_RESPONSES"].get <henifig::value_array>();
#endif
}

void cfg::init_logs() {
	std::lock_guard L(logfile_mutex);
	const std::filesystem::path path_release{std::format("{}/release", logs_directory)};
	const std::filesystem::path path_dev{std::format("{}/dev", logs_directory)};
	if (!std::filesystem::exists(path_release)) {
		std::filesystem::create_directories(std::format("{}/release", logs_directory));
	}
	if (!std::filesystem::exists(path_dev)) {
		std::filesystem::create_directories(std::format("{}/dev", logs_directory));
	}
	logfile_paths[&my_logs_release] = std::format("{}/release/my_logs.log", logs_directory);
	logfile_names[&my_logs_release] = "my_logs.log";
	logfile_paths[&my_logs_dev] = std::format("{}/dev/my_logs.log", logs_directory);
	logfile_names[&my_logs_dev] = "my_logs.log";
#ifdef GL_BOT_ENVIRONMENT
	logfile_paths[&guild_logs_release] = std::format("{}/release/guild_logs.log", logs_directory);
	logfile_names[&guild_logs_release] = "guild_logs.log";
	logfile_paths[&guild_logs_dev] = std::format("{}/dev/guild_logs.log", logs_directory);
	logfile_names[&guild_logs_dev] = "guild_logs.log";
	logfile_paths[&other_logs_release] = std::format("{}/release/other_logs.log", logs_directory);
	logfile_names[&other_logs_release] = "other_logs.log";
	logfile_paths[&other_logs_dev] = std::format("{}/dev/other_logs.log", logs_directory);
	logfile_names[&other_logs_dev] = "other_logs.log";
#else
	logfile_paths[&other_logs_release] = std::format("{}/release/dpp_logs.log", logs_directory);
	logfile_names[&other_logs_release] = "dpp_logs.log";
	logfile_paths[&other_logs_dev] = std::format("{}/dev/dpp_logs.log", logs_directory);
	logfile_names[&other_logs_dev] = "dpp_logs.log";
#endif
#ifdef GL_USE_LOCAL_DB
	logfile_paths[&sql_logs_release] = std::format("{}/release/sql_logs.log", logs_directory);
	logfile_names[&sql_logs_release] = "sql_logs.log";
	logfile_paths[&sql_logs_dev] = std::format("{}/dev/sql_logs.log", logs_directory);
	logfile_names[&sql_logs_dev] = "sql_logs.log";
#endif
	if (ENABLE_TOPGG) {
		logfile_paths[&topgg_logs_release] = std::format("{}/release/topgg_logs.log", logs_directory);
		logfile_names[&topgg_logs_release] = "topgg_logs.log";
		logfile_paths[&topgg_logs_dev] = std::format("{}/dev/topgg_logs.log", logs_directory);
		logfile_names[&topgg_logs_dev] = "topgg_logs.log";
	}
	for (auto& [logfile, path] : logfile_paths) {
		if (!TO_DUMP) {
			if (IS_CLI || !path.contains(IS_DEV ? "release" : "dev")) {
				logfile->open(path);
			}
		}
	}
}

#ifdef GL_BOT_ENVIRONMENT

void cfg::init_bot() {
	write_down_slashcommands();

	std::lock_guard L(cfg_values_mutex);

	dpptgg::poker* server_count_updater{};
	if (!TOPGG_BOT_TOKEN.empty()) {
		server_count_updater = new dpptgg::poker(TOPGG_BOT_TOKEN, bot);
	}
	const auto update_guild_count = [server_count_updater]() -> void {
		const uint64_t curr_guild_count = dpp::get_guild_count();
		bot->set_presence(dpp::presence(dpp::ps_idle, dpp::activity(
			dpp::activity_type::at_watching, std::format("VCs in {} guilds", dpp::get_guild_count()), "", ""
		)));
		if (!TOPGG_BOT_TOKEN.empty()) {
			server_count_updater->post_server_count([](const dpptgg::v0::request_completion_t&) {}, curr_guild_count);
		}
	};
	bot->start_timer([update_guild_count](const dpp::timer h) -> void {
		update_guild_count();
		bot->stop_timer(h);
	}, 10);
	update_guild_count();
	bot->start_timer([update_guild_count](const dpp::timer) -> void {
		update_guild_count();
	}, 180);
	// Keep on setting the presence to update the guild count on it.

	std::cout << "Guild count updater set up.\n";
	log("Guild count updater set up.");

	if (ENABLE_TOPGG) {
		configure_listener("../http_server/config.hfg");
		topgg_cluster = bot;
		auto* const listener = new dpptgg::listener(TOPGG_WEBHOOK_LISTEN_IP, TOPGG_WEBHOOK_LISTEN_PORT, secrets,
			send_vote_info, std::bind_back(serve_random_request, "../http_server/resources/"),
			topgg_cluster
		);
		listener->on_log(topgg_log);
		std::cout << "top.gg listener set up.\n";
		log("top.gg listener set up.");
	}
}

dpp::coroutine <> cfg::init_restriction_lists(const std::vector <dpp::snowflake>& channels) {
	for (const dpp::snowflake x : channels) {
		const dpp::channel channel = co_await lookup_channel(x);
		if (channel.get_type() != dpp::channel_type::CHANNEL_VOICE) {
			continue;
		}
		std::lock_guard L(restriction_mutex);
		for (const dpp::permission_overwrite& y : channel.permission_overwrites) {
			if (!temp_vc_is_accessible(y)) {
				banned[x].insert(y.id);
			}
			if (!temp_vc_is_speakable(y)) {
				muted[x].insert(y.id);
			}
		}
	}
}

void cfg::write_down_slashcommands() {
	std::lock_guard L(slashcommands::list_mutex);

	slash::global_created.clear();
	slash::guild_created.clear();
	bot->global_commands_get([](const dpp::confirmation_callback_t& callback) -> void {
		if (error_callback(callback)) {
			return;
		}
		const auto& map = callback.get <dpp::slashcommand_map>();

		std::lock_guard L(slashcommands::list_mutex);
		for (const auto& cmd : map | std::views::values) {
			slash::global_created[cmd.name] = cmd;
			if (IS_CLI) {
				slash::global_vector.push_back(cmd);
			}
		}
		bot->guild_commands_get(MY_PRIVATE_GUILD_ID, [](const dpp::confirmation_callback_t& callback) -> void {
			if (error_callback(callback)) {
				return;
			}
			const auto& map = callback.get <dpp::slashcommand_map>();

			std::lock_guard L(slashcommands::list_mutex);
			for (const auto& cmd : map | std::views::values) {
				slash::guild_created[cmd.name] = cmd;
			}
		});
	});
}

dpp::message cfg::help_message(const std::string_view lang, const uint8_t page) {
	std::lock_guard L(config_mutex);
	const henifig::value_map& help_components = responses["LOCALISATION"]["HELP_EMBEDS"];
	const henifig::value_t& rp = response(DESCRIPTIONS, lang, help_components);
	if (!rp.is <henifig::array_t>()) {
		return dpp::message(rp.get <std::string>()).set_flags(dpp::m_ephemeral);
	}
	const std::vector <std::string> descriptions = rp;
	const henifig::value_array& help_cmd_mentions = responses["HELP_COMMAND_MENTIONS"];
	const std::vector <std::string> help_cmd_page_names = get_help_command_page_names(lang);
	std::string pagination_string;
	for (uint8_t i = 0; i < help_cmd_mentions.size(); i++) {
		if (i == page) {
			pagination_string += std::format("**{0}**. **{1}**\n", i + 1, help_cmd_page_names[i]);
		}
		else {
			pagination_string += std::format("{0}. {1}\n", i + 1, help_cmd_page_names[i]);
		}
	}

	dpp::message res;
	res.add_embed(
		dpp::embed()
		.set_color(dpp::colors::sti_blue)
		.set_description(std::format("{0}{1}{2}",
			pagination_string,
			std::format("# {}\n", help_cmd_page_names[page]),
			format_with_vector(descriptions[page], slash::get_mentions(help_cmd_mentions[page])))
		)
	);
	res.embeds[0].set_author(response(AUTHOR, lang, help_components), bot->me.get_url(), bot->me.get_avatar_url());
	res.embeds[0].set_footer(dpp::embed_footer()
		.set_text(response(FOOTER, lang, help_components))
		.set_icon(bot->me.get_avatar_url())
	);
	dpp::component buttons;
	buttons.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(std::format("help{}", page - 1))
		.set_label(dpp::unicode_emoji::arrow_left)
	);
	if (page == 0) {
		buttons.components[0].set_disabled(true);
	}
	buttons.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id("help_search")
		.set_label(dpp::unicode_emoji::mag)
	);
	buttons.add_component(dpp::component()
		.set_type(dpp::cot_button)
		.set_id(std::format("help{}", page + 1))
		.set_label(dpp::unicode_emoji::arrow_right)
	);
	if (page == descriptions.size() - 1) {
		buttons.components[2].set_disabled(true);
	}
	return res.add_component(buttons).set_flags(dpp::m_ephemeral);
}

#endif
