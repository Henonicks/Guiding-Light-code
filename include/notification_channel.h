#ifndef NOTIFICATION_CHANNEL_H
#define NOTIFICATION_CHANNEL_H

#include <guidingLight/guiding_light.h>
#include "database.h"

struct notification_channel {
    dpp::snowflake channel_id, guild_id;
};

bool operator <(notification_channel chnl1, notification_channel chnl2);

notification_channel get_ntf_chnl(const std::string& line);

extern std::map <guild_snowflake, channel_snowflake> temp_vc_notifications;
extern std::map <guild_snowflake, channel_snowflake> topgg_notifications;

#endif
