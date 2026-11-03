#pragma once

#include "guiding_light/helpers/format.tpp"
#include "guiding_light/internal/config_values.hpp"
#include "guiding_light/internal/logging.hpp"
#include "guiding_light/database/database.hpp"
#include "guiding_light/internal/cfg.hpp"
#include "guiding_light/internal/signal.hpp"
#include "guiding_light/handlers/topgg.hpp"
#include "guiding_light/resources/topgg_guild_vote_amount.hpp"
#include "guiding_light/resources/topgg_guild_choice.hpp"

#include "dpptgg/dpptgg.hpp"

inline dpp::cluster* topgg_cluster;
inline dpp::webhook DISCORD_TOPGG_WEBHOOK;

enum response_codes : uint16_t {
	OK = 200,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	NOT_FOUND = 404,
	INTERNAL_SERVER_ERROR = 500,
};

inline std::string TOPGG_WEBHOOK_LISTEN_IP;
inline uint16_t TOPGG_WEBHOOK_LISTEN_PORT;
inline std::map <std::string, std::vector <std::string>> secrets;

inline void configure_listener(std::string_view const path) {
	ENABLE_TOPGG = true;
	henifig::config_t const config(path);
	TOPGG_WEBHOOK_LISTEN_IP = config["TOPGG_WEBHOOK_LISTEN_IP"].get <std::string>();
	TOPGG_WEBHOOK_LISTEN_PORT = config["TOPGG_WEBHOOK_LISTEN_PORT"];
	secrets = cast <decltype(secrets)>(config["TOPGG_WEBHOOK_SECRETS"]);
	DISCORD_TOPGG_WEBHOOK = dpp::webhook(config["TOPGG_WEBHOOK_LINK"].get <std::string>());
}

inline std::string cleanup_request_path(std::string_view const path) {
	std::string res = "/";
	for (char const x : path) {
		if (res.back() != '/' || (res.back() == '/' && x != '/')) {
			res += x;
		}
	}
	size_t const new_size = res.size() - 1 - (res.back() == '/');
	res = res.substr(1, new_size);
	return res;
}

inline std::vector <std::string> get_path_points(std::string_view const path) {
	std::vector <std::string> res;
	std::string curr_point;
	for (char const x : path) {
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

inline dpp::job send_vote_info(dpptgg::topgg_request const& request) {
	dpptgg::vote_types const vote_type = request.vote_type;
	log(std::format("top.gg vote with type {}", cast <int16_t>(vote_type)));
	int16_t const weight = std::max(request.vote_weight, cast <uint8_t>(1));
	log(std::format("The vote weight is {}", weight));
	dpp::snowflake const user_id = request.user_platform_id;
	log(std::format("The user ID is {}", user_id));
	topgg_guild_choice_t const guild = co_await db::get <topgg_guild_choice_t>(user_id);
	if (guild.exists()) {
		topgg_guild_vote_amount_t curr_progress = co_await db::get <topgg_guild_vote_amount_t>(guild.guild_id);
		if (curr_progress.is_valid()) {
			curr_progress.votes += weight;
			if (curr_progress.exists()) {
				curr_progress = co_await db::update(curr_progress);
			}
			else {
				curr_progress.guild_id = guild.guild_id;
				curr_progress = co_await db::create(curr_progress);
			}
#ifdef GL_BOT_ENVIRONMENT
			if (curr_progress.is_valid()) {
				co_await topgg::vote(user_id, cast <int8_t>(weight), guild.guild_id);
			}
#endif
		}
	}
	topgg_cluster->execute_webhook(DISCORD_TOPGG_WEBHOOK, dpp::message(DISCORD_TOPGG_WEBHOOK.channel_id,
		std::format("{0} {1}", dpp::utility::user_mention(user_id), weight))
	);
}

inline std::string to_lower(std::string str) {
	for (char& x : str) {
		x = std::tolower(x);
	}
	return str;
}

inline void serve_random_request(dpptgg::non_topgg_request const& request, const std::string_view resources_path) {
	if (request.status != dpptgg::sis_no_secrets) { // If the problem isn't a lack of secrets, that means that someone random tried to hit a top.gg endpoint
		return;
	}
	try {
		std::string clean_path = cleanup_request_path(request.request->get_path());
		clean_path = clean_path.substr(0, clean_path.find('?'));
		clean_path = to_lower(clean_path);
		log(std::format("Serving non-top.gg request to {}", clean_path));
		clean_path = std::string(resources_path) + clean_path;
		auto const requested_path = std::filesystem::path(clean_path);
		for (auto const& x : std::filesystem::recursive_directory_iterator(resources_path)) {
			std::string const curr = to_lower(x.path().string());
			if (std::filesystem::path(curr) == requested_path) {
				if (x.is_directory()) {
					log("The request hit a directory.");
					std::filesystem::path const redirect_path = x.path() / ".redirect.hfg";
					if (std::filesystem::is_regular_file(redirect_path)) {
						try {
							log("Trying to redirect.");
							henifig::config_t const redirect_config(redirect_path.string());
							request.request->set_response_header(
								"Location", redirect_config["Location"]
							).set_status(MOVED_PERMANENTLY);
						}
						catch (henifig::parse_exception const& e) {
							if (e.what() == henifig::error_messages[henifig::FILE_OPEN_FAILED]) {
								log("But there's no redirect config in this directory. This shouldn't be possible though!");
								request.request->set_status(NOT_FOUND);
							}
							else {
								log("The redirect config could not be parsed!");
								request.request->set_status(INTERNAL_SERVER_ERROR);
							}
						}
						catch (henifig::retrieval_exception const&) {
							log("Error accessing the redirect path!");
							request.request->set_status(INTERNAL_SERVER_ERROR);
						}
						catch (std::exception const& e) {
							log(std::format("Error redirecting! What: {}", e.what()));
							request.request->set_status(INTERNAL_SERVER_ERROR);
						}
						return;
					}
					request.request->set_status(NOT_FOUND);
				}
				else {
					log("The request hit a file.");
					try {
						request.request->set_response_body(
							dpp::utility::read_file(x.path().string())
						).set_status(OK);
						log("File read, served.");
					}
					catch (std::exception const& e) {
						log(std::format("Error reading the file! What: {}", e.what()));
					}
				}
				return;
			}
		}
		log("The request hit nothing.");
		request.request->set_status(NOT_FOUND);
	}
	catch (std::exception const& e) {
		log(std::format("Uncaught exception while serving! What: {}", e.what()));
		request.request->set_status(INTERNAL_SERVER_ERROR);
	}
}
