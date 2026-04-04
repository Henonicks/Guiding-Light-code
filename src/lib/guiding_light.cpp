#include "guiding_light/guiding_light.hpp"

#include "guiding_light/logging.hpp"
#include "guiding_light/responses.hpp"
#include "guiding_light/temp_vc_handler.hpp"
#include "guiding_light/slash_funcs.hpp"

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
		slash::enabled = true;
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
