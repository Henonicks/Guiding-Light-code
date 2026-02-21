#include "guiding_light/logging.hpp"
#include "guiding_light/responses.hpp"

void autodump(std::ofstream* logfile)  {
	if (logfile->tellp() >= LOGS_MAX_SIZE) {
		open_logfile(logfile);
	}
}

void open_logfile(std::ofstream* logfile)  {
	dump_logfile(logfile);
	if (logfile->is_open()) {
		logfile->close();
		logfile->clear();
	}
	logfile->open(logfile_paths[logfile], std::ofstream::out | std::ofstream::trunc);
}

void dump_logfile(std::ofstream* logfile) {
	const std::string logfile_content = dpp::utility::read_file(logfile_paths[logfile]);
	bot->message_create(dpp::message(LOGS_CHANNEL_ID, "Autodumping.")
		.add_file(logfile_names[logfile], logfile_content)
	, [logfile_content, logfile](const dpp::confirmation_callback_t& callback) {
		if (callback.is_error()) {
			backup_logfile(logfile, logfile_content);
		}
	});
}

void backup_logfile(std::ofstream* logfile, const std::string_view logfile_content) {
	for (int i = 0; true; i++) {
		const std::string backup_path = logfile_paths[logfile] + ".backup" + std::to_string(i);
		if (!std::filesystem::exists(backup_path)) {
			std::ofstream backup_file;
			backup_file.open(backup_path);
			backup_file << logfile_content;
			backup_file.close();
			break;
		}
	}
}

void bot_log(const dpp::log_t& _log) {
	std::ofstream* other_logs = &(IS_DEV ? other_logs_dev : other_logs_release);
	*other_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), _log.message) << std::endl;
	if (_log.message == "Shards started.") {
		if (IS_CLI) {
			return;
		}
		if (!db::connection_successful()) {
			std::cerr << fmt::format("{0} connection to DB failed! imma js crash ts g 💔🥀\nHINT: have you imported your database as database/{1}.db or initialised the database with init_db?", color::rize("ERROR:", "Red"), MODE_NAME) << std::endl;
			exit(1);
		}
		log("Waiting till we receive all the cache...");
		bot->start_timer([](const dpp::timer& h) -> void {
			const uint64_t new_guild_amount = dpp::get_guild_count();
			const uint64_t new_channel_amount = dpp::get_channel_count();
			const uint64_t new_user_amount = dpp::get_user_count();
			if (!(new_guild_amount > guild_amount || new_channel_amount > channel_amount || new_user_amount > user_amount)) {
				if (!move_on) {
					log("Confirming the absence of cache updates...");
					move_on = true;
				}
				else {
					log("Done! Doing stuff...");
					cfg::pray();
					if (bot_offline) {
						bot->stop_timer(h);
					}
					cfg::write_down_slashcommands();
					log("Done doing stuff!");
					bot->stop_timer(h);
				}
			}
			else {
				move_on = false;
				guild_amount = new_guild_amount;
				channel_amount = new_channel_amount;
				user_amount = new_user_amount;
				log(fmt::format("Amount of guilds, channels and users: {0}, {1}, {2}", new_guild_amount, new_channel_amount, new_user_amount));
			}
		}, CACHE_DELAY);
	}
}

void log(const std::string_view message) {
	std::ofstream* my_logs = &(IS_DEV ? my_logs_dev : my_logs_release);
	*my_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
	autodump(my_logs);
	// Here's a note to myself,
	// i know you're itching to replace "std::endl" with "'\n'" but please don't do it cuz then the logs won't work bro trust
}

void error_log(const std::string_view message, const std::string_view human_readable) {
	const uint64_t current_time = bot->uptime().to_secs();
	if (current_time >= last_error_message + 10) {
		bot->message_create(dpp::message(LOGS_CHANNEL_ID, fmt::format("ERROR! <@{0}> Go check **your** logs! Current message: `{1}`",
			MY_ID, !human_readable.empty() ? human_readable : message))
		.set_allowed_mentions(true), error_callback);
		last_error_message = current_time;
	}
	log(message);
}

void guild_log(const std::string_view message) {
	std::ofstream* guild_logs = &(IS_DEV ? guild_logs_dev : guild_logs_release);
	*guild_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
	// i hope you know the drill by now
}

void sql_log(const sqlite::sqlite_exception& e, const std::string_view function) {
	std::ofstream* sql_logs = &(IS_DEV? sql_logs_dev : sql_logs_release);
	*sql_logs << fmt::format(
		"[{0}]:{1} Error code: {2}, error: {3}, query: {4}",
		dpp::utility::current_date_time(), !function.empty() ? fmt::format(" In {}:", function) : "", e.get_code(), e.what(), e.get_sql()
	) << std::endl;
	// this ain't a drill, this is the way that I dream of goin' out
}

