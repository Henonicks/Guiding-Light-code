#ifndef CONFIG_VALUES_H
#define CONFIG_VALUES_H

#include <no_warns/dpp/dpp.h>
#include "fstream"

extern std::string BOT_TOKEN, BOT_TOKEN_DEV, MODE_NAME, logs_directory;
extern dpp::snowflake BOT_DM_LOGS, MY_ID, TOPGG_WEBHOOK_CHANNEL_ID, MY_GUILD_ID, MY_PRIVATE_GUILD_ID, TICKETS_GUILD_ID;
extern std::ofstream my_logs_release, my_logs_dev, guild_logs_release, guild_logs_dev, other_logs_release, other_logs_dev, sql_logs_release, sql_logs_dev;
extern dpp::start_type BOT_RETURN;
extern bool IS_DEV, IS_CLI;
extern int DELAY;
extern uint64_t guild_amount, channel_amount, user_amount;
extern std::vector <std::string> PING_RESPONSES;

#endif
