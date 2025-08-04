#ifndef DATABASE_H
#define DATABASE_H

#include "configuration.h"
#include "sqlite_modern_cpp.h"
#include "sqlite_modern_cpp/log.h"

namespace db {
	using TINYINT = int16_t; // bro why is there no int8_t which isn't a char in c++ twin icl ts pmo lwk 💔🥀
	using SMALLINT = int16_t;
	using MEDIUMINT = int;
	using BIGINT = unsigned long long;
	
	extern sqlite::database sql;
	bool connection_successful();
}

#endif
