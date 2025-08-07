#include "database.h"

sqlite::database db::sql{""};
const std::vector <const char*> db::table_names =
{"jtc_vcs", "temp_vc_notifications", "jtc_default_values", "no_temp_ping", "topgg_guild_choices", "topgg_guild_votes_amount", "no_noguild_reminder", "topgg_notifications", "tickets", "temp_vcs"};

bool db::connection_successful() {
	sqlite::error_log([](sqlite::sqlite_exception& e) {
		sql_log(e);
	});
	sql = sqlite::database(fmt::format("../database/{}.db", MODE_NAME));
	try {
		sql << "SELECT 1 FROM jtc_vcs;"; // try to select a random value from jtc_vcs. If the table doesn't exist, we need to handle an exception.
	}
	catch (std::exception& e) { // wait till the user finds out we're gonna crash their shi 🔥
		log(fmt::format("Error connecting to the database: {}", e.what()));
		return false;
	}
	return true;
}
