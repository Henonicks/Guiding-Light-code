#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "guidingLight/guiding_light.h"
#include "notification_channel.h"
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "file_namespace.h"
#include "logging.h"

extern std::string BOT_TOKEN;
extern dpp::snowflake bot_dm_logs, my_id;
extern std::ofstream my_logs, guild_logs, other_logs;

namespace configuration {
    void configure_bot(bool is_dev);
    void configure_channels();
}

#endif
