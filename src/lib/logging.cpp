#include "logging.h"

void bot_log(const dpp::log_t& _log, dpp::cluster& bot) {
	other_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), _log.message) << std::endl;
	if (_log.message == "Shards started.") {
		log(fmt::format("[{}]: Waiting till we receive all the cache...", dpp::utility::current_date_time()));
		bot.start_timer([&bot](dpp::timer h) -> void {
			const uint64_t new_guild_amount = dpp::get_guild_count();
			const uint64_t new_channel_amount = dpp::get_channel_count();
			const uint64_t new_user_amount = dpp::get_user_count();
			if (!(new_guild_amount > guild_amount || new_channel_amount > channel_amount || new_user_amount > user_amount)) {
				configuration::pray(bot);
				configuration::write_down_slashcommands(bot);
				bot.stop_timer(h);
			}
			else {
				guild_amount = new_guild_amount;
				channel_amount = new_channel_amount;
				user_amount = new_user_amount;
				log(fmt::format("Amount of guilds, channels and users: {0}, {1}, {2}", new_guild_amount, new_channel_amount, new_user_amount));
			}
		}, delay);
	}
}

void log(std::string_view message) {
    my_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
	// i know you're itching to replace "std::endl" with "'\n'" but please don't do it cuz then the logs won't work bro trust
}

void guild_log(std::string_view message) {
	guild_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
	// no seriously dude don't replace that fr bro no cap youll regret this on god
}
