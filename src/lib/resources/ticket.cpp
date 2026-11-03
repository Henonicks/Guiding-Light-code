#include "guiding_light/resources/ticket.hpp"

std::string ticket_t::insert() const {
	return this->user_id.str() + ' ' + this->channel_id.str() + ' ' + this->dm_channel_id.str();
}

void fill_from_json_impl(ticket_t& res, const nlohmann::json& json) {
	if (json.contains("user")) {
		res.user_id = json["user"];
	}
	if (json.contains("channel")) {
		res.channel_id = json["channel"];
	}
	if (json.contains("dm_channel")) {
		res.dm_channel_id = json["dm_channel"];
	}
}

nlohmann::json to_json_impl(const ticket_t& ticket) {
	nlohmann::json res = nlohmann::json::object();
	res["user"] = ticket.user_id;
	res["channel"] = ticket.channel_id;
	res["dm_channel"] = ticket.dm_channel_id;
	return res;
}

void fill_from_json_impl(ticket_t::keep_list_t& res, const nlohmann::json& json) {
	if (json.contains("channels")) {
		res.channel_ids = json["channels"];
	}
}

nlohmann::json to_json_impl(const ticket_t::keep_list_t& bulk_list) {
	nlohmann::json res = nlohmann::json::object();
	res["channels"] = bulk_list.channel_ids;
	return res;
}

bool operator <(const ticket_t& t1, const ticket_t& t2) {
	if (t1.user_id == t2.user_id) {
		if (t1.channel_id == t2.channel_id) {
			return t1.dm_channel_id < t2.dm_channel_id;
		}
		return t1.channel_id < t2.channel_id;
	}
	return t1.user_id < t2.user_id;
}
