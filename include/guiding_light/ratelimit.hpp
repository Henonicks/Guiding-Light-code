#ifndef RATELIMIT_H
#define RATELIMIT_H

#include "guiding_light.hpp"

inline constexpr uint16_t CHANNEL_RENAME_RATELIMIT_SECONDS = 600 / 2; // twice per ten minutes

enum channel_edit_type : uint8_t {
	cet_name,

	cet_size,
};

inline std::map <channel_snowflake, bool> channel_edits[cet_size];
inline std::map <channel_snowflake, time_t> channel_edit_timers[cet_size];

void add_channel_edit(dpp::snowflake channel_id, channel_edit_type edit_type);
void remove_channel_edit(dpp::snowflake channel_id, channel_edit_type edit_type);
void remove_channel_edit(dpp::snowflake channel_id, channel_edit_type edit_type, time_t delay);

#endif
