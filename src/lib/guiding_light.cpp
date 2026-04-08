#include "guiding_light/guiding_light.hpp"

#include "guiding_light/logging.hpp"
#include "guiding_light/responses.hpp"
#include "guiding_light/temp_vc_handler.hpp"
#include "guiding_light/slash_funcs.hpp"

void wait_for_guild_readiness(const dpp::snowflake guild_id) {
	std::mutex wait_mutex;
	std::unique_lock this_lock(wait_mutex);
	guild_readiness_cv.wait(this_lock, [guild_id] {
		return ready_guilds.contains(guild_id);
	});
}

dpp::cluster* get_bot() {
	return !IS_DEV ? bot_release : bot_dev;
}

bool is_running() {
	return !bot->me.id.empty();
	// We get the ID when the bot is ready, so if we have
	// the ID already then the bot is definitely running.
}

std::string bot_name() {
	return !IS_DEV ? "Guiding Light" : "Curious Light";
	// See README.md if you wonder why these names.
}

void dump_data(const bool fatal, const bool deadlock) {
	if (!deadlock) {
		std::cout << "Waiting for all the mutexes to be free.\n";
		log("Waiting for all the mutexes to be free.");
		bool lock_passed{};
		std::thread deadlock_prevention([fatal, &lock_passed]() -> void {
			std::this_thread::sleep_for(std::chrono::seconds(5));
			if (!lock_passed) {
				std::cout << "5 seconds in, nothing happened. Impatiently dumping instead.\n";
				log("5 seconds in, nothing happened. Impatiently dumping instead.");
				dump_data(fatal, true);
			}
		});
		deadlock_prevention.detach();
		std::scoped_lock L(
			cfg::config_mutex,
			slashcommands::list_mutex,
			cfg_values_mutex,
			server_mutex,
			jtc_mutex,
			logfile_mutex,
			notification_mutex,
			ratelimit_mutex,
			slash::in_progress_mutex,
			temp_vc_mutex,
			restriction_mutex,
			ticket_mutex,
			topgg::mutex,
			db::mutex
		);
		lock_passed = true;
	}
	bot->message_create(
		dpp::message(LOGS_CHANNEL_ID, "Shutting down, dumping.")
			.add_file(fmt::format("{}.db", MODE_NAME), dpp::utility::read_file(fmt::format("../database/{}.db", MODE_NAME)))
			.add_file("my_logs.log", dpp::utility::read_file(fmt::format("../logging/bot/{}/my_logs.log", MODE_NAME)))
			.add_file("other_logs.log", dpp::utility::read_file(fmt::format("../logging/bot/{}/other_logs.log", MODE_NAME)))
			.add_file("guild_logs.log", dpp::utility::read_file(fmt::format("../logging/bot/{}/guild_logs.log", MODE_NAME)))
			.add_file("sql_logs.log", dpp::utility::read_file(fmt::format("../logging/bot/{}/sql_logs.log", MODE_NAME)))
	, [fatal](const dpp::confirmation_callback_t& callback) {
		if (error_callback(callback)) {
			log("Couldn't dump on Discord, backing up instead.");
			std::cout << "Couldn't dump on Discord, backing up instead.\n";
			for (auto& [logfile, path] : logfile_paths) {
				if (path.find(MODE_NAME) != std::string::npos) {
					backup_logfile(logfile_names[logfile], dpp::utility::read_file(path));
				}
			}
		}
		log("Goodnight!");
		std::cout << "Goodnight!\n";
		if (fatal) {
			std::abort();
		}
		else {
			std::exit(0);
		}
	});
}

dpp::coroutine <dpp::user> lookup_user(const dpp::snowflake user_id) {
	std::shared_lock L(dpp::get_user_cache()->get_mutex());
	dpp::user* res = dpp::find_user(user_id);
	if (res == nullptr) {
		L.unlock();
		const dpp::confirmation_callback_t callback = co_await bot->co_user_get(user_id);
		if (error_callback(callback)) {
			co_return {};
		}
		res = new dpp::user;
		*res = callback.get <dpp::user>();
		dpp::get_user_cache()->store(res);
	}
	co_return *res;
}

