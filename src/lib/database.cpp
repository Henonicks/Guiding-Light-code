#include "database.h"

sqlite::database db::sql{""};

bool db::connection_successful() {
	sqlite::error_log([](sqlite::sqlite_exception& e) {
		sql_log(fmt::format("Code: {0}, error: {1}, query: {2}", e.get_code(), e.what(), e.get_sql()));
	});
	sql = sqlite::database(fmt::format("../database/{}.db", LOGS_SUFFIX));
	try {
		sql << "SELECT 1 FROM jtc_vcs;"; // try to select a random value from jtc_vcs. If the table doesn't exist, we need to handle an exception.
	}
	catch (std::exception& e) { // wait till the user finds out we're gonna crash their shi 🔥
		log(fmt::format("Error connecting to the database: {}", e.what()));
		return false;
	}
	return true;
}