bool error_callback(const dpp::confirmation_callback_t& callback) {
	if (callback.is_error()) {
		std::string error = "ERROR! ";
		if (!callback.get_error().errors.empty()) {
			for (const dpp::error_detail& x : callback.get_error().errors) {
				error += fmt::format("\n\tFIELD: {0} REASON: {1} OBJECT: {2} CODE: {3}", x.field, x.reason, x.object, x.code);
			}
		}
		else {
			error += callback.get_error().human_readable;
		}
		if (!callback.http_info.headers.empty()) {
			error += "\nHeaders:\n{";
			for (const auto& [key, value] : callback.http_info.headers) {
				error += fmt::format("\n\t{0}: {1}", key, value);
			}
			error += "\n}";
		}
		if (!callback.http_info.body.empty()) {
			error += fmt::format("\nReply body: \n\t{}", callback.http_info.body);
		}
		error_log(error, callback.get_error().human_readable);
		return true;
	}
	return false;
}

bool error_pingback(const dpp::confirmation_callback_t& callback, const channel_snowflake& channel_id, const user_snowflake& user_id, const std::string_view error_intro) {
	if (callback.is_error()) {
		std::string error = "ERROR! ";
		if (!callback.get_error().errors.empty()) {
			for (const dpp::error_detail& x : callback.get_error().errors) {
				error += fmt::format("\n\tFIELD: {0} REASON: {1} OBJECT: {2} CODE: {3}", x.field, x.reason, x.object, x.code);
			}
		}
		else {
			error += callback.get_error().human_readable;
		}
		if (!callback.http_info.headers.empty()) {
			error += "\nHeaders:";
			for (const auto& [key, value] : callback.http_info.headers) {
				error += fmt::format("\n\t{0}: {1}", key, value);
			}
		}
		if (!callback.http_info.body.empty()) {
			error += fmt::format("\nReply body: \n\t{}", callback.http_info.body);
		}
		error_log(error, callback.get_error().human_readable);
		const std::string human_readable = fmt::format("{0}{1}: {2}. {3}\n{4}",
			(!user_id.empty() ? dpp::utility::user_mention(user_id) + '\n' : ""),
			error_intro, callback.get_error().message, error_response(callback.get_error().code),
			response_str(CREATE_A_TICKET));
		bot->message_create(dpp::message(channel_id, human_readable).set_allowed_mentions(true), error_callback);
		return true;
	}
	return false;
}

bool error_feedback(const dpp::confirmation_callback_t& callback, const dpp::interaction_create_t& event, const std::string_view lang, const std::string_view error_intro) {
	if (callback.is_error()) {
		std::string error = "ERROR! ";
		if (!callback.get_error().errors.empty()) {
			for (const dpp::error_detail& x : callback.get_error().errors) {
				error += fmt::format("\n\tFIELD: {0} REASON: {1} OBJECT: {2} CODE: {3}", x.field, x.reason, x.object, x.code);
			}
		}
		else {
			error += callback.get_error().human_readable;
		}
		if (!callback.http_info.headers.empty()) {
			error += "\nHeaders:";
			for (const auto& [key, value] : callback.http_info.headers) {
				error += fmt::format("\n\t{0}: {1}", key, value);
			}
		}
		if (!callback.http_info.body.empty()) {
			error += fmt::format("\nReply body: \n\t{}", callback.http_info.body);
		}
		error_log(error, callback.get_error().human_readable);
		const std::string human_readable = fmt::format("{0}: {1}. {2}\n{3}", error_intro, callback.get_error().message, error_response(callback.get_error().code, lang), response_str(CREATE_A_TICKET, lang));
		event.reply(dpp::message(human_readable).set_flags(dpp::m_ephemeral), error_callback);
		return true;
	}
	return false;
}

bool error_feedback(const dpp::confirmation_callback_t& callback, const dpp::message_create_t& event, std::string_view error_intro) {
	if (callback.is_error()) {
		std::string error = "ERROR! ";
		if (!callback.get_error().errors.empty()) {
			for (const auto& x : callback.get_error().errors) {
				error += fmt::format("\n\tFIELD: {0} REASON: {1} OBJECT: {2} CODE: {3}", x.field, x.reason, x.object, x.code);
			}
		}
		else {
			error += callback.get_error().human_readable;
		}
		if (!callback.http_info.headers.empty()) {
			error += "\nHeaders:";
			for (const auto& [key, value] : callback.http_info.headers) {
				error += fmt::format("\n\t{0}: {1}", key, value);
			}
		}
		if (!callback.http_info.body.empty()) {
			error += fmt::format("\nReply body: \n\t{}", callback.http_info.body);
		}
		error_log(error, callback.get_error().human_readable);
		const std::string human_readable = fmt::format("{0}: {1}. {2}\n{3}", error_intro, callback.get_error().message, error_response(callback.get_error().code), response_str(CREATE_A_TICKET));
		event.reply(dpp::message(human_readable), true, error_callback);
		return true;
	}
	return false;
}
