#ifndef LOGGING_H
#define LOGGING_H

#include "fmt/format.h"
#include "dpp/utility.h"
#include <fstream>
#include "configuration.h"

void bot_log(const dpp::log_t& log, dpp::cluster& bot);
void log(std::string_view message);
void guild_log(std::string_view message);
void error_callback(const dpp::confirmation_callback_t& callback);

#endif
