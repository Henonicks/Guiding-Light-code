#include <configuration.h>

using json = nlohmann::json;

std::string BOT_TOKEN;
dpp::snowflake bot_logs, bot_dm_logs, my_id;
std::ofstream my_logs;
static std::ofstream other_logs;

void configuration::configure_bot() {
    json config;
    std::ifstream config_file_stream("../config.json");
    config_file_stream >> config;

    bot_logs = (dpp::snowflake) config["BOT_LOGS_ID"];
    bot_dm_logs = (dpp::snowflake) config["BOT_DM_LOGS_ID"];
    my_id = (dpp::snowflake) config["MY_ID"];
    BOT_TOKEN = config["BOT_TOKEN"];

    my_logs.open("../logging/my_logs.log");
    other_logs.open("../logging/other_logs.log");
}
void configuration::configure_channels(dpp::cluster& bot) {
    bot.on_log([](const dpp::log_t& log) -> void {
        other_logs << fmt::format("[{0}]: {1}", dpp::utility::current_date_time(), log.message) << std::endl;
    });

    std::string line;

    std::ifstream last_jtc_vcs;
    last_jtc_vcs.open(file::jtc_vcs);
    while (std::getline(last_jtc_vcs, line)) {
        try {
            dpp::channel channel = bot.channel_get_sync(get_jtc_vc(line).channelid);
            jtc_vc current = get_jtc_vc(line);
            jtc_vcs[channel.id] = current;
            jtc_channels_map[current.channelid] = channel;
        }
        catch (...) {
            file::delete_line_once(line, file::jtc_vcs);
        }
    }

    last_jtc_vcs.close();
    std::ifstream last_temp_vc_notifications;
    last_temp_vc_notifications.open(file::temp_vc_notifications);

    while (std::getline(last_temp_vc_notifications, line)) {
        try {
            dpp::channel channel = bot.channel_get_sync(get_ntf_chnl(line).channelid);
            vc_notification_chnl current = get_ntf_chnl(line);
            ntif_chnls[current.guildid] = current;
        }
        catch (...) {
            file::delete_line_once(line, file::temp_vc_notifications);
        }
    }

    std::ifstream jtc_default_values_file;
    jtc_default_values_file.open(file::jtc_default_values);

    while (std::getline(jtc_default_values_file, line)) {
        try {
            dpp::channel channel = bot.channel_get_sync(get_jtc_defs(line).channelid);
            jtc_defaults current = get_jtc_defs(line);
            jtc_default_values[current.channelid] = current;
        }
        catch (...) {
            file::delete_line_once(line, file::jtc_default_values);
        }
    }

    jtc_default_values_file.close();

}