dpp::coroutine <dpp::channel> lookup_channel(const dpp::snowflake channel_id) {
	std::shared_lock L(dpp::get_channel_cache()->get_mutex());
	dpp::channel* res = dpp::find_channel(channel_id);
	if (res == nullptr) {
		L.unlock();
		const dpp::confirmation_callback_t callback = co_await bot->co_channel_get(channel_id);
		if (error_callback(callback)) {
			co_return {};
		}
		res = new dpp::channel;
		*res = callback.get <dpp::channel>();
		dpp::get_channel_cache()->store(res);
	}
	co_return *res;
}

dpp::coroutine <dpp::role> lookup_role(const dpp::snowflake role_id, const dpp::snowflake guild_id) {
	std::shared_lock L(dpp::get_role_cache()->get_mutex());
	dpp::role* res = dpp::find_role(role_id);
	if (res == nullptr) {
		L.unlock();
		const dpp::confirmation_callback_t callback = co_await bot->co_roles_get(guild_id);
		if (error_callback(callback)) {
			co_return {};
		}
		const auto& callback_res = callback.get <dpp::role_map>();
		std::ranges::for_each(callback_res, [](const auto& role_pair) {
			auto* const role = new dpp::role;
			*role = role_pair.second;
			dpp::get_role_cache()->store(role);
		});
		if (callback_res.contains(role_id)) {
			co_return callback_res.at(role_id);
		}
		co_return {};
	}
	co_return *res;
}

dpp::coroutine <dpp::guild> lookup_guild(const dpp::snowflake guild_id) {
	std::shared_lock L(dpp::get_guild_cache()->get_mutex());
	dpp::guild* res = dpp::find_guild(guild_id);
	if (res == nullptr) {
		L.unlock();
		const dpp::confirmation_callback_t callback = co_await bot->co_guild_get(guild_id);
		if (error_callback(callback)) {
			co_return {};
		}
		res = new dpp::guild;
		*res = callback.get <dpp::guild>();
		dpp::get_guild_cache()->store(res);
	}
	co_return *res;
}

dpp::coroutine <dpp::emoji> lookup_emoji(const dpp::snowflake emoji_id) {
	std::shared_lock L(dpp::get_emoji_cache()->get_mutex());
	dpp::emoji* res = dpp::find_emoji(emoji_id);
	if (res == nullptr) {
		L.unlock();
		const dpp::confirmation_callback_t callback = co_await bot->co_application_emoji_get(emoji_id);
		if (error_callback(callback)) {
			co_return {};
		}
		const auto& callback_res = callback.get <dpp::emoji_map>();
		std::ranges::for_each(callback_res, [](const auto& emoji_pair) {
			auto* const role = new dpp::emoji;
			*role = emoji_pair.second;
			dpp::get_emoji_cache()->store(role);
		});
		if (callback_res.contains(emoji_id)) {
			co_return callback_res.at(emoji_id);
		}
		co_return {};
	}
	co_return *res;
}

dpp::coroutine <dpp::emoji> lookup_emoji(const dpp::snowflake emoji_id, const dpp::snowflake guild_id) {
	std::shared_lock L(dpp::get_emoji_cache()->get_mutex());
	dpp::emoji* res = dpp::find_emoji(emoji_id);
	if (res == nullptr) {
		L.unlock();
		const dpp::confirmation_callback_t callback = co_await bot->co_guild_emoji_get(guild_id, emoji_id);
		if (error_callback(callback)) {
			co_return {};
		}
		const auto& callback_res = callback.get <dpp::emoji_map>();
		std::ranges::for_each(callback_res, [](const auto& emoji_pair) {
			auto* const role = new dpp::emoji;
			*role = emoji_pair.second;
			dpp::get_emoji_cache()->store(role);
		});
		if (callback_res.contains(emoji_id)) {
			co_return callback_res.at(emoji_id);
		}
		co_return {};
	}
	co_return *res;
}

dpp::coroutine <dpp::guild_member> lookup_guild_member(const dpp::snowflake guild_id, const dpp::snowflake user_id) {
	std::shared_lock L(dpp::get_guild_cache()->get_mutex());
	try {
		co_return dpp::find_guild_member(guild_id, user_id);
	}
	catch (...) {}
	L.unlock();
	const dpp::confirmation_callback_t callback = co_await bot->co_guild_get_member(guild_id, user_id);
	if (error_callback(callback)) {
		co_return {};
	}
	co_return callback.get <dpp::guild_member>();
}
