#ifndef LOGGING_H
#define LOGGING_H

#include "fmt/format.h"
#include "dpp/utility.h"
#include <fstream>
#include "configuration.h"

void log(std::string_view message);

#endif
