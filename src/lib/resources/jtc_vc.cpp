#include "guiding_light/resources/jtc_vc.hpp"

std::string jtc_vc_t::insert() const {
	return this->channel_id.str() + ' ' + this->guild_id.str() + ' ' + this->name + ' ' + std::to_string(this->limit) + ' ' + std::to_string(this->bitrate);
}

void fill_from_json_impl(jtc_vc_t& res, const nlohmann::json& json) {
	if (json.contains("channel")) {
		res.channel_id = json["channel"];
	}
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("name")) {
		res.name = json["name"];
	}
	if (json.contains("vc_limit")) {
		res.limit = json["vc_limit"];
	}
	if (json.contains("bitrate")) {
		res.bitrate = json["bitrate"];
	}
}

nlohmann::json to_json_impl(const jtc_vc_t& jtc_vc) {
	nlohmann::json res = nlohmann::json::object();
	res["channel"] = jtc_vc.channel_id;
	res["guild"] = jtc_vc.guild_id;
	res["name"] = jtc_vc.name;
	res["vc_limit"] = jtc_vc.limit;
	res["bitrate"] = jtc_vc.bitrate;
	return res;
}

void fill_from_json_impl(jtc_vc_t::keep_list_t& res, const nlohmann::json& json) {
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("channels")) {
		res.channel_ids = json["channels"];
	}
}

nlohmann::json to_json_impl(const jtc_vc_t::keep_list_t& bulk_list) {
	nlohmann::json res = nlohmann::json::object();
	res["guild"] = bulk_list.guild_id;
	res["channels"] = bulk_list.channel_ids;
	return res;
}

void fill_from_json_impl(jtc_vc_t::count_t& res, const nlohmann::json& json) {
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("count")) {
		res.count = json["count"];
	}
}

nlohmann::json to_json_impl(const jtc_vc_t::count_t& count) {
	nlohmann::json res = nlohmann::json::object();
	res["guild"] = count.guild_id;
	res["votes"] = count.count;
	return res;
}
