#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "guidingLight/guiding_light.h"
#include "notification_channel.h"
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "file_namespace.h"
#include "logging.h"
#include "slash_funcs.h"
#include "topgg.h"

extern std::string_view logs_directory;
extern std::string BOT_TOKEN, logs_suffix;
extern dpp::snowflake bot_dm_logs, my_id, TOPGG_WEBHOOK_CHANNEL_ID, MY_GUILD_ID;
extern std::ofstream my_logs, guild_logs, other_logs;
extern bool bot_return;
extern bool is_dev;
extern int delay;

namespace configuration {
    void configure_bot(const bool& is_dev);
    void configure_channels(dpp::cluster& bot);
	void write_down_slashcommands(dpp::cluster& bot);
}

#endif
