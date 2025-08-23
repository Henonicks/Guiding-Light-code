#include "guiding_light/logging.hpp"

void bot_log(const dpp::log_t& _log) {
	std::ofstream* other_logs = &(IS_DEV ? other_logs_dev : other_logs_release);
	*other_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), _log.message) << std::endl;
	if (_log.message == "Shards started.") {
		if (!db::connection_successful()) {
			std::cerr << fmt::format("ERROR: connection to DB failed! imma js crash ts g 💔🥀\nHINT: have you imported your database as database/{}.db or initialised the database with init_db.sh?", MODE_NAME) << std::endl;
			bot->shutdown();
			return;
		}
		log("Waiting till we receive all the cache...");
		bot->start_timer([](const dpp::timer& h) -> void {
			const uint64_t new_guild_amount = dpp::get_guild_count();
			const uint64_t new_channel_amount = dpp::get_channel_count();
			const uint64_t new_user_amount = dpp::get_user_count();
			if (!(new_guild_amount > guild_amount || new_channel_amount > channel_amount || new_user_amount > user_amount)) {
				log("Done! Doing stuff...");
				configuration::pray();
				configuration::write_down_slashcommands();
				log("Done doing stuff!");
				bot->stop_timer(h);
			}
			else {
				guild_amount = new_guild_amount;
				channel_amount = new_channel_amount;
				user_amount = new_user_amount;
				log(fmt::format("Amount of guilds, channels and users: {0}, {1}, {2}", new_guild_amount, new_channel_amount, new_user_amount));
			}
		}, DELAY);
	}
}

void log(std::string_view message) {
	std::ofstream* my_logs = &(IS_DEV ? my_logs_dev : my_logs_release);
	*my_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
	// Here's a note to myself,
	// i know you're itching to replace "std::endl" with "'\n'" but please don't do it cuz then the logs won't work bro trust
}

void guild_log(std::string_view message) {
	std::ofstream* guild_logs = &(IS_DEV ? guild_logs_dev : guild_logs_release);
	*guild_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
	// i hope you know the drill by now
}

void sql_log(const sqlite::sqlite_exception& e, std::string_view function) {
	std::ofstream* sql_logs = &(IS_DEV? sql_logs_dev : sql_logs_release);
	*sql_logs << fmt::format(
		"[{0}]:{1} Error code: {2}, error: {3}, query: {4}",
		dpp::utility::current_date_time(), !function.empty() ? fmt::format(" In {}:", function) : "", e.get_code(), e.what(), e.get_sql()
	) << std::endl;
	// this ain't a drill, this is the way that I dream of goin' out
}

void error_callback(const dpp::confirmation_callback_t& callback) {
	if (callback.is_error()) {
		if (!callback.get_error().errors.empty()) {
			log(fmt::format("ERROR! FIELD: {0} REASON: {1}", callback.get_error().errors[0].field, callback.get_error().errors[0].reason));
		}
		else {
			log("ERROR!" + callback.get_error().message);
		}
	}
}

void error_feedback(const dpp::confirmation_callback_t& callback, const dpp::interaction_create_t& event, std::string_view error_intro) {
	if (callback.is_error()) {
		if (!callback.get_error().errors.empty()) {
			log(fmt::format("ERROR! FIELD: {0} REASON: {1}", callback.get_error().errors[0].field, callback.get_error().errors[0].reason));
		}
		else {
			log("ERROR!" + callback.get_error().message);
		}
		event.reply(fmt::format("{0}: {1}.", error_intro, callback.get_error().message), error_callback);
	}
}

void error_feedback(const dpp::confirmation_callback_t& callback, const dpp::message_create_t& event, std::string_view error_intro) {
	if (callback.is_error()) {
		if (!callback.get_error().errors.empty()) {
			log(fmt::format("ERROR! FIELD: {0} REASON: {1}", callback.get_error().errors[0].field, callback.get_error().errors[0].reason));
		}
		else {
			log("ERROR!" + callback.get_error().message);
		}
		event.reply(fmt::format("{0}: {1}.", error_intro, callback.get_error().message), true, error_callback);
	}
}
