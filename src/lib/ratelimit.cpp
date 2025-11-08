#include "guiding_light/ratelimit.hpp"

#include "guiding_light/database.hpp"

void add_channel_edit(const dpp::snowflake channel_id, const channel_edit_type edit_type) {
	channel_edits[edit_type][channel_id] = true;
	channel_edit_timers[edit_type][channel_id] = dpp::utility::time_f() + CHANNEL_RENAME_RATELIMIT_SECONDS;
	db::sql << "INSERT INTO channel_name_edit_timers VALUES (?, ?);" << channel_id.str() << channel_edit_timers[edit_type][channel_id];
	bot->start_timer([channel_id, edit_type](const dpp::timer& h) {
		remove_channel_edit(channel_id, edit_type);
		bot->stop_timer(h);
	}, CHANNEL_RENAME_RATELIMIT_SECONDS);
}

void remove_channel_edit(const dpp::snowflake channel_id, const channel_edit_type edit_type) {
	db::sql << "DELETE FROM channel_name_edit_timers WHERE channel_id=?;" << channel_id.str();
	channel_edits[edit_type].erase(channel_id);
	channel_edit_timers[edit_type].erase(channel_id);
}

void remove_channel_edit(const dpp::snowflake channel_id, const channel_edit_type edit_type, const time_t delay) {
	bot->start_timer([channel_id, edit_type](const dpp::timer& h) {
		remove_channel_edit(channel_id, edit_type);
		bot->stop_timer(h);
	}, delay);
}
