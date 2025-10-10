#include "guiding_light/database.hpp"
#include "guiding_light/cfg.hpp"

db::wrapper::operator_with_error::operator_with_error(wrapper& _wrapper) : _wrapper(_wrapper) {}

sqlite::database_binder db::wrapper::operator_with_error::operator <<(sqlite::str_ref query) const {
	return _wrapper.database::operator <<(query);
}

sqlite::database_binder db::wrapper::operator <<(sqlite::str_ref _query) {
	size_t pos{_query.size()};
	bool hanging_quote{}, hanging_apostrophe{};
	for (size_t i = 0; i < _query.size(); i++) {
		if (_query[i] == '-' && (i < _query.size() - 1 && _query[i + 1] == '-') &&
		!hanging_quote && !hanging_apostrophe) {
			pos = i;
			break;
		}
		if (_query[i] == '"' && !hanging_apostrophe) {
			hanging_quote = !hanging_quote;
		}
		else if (_query[i] == '\'' && !hanging_quote) {
			hanging_apostrophe = !hanging_apostrophe;
		}
	}
	const std::string_view query = _query.substr(0, pos);
	try {
		// try to return the result of the original operator
		return database::operator <<(query);
	}
	catch (const sqlite::sqlite_exception& e) {
		// catch an exception if there is, log it
		// if there's a comment, see where we came from
		std::string function;
		if (pos != std::string::npos) {
			function = _query.substr(pos + 3).data();
			if (errors_pending[function]) {
				errors_pending[function] = false;
				std::cerr << fmt::format(
					"SQL errors occurred. Check logging/{0}/{1}/sql_logs.log for more info.\n",
					IS_CLI ? "cli" : "bot", MODE_NAME
				);
			}
		}
		sql_log(e, function);
		return database::operator <<(";");
	}
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
		sql.with_error << "SELECT 1 FROM jtc_vcs;";
		// try to select a random value from jtc_vcs.
		// If the table doesn't exist, we need to handle an exception.
		return true;
	}
	catch (const std::exception& e) { // wait till the user finds out we're gonna crash their shi 🔥
		log(fmt::format("Error connecting to the database: {}", e.what()));
		return false;
	}
}
