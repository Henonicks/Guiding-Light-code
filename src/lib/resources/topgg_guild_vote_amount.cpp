#include "guiding_light/resources/topgg_guild_vote_amount.hpp"

std::string topgg_guild_vote_amount_t::insert() const {
    return this->guild_id.str() + ' ' + std::to_string(votes);
}

void fill_from_json_impl(topgg_guild_vote_amount_t& res, const nlohmann::json& json) {
	if (json.contains("guild")) {
		res.guild_id = json["guild"];
	}
	if (json.contains("votes")) {
		res.votes = json["votes"];
	}
}

nlohmann::json to_json_impl(const topgg_guild_vote_amount_t& topgg_guild_vote_amount) {
	nlohmann::json res = nlohmann::json::object();
	res["guild"] = topgg_guild_vote_amount.guild_id;
	res["votes"] = topgg_guild_vote_amount.votes;
	return res;
}
