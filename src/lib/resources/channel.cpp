#include "guiding_light/resources/channel.hpp"

void fill_from_json_impl(channel_t& res, const nlohmann::json& json) {
	if (json.contains("channel")) {
		res.channel_id = json["channel"];
	}
	if (json.contains("resource")) {
		res.resource = json["resource"];
	}
}
