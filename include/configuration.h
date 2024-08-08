#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "guidingLight/guiding_light.h"
#include "notification_channel.h"
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "file_namespace.h"

std::string BOT_TOKEN;
dpp::snowflake bot_logs, bot_dm_logs, my_id;
std::ofstream my_logs;

namespace configuration {
    void configure_bot();
    void configure_channels(dpp::cluster& bot);
}

#endif
