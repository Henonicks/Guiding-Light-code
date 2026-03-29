#include <guiding_light/temp_vc.hpp>

bool temp_vc::exists() const {
	return this->id != 0;
}

restrictions_list_t* get_restrictions_list(const restrictions_types rest_type) {
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
