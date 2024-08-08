#ifndef GUIDING_LIGHT_H
#define GUIDING_LIGHT_H

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>

// Place any forward declarations here
dpp::command_completion_event_t error_callback([](const dpp::confirmation_callback_t& callback) {
	if (callback.is_error()) {

	}
});

#endif
