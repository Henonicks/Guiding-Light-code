#include "guiding_light/resources/temp_ping_blocker.hpp"

void fill_from_json_impl(temp_ping_blocker_t& res, const nlohmann::json& json) {
	if (json.contains("user")) {
		res.user_id = json["user"];
	}
}

nlohmann::json to_json_impl(const temp_ping_blocker_t& temp_ping_blocker) {
	nlohmann::json res = nlohmann::json::object();
	res["user"] = temp_ping_blocker.user_id;
	return res;
}
