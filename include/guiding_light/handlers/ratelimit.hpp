#ifndef RATELIMIT_H
#define RATELIMIT_H

#include "guiding_light/resources/temp_vc.hpp"

inline constexpr uint16_t CHANNEL_RENAME_RATELIMIT_SECONDS = 600 / 2; // twice every ten minutes

enum channel_edit_type : uint8_t {
	cet_name,

	cet_size,
};

// TODO: document
dpp::coroutine <> make_channel_rename(dpp::snowflake channel_id);
dpp::coroutine <bool> early_for_channel_rename(dpp::snowflake channel_id);
bool early_for_channel_rename(const temp_vc_t& channel);

#endif
