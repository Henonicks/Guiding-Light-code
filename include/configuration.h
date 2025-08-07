#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "notification_channel.h"
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "logging.h"
#include "slash_funcs.h"
#include "topgg.h"
#include "ticket.h"

extern std::string_view logs_directory;
extern std::string BOT_TOKEN, MODE_NAME;
extern dpp::snowflake BOT_DM_LOGS, MY_ID, TOPGG_WEBHOOK_CHANNEL_ID, MY_GUILD_ID, MY_PRIVATE_GUILD_ID, TICKETS_GUILD_ID;
extern std::ofstream my_logs, guild_logs, other_logs, sql_logs;
extern dpp::start_type BOT_RETURN;
extern bool IS_DEV, IS_CLI;
extern int DELAY;
extern uint64_t guild_amount, channel_amount, user_amount;

namespace configuration {
	void read_config();
	void init_logs();
	void pray();
	void write_down_slashcommands(dpp::cluster& bot);
}

#endif
