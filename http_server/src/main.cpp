#include "http_server/topgg_listener.hpp"

dpp::cluster* server_cluster;
std::string TOPGG_WEBHOOK_LISTEN_IP;
uint16_t TOPGG_WEBHOOK_LISTEN_PORT;
std::string TOPGG_BOT_WEBHOOK_SECRET;
std::string TOPGG_SERVER_WEBHOOK_SECRET;
dpp::webhook TOPGG_WEBHOOK;

enum response_codes : uint16_t {
	OK = 200,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	NOT_FOUND = 404,
	INTERNAL_SERVER_ERROR = 500,
};

std::ofstream my_logs;
std::ofstream dpp_logs;

void log(std::string_view const& message) {
	my_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), message) << std::endl;
}

std::string cleanup_request_path(std::string_view const path) {
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

std::vector <std::string> get_path_points(std::string_view const path) {
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

void send_vote_info(dpptgg::topgg_request const& request) { // The bot will pick the message up
	dpptgg::vote_types const vote_type = request.vote_type;
	log(fmt::format("top.gg vote with type {}", static_cast <int16_t>(vote_type)));
	int16_t const weight = std::max(request.vote_weight, static_cast <uint8_t>(1));
	log(fmt::format("The vote weight is {}", weight));
	dpp::snowflake const user_id = request.user_platform_id;
	log(fmt::format("The user ID is {}", user_id));
	server_cluster->execute_webhook(TOPGG_WEBHOOK, dpp::message(TOPGG_WEBHOOK.channel_id,
		fmt::format("{0} {1}", dpp::utility::user_mention(user_id), weight))
	);
}

std::string to_lower(std::string str) {
	for (char& x : str) {
		x = std::tolower(x);
	}
	return str;
}

int main() {
	henifig::config_t const config("../config.hfg");
	TOPGG_WEBHOOK_LISTEN_IP = config["TOPGG_WEBHOOK_LISTEN_IP"].get <std::string>();
	TOPGG_WEBHOOK_LISTEN_PORT = config["TOPGG_WEBHOOK_LISTEN_PORT"];
	TOPGG_BOT_WEBHOOK_SECRET = config["TOPGG_BOT_WEBHOOK_SECRET"].get <std::string>();
	TOPGG_SERVER_WEBHOOK_SECRET = config["TOPGG_SERVER_WEBHOOK_SECRET"].get <std::string>();
	TOPGG_WEBHOOK = dpp::webhook(config["TOPGG_WEBHOOK_LINK"].get <std::string>());

	server_cluster = new dpp::cluster();

	dpptgg::listener listener(TOPGG_WEBHOOK_LISTEN_IP, TOPGG_WEBHOOK_LISTEN_PORT, TOPGG_BOT_WEBHOOK_SECRET, TOPGG_SERVER_WEBHOOK_SECRET,
		[](dpptgg::topgg_request const& request) {
			send_vote_info(request);
		},
		[](dpptgg::non_topgg_request const& request) {
			std::string clean_path = cleanup_request_path(request.request->get_path());
			clean_path = clean_path.substr(0, clean_path.find('?'));
			clean_path = "../resources/" + clean_path;
			clean_path = to_lower(clean_path);
			auto const requested_path = std::filesystem::path(clean_path);
			for (auto const& x : std::filesystem::recursive_directory_iterator("../resources")) {
				std::string const curr = to_lower(x.path().string());
				if (std::filesystem::path(curr) == requested_path) {
					if (x.is_directory()) {
						std::filesystem::path const redirect_path = x.path() / ".redirect.hfg";
						if (std::filesystem::is_regular_file(redirect_path)) {
							try {
								henifig::config_t const redirect_config(redirect_path.string());
								request.request->set_response_header(
									"Location", redirect_config["Location"]
								).set_status(MOVED_PERMANENTLY);
							}
							catch (...) {
								request.request->set_status(INTERNAL_SERVER_ERROR);
							}
							return;
						}
						request.request->set_status(NOT_FOUND);
					}
					else {
						request.request->set_response_body(
							dpp::utility::read_file(x.path().string())
						).set_status(OK);
					}
					break;
				}
			}
		}, server_cluster
	);

	if (!std::filesystem::exists("../logging")) {
		std::filesystem::create_directory("../logging");
	}

	server_cluster->on_log([](dpp::log_t const& log) {
		dpp_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), log.message) << std::endl;
	});

	my_logs.open("../logging/my_logs.log");
	dpp_logs.open("../logging/dpp_logs.log");

	listener.on_log([](dpp::log_t const& log_arg) {
		log(log_arg.message);
	});

	listener.start(dpp::st_wait);

	return 0;
}
