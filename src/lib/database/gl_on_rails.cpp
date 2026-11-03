#include "guiding_light/database/gl_on_rails.hpp"

gor::poker::poker(const std::string_view tkn, dpp::cluster* const cluster) : token("Bearer " + std::string(tkn)) {
	if (cluster == nullptr) {
		this->poker_cluster = new dpp::cluster();
	}
	else {
		this->poker_cluster = cluster;
		this->is_user_cluster = true;
	}
}

gor::poker::~poker() {
	if (!is_user_cluster) {
		delete this->poker_cluster;
	}
}
