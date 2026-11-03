#include "guiding_light/resources/noguild_reminder_blocker.hpp"

std::string noguild_reminder_blocker_t::insert() const {
	return user_id.str();
}

void fill_from_json_impl(noguild_reminder_blocker_t& res, const nlohmann::json& json) {
	if (json.contains("user")) {
		res.user_id = json["user"];
	}
}

nlohmann::json to_json_impl(const noguild_reminder_blocker_t& noguild_reminder_blocker) {
	nlohmann::json res = nlohmann::json::object();
	res["user"] = noguild_reminder_blocker.user_id;
	return res;
}
