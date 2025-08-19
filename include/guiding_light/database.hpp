#ifndef DATABASE_H
#define DATABASE_H

#include <set>
#include "sqlite_modern_cpp.h"
#include "sqlite_modern_cpp/log.h"

namespace db {
	using TINYINT = int16_t; // bro why is there no int8_t which isn't a char in c++ twin icl ts pmo lwk 💔🥀
	using SMALLINT = int16_t;
	using MEDIUMINT = int;
	using BIGINT = unsigned long long;
	// SQL types that I use

	extern const std::set <std::string> table_names;

	extern sqlite::database sql;

	/**
	 * @brief Tries to connect to the database and reports whether the connection was successful.
	 * @return True if the connection was successful, false otherwise.
	 */
	bool connection_successful();
}

#endif
