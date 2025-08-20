#ifndef DATABASE_H
#define DATABASE_H

#include <map>
#include <set>
#include "sqlite_modern_cpp.h"

namespace db {
	using TINYINT = int16_t; // bro why is there no int8_t which isn't a char in c++ twin icl ts pmo lwk 💔🥀
	using SMALLINT = int16_t;
	using MEDIUMINT = int; // 24-bit ints also don't exist i think 💔🥀
	using BIGINT = unsigned long long;
	// SQL types that I use

	extern std::string_view LOCATION;
	extern std::string_view SELECT_LOCATION;
	extern const std::set <std::string> table_names;

	class wrapper : public sqlite::database {
	public:
		class operator_with_error {
		public:
			wrapper& _wrapper;
			operator_with_error() = delete; // the whole point of this class is to have a ref to the parent
			explicit operator_with_error(wrapper&);
			sqlite::database_binder operator <<(sqlite::str_ref query) const;
		};
		using database::database;
		using database::operator=;

		operator_with_error with_error{*this};

		/**
		 * @brief Handle an SQL query and catch any exceptions on failure.
		 * @param _query The SQL query to handle.
		 * @return I've honestly got no clue what a database_binder is
		 */
		sqlite::database_binder operator <<(sqlite::str_ref _query);
	};

	// A map with true/false values representing the existence of potential
	// pending errors, caught during an SQL query in functions.
	extern std::map <std::string, bool> errors_pending;

	std::string line_comment(std::string_view comment);

	extern wrapper sql;

	/**
	 * @brief Tries to connect to the database and reports whether the connection was successful.
	 * @return True if the connection was successful, false otherwise.
	 */
	bool connection_successful();
}

#endif
