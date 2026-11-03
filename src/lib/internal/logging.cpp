#include "guiding_light/internal/logging.hpp"

#include "guiding_light/helpers/format.tpp"
#include "guiding_light/internal/config_values.hpp"

void autodump(std::ofstream* const logfile) {
	if (logfile->tellp() >= LOGS_MAX_SIZE) {
		open_logfile(logfile);
	}
}

void open_logfile(std::ofstream* const logfile) {
	dump_logfile(logfile);
	if (logfile->is_open()) {
		logfile->close();
		logfile->clear();
	}
	logfile->open(logfile_paths[logfile], std::ofstream::out | std::ofstream::trunc);
}

void dump_logfile(std::ofstream* logfile) {
	std::lock_guard L(logfile_mutex);

	const std::string logfile_content = dpp::utility::read_file(logfile_paths[logfile]);
	bot->message_create(dpp::message(LOGS_CHANNEL_ID, "Autodumping.")
		.add_file(logfile_names[logfile], logfile_content)
	, [logfile_content, logfile](const dpp::confirmation_callback_t& callback) {
		if (callback.is_error()) {
			backup_logfile(logfile_names[logfile], logfile_content);
		}
	});
}

void backup_logfile(const std::string_view logfile_name, const std::string_view logfile_content) {
	for (int i = 0; true; i++) {
		const std::string backup_path = std::string(logfile_name) + ".backup" + std::to_string(i);
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
	std::lock_guard L(logfile_mutex);

	std::ofstream* topgg_logs = &(IS_DEV ? other_logs_dev : other_logs_release);
	*topgg_logs << std::format("[{0}]: {1}", dpp::utility::current_date_time(), _log.message) << std::endl;
	autodump(topgg_logs);
}

void topgg_log(const dpp::log_t& _log) {
	std::lock_guard L(logfile_mutex);

	std::ofstream* topgg_logs = &(IS_DEV ? topgg_logs_dev : topgg_logs_release);
	*topgg_logs << std::format("[{0}]: {1}", dpp::utility::current_date_time(), _log.message) << std::endl;
	autodump(topgg_logs);
}

void log(const std::string_view message) {
	std::lock_guard L(logfile_mutex);
	std::ofstream* my_logs = &(IS_DEV ? my_logs_dev : my_logs_release);
	const std::string full_message = std::format("[{0}]: {1}", dpp::utility::current_date_time(), message);
	*my_logs << full_message << std::endl;
	if (IS_DEV && !IS_CLI) {
		std::cout << full_message << std::endl;
	}
	// Here's a note to myself,
	// i know you're itching to replace "std::endl" with "'\n'" but please don't do it cuz then the logs won't work bro trust
	autodump(my_logs);
}

const std::unordered_set <uint32_t> ignored_error_codes = {
	10003, 10062, 40032, 50013
};

void error_log(const std::string_view message, const uint32_t error_code, const std::string_view human_readable) {
#ifdef GL_BOT_ENVIRONMENT
	if (!ignored_error_codes.contains(error_code)) {
		const uint64_t current_time = bot->uptime().to_secs();
		if (current_time >= last_error_message + 10) {
			bot->message_create(dpp::message(LOGS_CHANNEL_ID, std::format("ERROR! <@{0}> Go check **your** logs! Current message: `{1}`",
				MY_ID.load(), !human_readable.empty() ? human_readable : message))
			.set_allowed_mentions(true), error_callback);
			last_error_message = current_time;
		}
	}
#endif
	log(message);
}

#ifdef GL_BOT_ENVIRONMENT

void guild_log(const std::string_view message) {
	std::lock_guard L(logfile_mutex);

	std::ofstream* guild_logs = &(IS_DEV ? guild_logs_dev : guild_logs_release);
	*guild_logs << std::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
	autodump(guild_logs);
	// i hope you know the drill by now
}

#endif

#ifdef GL_USE_LOCAL_DB

void sql_log(const std::string_view function) {
	std::lock_guard L(logfile_mutex);

	std::ofstream* sql_logs = &(IS_DEV? sql_logs_dev : sql_logs_release);
	*sql_logs << std::format(
		"[{0}]: New errors coming up from {1}!",
		dpp::utility::current_date_time(), function
	) << std::endl;
	autodump(sql_logs);
	// this ain't a drill, this is the way that I dream of goin' out
}

void sql_log(const std::exception& e, const std::string_view function) {
	std::lock_guard L(logfile_mutex);

	std::ofstream* sql_logs = &(IS_DEV? sql_logs_dev : sql_logs_release);
	*sql_logs << std::format(
		"[{0}]:{1} error: {2}",
		dpp::utility::current_date_time(), !function.empty() ? std::format(" In {}:", function) : "", e.what()
	) << std::endl;
	autodump(sql_logs);
	// this ain't a drill, this is the way that I dream of goin' out
}

void sql_log(const sqlite::sqlite_exception& e, const std::string_view function) {
	std::lock_guard L(logfile_mutex);

	std::ofstream* sql_logs = &(IS_DEV? sql_logs_dev : sql_logs_release);
	*sql_logs << std::format(
		"[{0}]:{1} Error code: {2}, error: {3}, query: {4}",
		dpp::utility::current_date_time(), !function.empty() ? std::format(" In {}:", function) : "", e.get_code(), e.what(), e.get_sql()
	) << std::endl;
	autodump(sql_logs);
	// this ain't a drill, this is the way that I dream of goin' out
}

#endif

bool error_callback(const dpp::confirmation_callback_t& callback) {
	std::lock_guard L(logfile_mutex);

	if (callback.is_error()) {
		std::string error = "ERROR! ";
		if (!callback.get_error().errors.empty()) {
			for (const dpp::error_detail& x : callback.get_error().errors) {
				error += std::format("\n\tFIELD: {0} REASON: {1} OBJECT: {2} CODE: {3}", x.field, x.reason, x.object, x.code);
			}
		}
		else {
			error += callback.get_error().human_readable;
		}
		if (!callback.http_info.body.empty()) {
			error += std::format("\nReply body: \n\t{}", callback.http_info.body);
		}
		error_log(error, callback.get_error().code, callback.get_error().human_readable);
		return true;
	}
	return false;
}

#include "guiding_light/handlers/responses.hpp"

bool error_pingback(const dpp::confirmation_callback_t& callback, const channel_snowflake& channel_id, const user_snowflake& user_id, const std::string_view error_intro) {
	std::lock_guard L(logfile_mutex);

	if (callback.is_error()) {
		std::string error = "ERROR! ";
		if (!callback.get_error().errors.empty()) {
			for (const dpp::error_detail& x : callback.get_error().errors) {
				error += std::format("\n\tFIELD: {0} REASON: {1} OBJECT: {2} CODE: {3}", x.field, x.reason, x.object, x.code);
			}
		}
		else {
			error += callback.get_error().human_readable;
		}
		if (!callback.http_info.body.empty()) {
			error += std::format("\nReply body: \n\t{}", callback.http_info.body);
		}
		error_log(error, callback.get_error().code, callback.get_error().human_readable);
#ifdef GL_BOT_ENVIRONMENT
		const std::string human_readable = std::format("{0}{1}: {2}. {3}\n{4}",
			(!user_id.empty() ? dpp::utility::user_mention(user_id) + '\n' : ""),
			error_intro, callback.get_error().message, error_response(callback.get_error().code),
			response_str(CREATE_A_TICKET)
		);
		bot->message_create(dpp::message(channel_id, human_readable).set_allowed_mentions(true), error_callback);
#endif
		return true;
	}
	return false;
}

bool error_feedback(const dpp::confirmation_callback_t& callback, const dpp::interaction_create_t& event, const std::string_view lang, const std::string_view error_intro) {
	std::lock_guard L(logfile_mutex);

	if (callback.is_error()) {
		std::string error = "ERROR! ";
		if (!callback.get_error().errors.empty()) {
			for (const dpp::error_detail& x : callback.get_error().errors) {
				error += std::format("\n\tFIELD: {0} REASON: {1} OBJECT: {2} CODE: {3}", x.field, x.reason, x.object, x.code);
			}
		}
		else {
			error += callback.get_error().human_readable;
		}
		if (!callback.http_info.body.empty()) {
			error += std::format("\nReply body: \n\t{}", callback.http_info.body);
		}
		error_log(error, callback.get_error().code, callback.get_error().human_readable);
#ifdef GL_BOT_ENVIRONMENT
		const std::string human_readable = std::format("{0}: {1}. {2}\n{3}",
			error_intro, callback.get_error().message,
			error_response(callback.get_error().code, lang), response_str(CREATE_A_TICKET, lang)
		);
		event.reply(dpp::message(human_readable).set_flags(dpp::m_ephemeral), error_callback);
#endif
		return true;
	}
	return false;
}

bool error_feedback(const dpp::confirmation_callback_t& callback, const dpp::message_create_t& event, std::string_view error_intro) {
	std::lock_guard L(logfile_mutex);

	if (callback.is_error()) {
		std::string error = "ERROR! ";
		if (!callback.get_error().errors.empty()) {
			for (const auto& x : callback.get_error().errors) {
				error += std::format("\n\tFIELD: {0} REASON: {1} OBJECT: {2} CODE: {3}", x.field, x.reason, x.object, x.code);
			}
		}
		else {
			error += callback.get_error().human_readable;
		}
		if (!callback.http_info.body.empty()) {
			error += std::format("\nReply body: \n\t{}", callback.http_info.body);
		}
		error_log(error, callback.get_error().code, callback.get_error().human_readable);
#ifdef GL_BOT_ENVIRONMENT
		const std::string human_readable = std::format("{0}: {1}. {2}\n{3}",
			error_intro, callback.get_error().message,
			error_response(callback.get_error().code), response_str(CREATE_A_TICKET)
		);
		event.reply(dpp::message(human_readable), true, error_callback);
#endif
		return true;
	}
	return false;
}
