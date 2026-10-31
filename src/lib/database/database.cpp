#include "guiding_light/database/database.hpp"

#include "guiding_light/internal/cfg.hpp"
#include "guiding_light/internal/config_values.hpp"

#ifdef GL_USE_LOCAL_DB

db::wrapper::operator_with_error::operator_with_error(wrapper& _wrapper) : _wrapper(_wrapper) {}

sqlite::database_binder db::wrapper::operator_with_error::operator <<(sqlite::str_ref query) const {
	auto result = _wrapper << query;
	if (result.sql().size() <= 1) {
		throw std::exception();
	}
	return result;
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
		// catch an exception if there is one; log it
		// if there's a comment, see where we came from
		std::string function;
		if (pos != std::string::npos && pos < _query.size() - 2) {
			pos = _query.find_first_not_of(' ', pos + 2);
			if (pos != std::string::npos) {
				function = _query.substr(pos).data();
				if (errors_pending.contains(function)) {
					sql_log(function);
					std::cerr << std::format(
						"SQL errors occurred. Check logging/{0}/{1}/sql_logs.log for more info.\n",
						IS_CLI ? "cli" : "bot", MODE_NAME
					);
				}
				errors_pending.erase(function);
			}
		}
		sql_log(e, function);
	}
	catch (const std::exception& e) {
		std::string function;
		if (pos != std::string::npos && pos < _query.size() - 2) {
			pos = _query.find_first_not_of(' ', pos + 2);
			if (pos != std::string::npos) {
				function = _query.substr(pos).data();
				if (errors_pending.contains(function)) {
					sql_log(function);
					std::cerr << std::format(
						"SQL errors occurred. Check logging/{0}/{1}/sql_logs.log for more info.\n",
						IS_CLI ? "cli" : "bot", MODE_NAME
					);
				}
				errors_pending.erase(function);
			}
		}
		sql_log(e, function);
	}
	return database::operator <<(";");
}

[[nodiscard]] bool db::is_error(sqlite::database_binder& binder) {
	try {
		return binder.sql().size() <= 1;
	}
	catch (...) {
		return true;
	}
}

[[nodiscard]] bool db::is_error(sqlite::database_binder&& binder) {
	try {
		return binder.sql().size() <= 1;
	}
	catch (...) {
		return true;
	}
}

#endif

bool db::test_conn(const std::string_view path) {
	const std::filesystem::path db_path(LOCATION);
	if (!std::filesystem::exists(db_path)) {
		std::filesystem::create_directory(db_path);
	}
#ifdef GL_USE_LOCAL_DB
	sql = sqlite::database(path.data());
	// for some reason if I try to assign an instance of wrapper it tells me that the operator= is deleted
	// but then why can I do this? I'm confused.
	try {
		sql.with_error << "SELECT 1 FROM jtc_vcs;";
		// try to select a random value from jtc_vcs.
		// If the table doesn't exist, we need to handle an exception.

		sql << "PRAGMA journal_mode=WAL;";
		sql << "PRAGMA busy_timeout=1000;";
		return true;
	}
	catch (const std::exception& e) { // wait till the user finds out we're gonna crash their shi 🔥
		log(std::format("Error connecting to the database: {}", e.what()));
		return false;
	}
#else
	const auto callback = gor_client->poke <void>("test_conn", dpp::m_get).sync_wait();

	if (!callback.has_value()) {
		log(std::format("Error connecting to the database! Status: {0} Body: {1}", callback.error().status, callback.error().body));
		return false;
	}
	return true;
#endif
}
