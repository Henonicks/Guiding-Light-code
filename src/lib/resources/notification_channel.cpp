#include "guiding_light/resources/notification_channel.hpp"

std::string notification_channel_t::insert() const {
	return this->channel_id.str() + ' ' + guild_id.str();
}

void fill_from_json_impl(notification_channel_t& res, const nlohmann::json& json) {
	if (json.contains("channel")) {
		res.channel_id = json["channel"];
	}
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("source")) {
		res.source = json["source"];
	}
}

nlohmann::json to_json_impl(const notification_channel_t& notification_channel) {
	nlohmann::json res = nlohmann::json::object();
	res["channel"] = notification_channel.channel_id;
	res["guild"] = notification_channel.guild_id;
	res["source"] = notification_channel.source;
	return res;
}

void fill_from_json_impl(notification_channel_t::keep_list_t& res, const nlohmann::json& json) {
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("channels")) {
		res.channel_ids = json["channels"];
	}
}

nlohmann::json to_json_impl(const notification_channel_t::keep_list_t& bulk_list) {
	nlohmann::json res = nlohmann::json::object();
	res["guild"] = bulk_list.guild_id;
	res["channels"] = bulk_list.channel_ids;
	return res;
}
