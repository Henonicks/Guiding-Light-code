#include "logging.h"

void bot_log(const dpp::log_t& log) {
	other_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), log.message) << std::endl;
	if (log.message == "Shards started.") {
		configuration::configure_channels();
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
