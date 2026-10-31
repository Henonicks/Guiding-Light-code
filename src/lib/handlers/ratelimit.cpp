#include "guiding_light/handlers/ratelimit.hpp"

#include "guiding_light/database/database.hpp"
#include "guiding_light/resources/temp_vc.hpp"

dpp::coroutine <> make_channel_rename(const dpp::snowflake channel_id) {
	temp_vc_t vc = co_await db::get <temp_vc_t>(channel_id);
	vc.name_timer = dpp::utility::time_f() + CHANNEL_RENAME_RATELIMIT_SECONDS;
	co_await db::update <temp_vc_t>(vc);
}

dpp::coroutine <bool> early_for_channel_rename(const dpp::snowflake channel_id) {
	co_return early_for_channel_rename(co_await db::get <temp_vc_t>(channel_id));
}

bool early_for_channel_rename(const temp_vc_t& channel) {
	return channel.name_timer - dpp::utility::time_f() > 0;
}
