#include "guiding_light/config_values.hpp"

std::string BOT_TOKEN, BOT_TOKEN_DEV, MODE_NAME, logs_directory;
dpp::snowflake BOT_DM_LOGS, MY_ID, TOPGG_WEBHOOK_CHANNEL_ID, MY_GUILD_ID, MY_PRIVATE_GUILD_ID, TICKETS_GUILD_ID;
std::ofstream my_logs_release, my_logs_dev, guild_logs_release, guild_logs_dev, other_logs_release, other_logs_dev, sql_logs_release, sql_logs_dev;
dpp::start_type BOT_RETURN = dpp::st_wait;
bool IS_DEV{}, IS_CLI{};
int DELAY = 5;
uint64_t guild_amount = 0, channel_amount = 0, user_amount = 0;
henifig::value_array PING_RESPONSES;
