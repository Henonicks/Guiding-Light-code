#ifndef CHANNEL_H
#define CHANNEL_H

#include "guiding_light/resources/base_resource.hpp"

#include "guiding_light/resources/jtc_vc.hpp"
#include "guiding_light/resources/notification_channel.hpp"
#include "guiding_light/resources/temp_vc.hpp"
#include "guiding_light/resources/ticket.hpp"

struct channel_t : base_resource_t {
	using base_resource_t::base_resource_t;

	static constexpr std::string_view table_name = "channels";

	dpp::snowflake channel_id;
	std::string resource;

	using possible_types = std::variant <jtc_vc_t, notification_channel_t, temp_vc_t, ticket_t>;

	channel_t() = default;
	channel_t(const dpp::snowflake channel_id, const std::string& resource) : channel_id(channel_id), resource(resource) {}

	[[nodiscard]] static constexpr std::string where(const dpp::snowflake channel_id) {
		return "channel=" + channel_id.str();
	}
};

void fill_from_json_impl(channel_t& res, const nlohmann::json& json);

#endif
