#ifndef LOGGING_H
#define LOGGING_H

#include "fmt/format.h"
#include "dpp/utility.h"
#include <fstream>
#include "configuration.h"
#include "database.h"
#include "sqlite_modern_cpp.h"

void bot_log(const dpp::log_t& log);
void log(std::string_view message);
void guild_log(std::string_view message);
void sql_log(sqlite::sqlite_exception e);
void error_callback(const dpp::confirmation_callback_t& callback);

#endif
