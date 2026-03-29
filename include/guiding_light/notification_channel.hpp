#ifndef NOTIFICATION_CHANNEL_H
#define NOTIFICATION_CHANNEL_H

#include "guiding_light/guiding_light.hpp"

inline std::unordered_map <guild_snowflake, channel_snowflake> temp_vc_notifications;
inline std::unordered_map <guild_snowflake, channel_snowflake> topgg_notifications;
inline std::recursive_mutex notification_mutex;

#endif
