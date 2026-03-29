#ifndef CONFIG_VALUES_H
#define CONFIG_VALUES_H

#include <fstream>

#include "no_warns/dpp/dpp.h"
#include "henifig/types.hpp"

#include "guiding_light/launch_options.hpp"

inline std::string BOT_TOKEN, BOT_TOKEN_DEV, TOPGG_BOT_TOKEN, MODE_NAME, logs_directory;
inline std::atomic <dpp::snowflake> BOT_DM_LOGS, MY_ID, TOPGG_WEBHOOK_CHANNEL_ID, MY_GUILD_ID, MY_PRIVATE_GUILD_ID, TICKETS_GUILD_ID, LOGS_CHANNEL_ID;
inline dpp::emoji PING_EMOJI;
inline std::ofstream my_logs_release, my_logs_dev, guild_logs_release, guild_logs_dev, other_logs_release, other_logs_dev, sql_logs_release, sql_logs_dev;
inline constexpr uint8_t CACHE_DELAY = 5;
inline std::atomic <uint64_t> guild_amount, channel_amount, user_amount;
inline std::atomic <bool> move_on;
inline henifig::value_array PING_RESPONSES;
inline std::recursive_mutex cfg_values_mutex;

#endif
