#include "guiding_light/topgg.hpp"

#include "guiding_light/database.hpp"
#include "guiding_light/logging.hpp"
#include "guiding_light/helpers/hmac_sha256.hpp"

std::string cleanup_request_path(const std::string_view path) {
	std::string res = "/";
	for (const char x : path) {
		if (res.back() != '/' || (res.back() == '/' && x != '/')) {
			res += x;
		}
	}
	size_t const new_size = res.size() - 1 - (res.back() == '/');
	res = res.substr(1, new_size);
	return res;
}

std::vector <std::string> get_path_points(const std::string_view path) {
	std::vector <std::string> res;
	std::string curr_point;
	for (const char x : path) {
		if (x == '/') {
			res.push_back(curr_point);
			curr_point.clear();
		}
		else {
			curr_point += x;
		}
	}
	if (!curr_point.empty()) {
		res.push_back(curr_point);
	}
	return res;
}

void topgg::handle_request_if_topgg(dpp::http_server_request* request) {
	const std::vector <std::string> path_points = get_path_points(cleanup_request_path(request->get_path()));
	if (path_points.size() != 3 || path_points[0] != "guidinglight" || path_points[1] != "api" || path_points[2] != "topgg-votes") {
		request->set_response_body("This page doesn't exist.");
		request->set_status(dpp::err_not_found);
		return;
	}
	const std::string signature = request->get_header("x-topgg-signature");
	std::string timestamp, v1;
	bool error{};
	if (!signature.starts_with("t=") || !request->get_header("content-type").starts_with("application/json;")) {
		error = true;
	}
	else {
		size_t i = 2;
		for (; i < signature.size(); i++) {
			if (signature[i] >= '0' && signature[i] <= '9') {
				timestamp += signature[i];
			}
			else {
				error |= signature[i] != ',';
				break;
			}
		}
		if (signature.size() - (timestamp.size() + 2) <= 4) {
			error = true;
		}
		else for (i += 4; i < signature.size(); i++) {
			v1 += signature[i];
		}
	}
	if (error) { // The format of the headers is wrong, if they even exist to begin with. That's not top.gg then.
		request->set_status(dpp::err_forbidden);
		return;
	}
	nlohmann::json request_json;
	try {
		std::stringstream(request->get_request_body()) >> request_json;
	}
	catch (const nlohmann::detail::parse_error&) { // Couldn't parse the request body json
		request->set_status(dpp::err_forbidden);
		return;
	}
	const std::string hmac_sha256 = get_hmac_sha256_hex(TOPGG_WEBHOOK_SECRET, timestamp + '.' + request->get_request_body());
	if (hmac_sha256 != v1) {
		request->set_status(dpp::err_forbidden);
	}
	request->set_status(send_vote_info(request_json));
}

int16_t topgg::send_vote_info(const nlohmann::json& info_json) {
	// The json format can be found here: https://docs.top.gg/docs/API/v1/webhooks/
	const std::string type = info_json["type"];
	log(fmt::format("top.gg vote with type {}", type));
	if (type != "vote.create" && type != "webhook.test") {
		log("That one's unacceptable.");
		return dpp::err_unprocessable_content;
	}
	const int16_t weight = type == "vote.create" ? info_json["weight"].get <int16_t>() : 1;
	log(fmt::format("The vote weight is {}", weight));
	const dpp::snowflake user_id = info_json["data"]["user"]["platform_id"].get <std::string>();
	log(fmt::format("The user ID is {}", user_id));
	server_cluster->execute_webhook(TOPGG_WEBHOOK, dpp::message(TOPGG_WEBHOOK_CHANNEL_ID,
		fmt::format("{0} {1}", dpp::utility::user_mention(user_id), weight)),
		false, 0, "", error_callback
	);
	return 204;
}

bool topgg::vote(const dpp::snowflake& user_id, const int8_t weight) {
	log(fmt::format("User {} just voted, hell yeah!", user_id));
	bool failure = false;
	const dpp::snowflake& guild_id = guild_choices[user_id];
	if (dpp::find_guild(guild_id) == nullptr) {
		log("But they haven't selected a guild!");
		failure = true;
		return failure;
	}
	const int8_t old_allowed = jtc::count_allowed_jtcs(guild_id);
	guild_votes_amount[guild_id] += weight;
	const int8_t new_allowed = jtc::count_allowed_jtcs(guild_id);
	log(fmt::format("Therefore, the guild {0} now has {1} votes!", guild_id, guild_votes_amount[guild_id]));
	// The bot that's getting voted for gets a bonus vote if it's a weekend for every vote.
	db::sql << "DELETE FROM topgg_guild_votes_amount WHERE guild_id=?;" << guild_id.str();
	db::sql << "INSERT INTO topgg_guild_votes_amount VALUES (?, ?);" << guild_id.str() << guild_votes_amount[guild_id];
	const dpp::snowflake& channel_id = topgg_notifications[guild_id];
	if (!channel_id.empty()) {
		const int next_req = jtc::get_next_lvl_req(guild_id);
		bot->message_create(dpp::message(channel_id,
			fmt::format("<@{0}> has voted.{1} {2}{3}",
				user_id,
				(weight == 2 ? " A bonus point has been granted as today is a weekend!" : ""),
				(next_req ? fmt::format("Current progress: {0}/{1}. ", guild_votes_amount[guild_id], jtc::get_next_lvl_req(guild_id)) : ""),
				(old_allowed < new_allowed ? fmt::format("The new JTC limit in this server is now {}.", new_allowed) : "")
			)
		), error_callback);
	}
	return failure;
}

int8_t topgg::jtc::count_allowed_jtcs(const dpp::snowflake& guild_id) {
	const int votes = guild_votes_amount[guild_id];
	int8_t i = 1;
	for (;i < cast <int8_t>(votes_leveling.size()); i++) {
		if (votes_leveling[i] > votes) {
			break;
		}
	}
	return i;
}

int topgg::jtc::get_next_lvl_req(const dpp::snowflake& guild_id) {
	const int8_t allowed_jtcs = count_allowed_jtcs(guild_id);
	return allowed_jtcs >= cast <int8_t>(votes_leveling.size()) ? 0 : votes_leveling[allowed_jtcs];
}

