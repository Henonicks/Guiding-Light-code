#include <notification_channel.h>

bool operator <(notification_channel chnl1, notification_channel chnl2) {
    return chnl1.channel_id < chnl2.channel_id;
}

std::map <guild_snowflake, channel_snowflake> temp_vc_notifications;
std::map <guild_snowflake, channel_snowflake> topgg_notifications;
