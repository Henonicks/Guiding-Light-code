#include "guiding_light/database.hpp"
#include "guiding_light/configuration.hpp"

std::string_view db::LOCATION = "../database";
std::string_view db::SELECT_LOCATION = "../database/select";
const std::set <std::string> db::table_names =
{"jtc_vcs", "temp_vc_notifications", "jtc_default_values", "no_temp_ping", "topgg_guild_choices", "topgg_guild_votes_amount", "no_noguild_reminder", "topgg_notifications", "tickets", "temp_vcs"};
db::wrapper db::sql{""};
std::map <std::string, bool> db::errors_pending;

db::wrapper::operator_with_error::operator_with_error(wrapper& _wrapper) : _wrapper(_wrapper) {}

sqlite::database_binder db::wrapper::operator_with_error::operator <<(sqlite::str_ref query) const {
	return _wrapper << query;
}

sqlite::database_binder db::wrapper::operator <<(sqlite::str_ref query) {
	try {
		// try to return the result of the original operator
		return database::operator <<(query);
	}
	catch (const sqlite::sqlite_exception& e) {
		// catch an exception if there is, log it
		// if there's a comment, see where we came from
		const size_t pos = query.find("--");
		std::string function;
		if (pos != std::string::npos) {
			function = query.substr(pos + 3).data();
			if (errors_pending[function]) {
				errors_pending[function] = false;
				std::cerr << fmt::format(
					"SQL errors occurred. Check logging/{0}/{1}/sql_logs.log for more info.\n",
					IS_CLI ? "cli" : "bot", MODE_NAME
				);
			}
		}
		sql_log(e, function);
	}
	return database::operator <<(";");
}

std::string db::line_comment(std::string_view comment) {
	return fmt::format(" -- {}", comment);
}

bool db::connection_successful() {
	const std::filesystem::path db_path(LOCATION);
	if (!std::filesystem::exists(db_path)) {
		std::filesystem::create_directory(db_path);
	}
	sql = sqlite::database(fmt::format("../database/{}.db", MODE_NAME));
	// for some reason if I try to assign an instance of wrapper it tells me that the operator= is deleted
	// but then why can I do this? I'm confused.
	try {
		sql.with_error << "SELECT 1 FROM jtc_vcs;"; // try to select a random value from jtc_vcs. If the table doesn't exist, we need to handle an exception.
	}
	catch (const std::exception& e) { // wait till the user finds out we're gonna crash their shi 🔥
		log(fmt::format("Error connecting to the database: {}", e.what()));
		return false;
	}
	return true;
}
