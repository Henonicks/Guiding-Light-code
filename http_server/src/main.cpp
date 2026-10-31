#include "http_server/topgg_listener.hpp"

int main(int const argc, char* argv[]) {
	exec_subcommands(argc, argv);
	cfg::read_config("../../Guiding_Light_Config/config.hfg");
	configure_listener("../config.hfg");
	cfg::init_logs();

	topgg_cluster = new dpp::cluster();
	gor_client = new gor::poker(GL_ON_RAILS_TOKEN, topgg_cluster);

	dpptgg::listener listener(TOPGG_WEBHOOK_LISTEN_IP, TOPGG_WEBHOOK_LISTEN_PORT, secrets,
		send_vote_info, std::bind_back(serve_random_request, "../resources/"),
		topgg_cluster
	);

	topgg_cluster->on_log(bot_log);
	listener.on_log(topgg_log);

	topgg_cluster->start(dpp::st_return);

	if (!db::test_conn(std::format("../../database/{}.db", MODE_NAME))) {
		std::cerr << "Connection to database failed! Vote handling is impossible to do, so quitting.\n";
		explode(f_failure);
	}

	std::unique_lock L(bomb_mutex);
	bomb_cv.wait(L, [] { return last_signal != 0 || ready_to_explode; });
	handle_signal(last_signal);
	return 0;
}
