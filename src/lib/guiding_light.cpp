#include "guiding_light/guiding_light.hpp"

#include "guiding_light/logging.hpp"

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

void dump_data(const int code) {
	bot->message_create(
		dpp::message(LOGS_CHANNEL_ID, "Shutting down, dumping.")
			.add_file(fmt::format("{}.db", MODE_NAME), dpp::utility::read_file(fmt::format("../database/{}.db", MODE_NAME)))
			.add_file("my_logs.log", dpp::utility::read_file(fmt::format("../logging/bot/{}/my_logs.log", MODE_NAME)))
			.add_file("other_logs.log", dpp::utility::read_file(fmt::format("../logging/bot/{}/other_logs.log", MODE_NAME)))
			.add_file("guild_logs.log", dpp::utility::read_file(fmt::format("../logging/bot/{}/guild_logs.log", MODE_NAME)))
			.add_file("sql_logs.log", dpp::utility::read_file(fmt::format("../logging/bot/{}/sql_logs.log", MODE_NAME)))
	, [code](const dpp::confirmation_callback_t& callback) {
		if (!error_callback(callback)) {
			log("Goodnight!");
			std::cout << "Goodnight!\n";
			exit(code);
		}
		else {
			log("nvm lmao");
			std::cout << "nvm lmao\n";
		}
	});
}
