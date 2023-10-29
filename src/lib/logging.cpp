#include "logging.h"

void log(std::string_view message) {
    my_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
}
