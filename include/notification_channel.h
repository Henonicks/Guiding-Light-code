#pragma once
#include <guidingLight/guiding_light.h>

struct vc_notification_chnl {
    dpp::snowflake channelid, guildid;
};

bool operator <(vc_notification_chnl chnl1, vc_notification_chnl chnl2);

vc_notification_chnl get_ntf_chnl(const std::string& line);

extern std::map <dpp::snowflake, vc_notification_chnl> ntif_chnls;