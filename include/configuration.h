#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "guidingLight/guiding_light.h"
#include "notification_channel.h"
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "file_namespace.h"

extern std::string elgato, BOT_TOKEN;
extern dpp::snowflake bot_logs_copy, bot_dm_logs, my_id, bot_id, my_guild_id;
extern std::ofstream my_logs;

namespace configuration {
    void configure_bot();
    void configure_channels(dpp::cluster& bot);
}
#endif