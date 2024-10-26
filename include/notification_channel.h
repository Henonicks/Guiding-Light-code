#ifndef NOTIFICATION_CHANNEL_H
#define NOTIFICATION_CHANNEL_H

#include <guidingLight/guiding_light.h>

struct notification_chnl {
    dpp::snowflake channelid, guildid;
};

bool operator <(notification_chnl chnl1, notification_chnl chnl2);

notification_chnl get_ntf_chnl(const std::string& line);

extern std::map <guild_snowflake, channel_snowflake> ntif_chnls;
extern std::map <guild_snowflake, channel_snowflake> topgg_ntif_chnls;

#endif
