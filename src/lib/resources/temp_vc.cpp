#include "guiding_light/resources/temp_vc.hpp"

#include "guiding_light/json/json_interface.hpp"

std::string temp_vc_t::insert() const {
	return this->channel_id.str() + ' ' + this->guild_id.str() + ' ' + this->creator_id.str() + ' ' + this->parent_id.str() + ' ' + std::to_string(this->name_timer);
}

void fill_from_json_impl(temp_vc_t& res, const nlohmann::json& json) {
	if (json.contains("channel")) {
		res.channel_id = json["channel"];
	}
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("creator")) {
		res.creator_id = json["creator"];
	}
	if (json.contains("parent")) {
		res.parent_id = json["parent"];
	}
	if (json.contains("name_timer")) {
		res.name_timer = json["name_timer"];
	}
}

nlohmann::json to_json_impl(const temp_vc_t& temp_vc) {
	nlohmann::json res = nlohmann::json::object();
	res["channel"] = temp_vc.channel_id;
	res["guild"] = temp_vc.guild_id;
	res["creator"] = temp_vc.creator_id;
	res["parent"] = temp_vc.parent_id;
	res["name_timer"] = temp_vc.name_timer;
	return res;
}

void fill_from_json_impl(temp_vc_t::keep_list_t& res, const nlohmann::json& json) {
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("channels")) {
		res.channel_ids = json["channels"];
	}
}

nlohmann::json to_json_impl(const temp_vc_t::keep_list_t& bulk_list) {
	nlohmann::json res = nlohmann::json::object();
	res["guild"] = bulk_list.guild_id;
	res["channels"] = bulk_list.channel_ids;
	return res;
}

void fill_from_json_impl(temp_vc_t::count_t& res, const nlohmann::json& json) {
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("count")) {
		res.count = json["count"];
	}
}

nlohmann::json to_json_impl(const temp_vc_t::count_t& count) {
	nlohmann::json res = nlohmann::json::object();
	res["guild"] = count.guild_id;
	res["votes"] = count.count;
	return res;
}

restriction_list_t* get_restrictions_list(const restrictions_types rest_type) {
	switch (rest_type) {
	case RRT_BLOCKLIST:
		return &banned;
	// case LT_MUTELIST:
	default:
		return &muted;
	}
}

dpp::permission get_restriction_permissions(const restrictions_types rest_type) {
	switch (rest_type) {
	case RRT_BLOCKLIST:
		return dpp::p_view_channel | dpp::p_connect;
	// case RRT_MUTELIST:
	default:
		return dpp::p_speak;
	}
}
