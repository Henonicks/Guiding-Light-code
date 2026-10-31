#include "guiding_light/resources/topgg_guild_choice.hpp"

std::string topgg_guild_choice_t::insert() const {
	return this->user_id.str() + ' ' + this->guild_id.str();
}

void fill_from_json_impl(topgg_guild_choice_t& res, const nlohmann::json& json) {
	if (json.contains("user")) {
		res.user_id = json["user"];
	}
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
}

nlohmann::json to_json_impl(const topgg_guild_choice_t& topgg_guild_choice) {
	nlohmann::json res = nlohmann::json::object();
	res["user"] = topgg_guild_choice.user_id;
	res["guild"] = topgg_guild_choice.guild_id;
	return res;
}
