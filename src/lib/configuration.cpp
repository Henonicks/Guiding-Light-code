#include <configuration.h>

using json = nlohmann::json;

std::string BOT_TOKEN;
dpp::snowflake bot_dm_logs, my_id;
std::ofstream my_logs, guild_logs, other_logs;

void configuration::configure_bot(bool is_dev) {
    json config;
    std::ifstream config_file_stream("../config.json");
    config_file_stream >> config;

    bot_dm_logs = config["BOT_DM_LOGS_ID"];
    my_id = config["MY_ID"];

    BOT_TOKEN = (is_dev ? config["BOT_TOKEN_DEV"] : config["BOT_TOKEN"]);

	std::string_view logs_suffix = (is_dev ? "dev" : "release");

    my_logs.open(fmt::format("../logging/{}/my_logs.log", logs_suffix));
    guild_logs.open(fmt::format("../logging/{}/guild_logs.log", logs_suffix));
    other_logs.open(fmt::format("../logging/{}/other_logs.log", logs_suffix));

	file::temp_vc_notifications = fmt::format("../src/{}/temp_vc_notifications.txt", logs_suffix);
	file::jtc_vcs = fmt::format("../src/{}/jtc_vcs.txt", logs_suffix);
	file::temp = fmt::format("../src/{}/temp.txt", logs_suffix);
	file::jtc_default_values = fmt::format("../src/{}/jtc_default_values.txt", logs_suffix);
}

void configuration::configure_channels() {
    std::string line;

    std::ifstream last_jtc_vcs;
    last_jtc_vcs.open(file::jtc_vcs);
    while (std::getline(last_jtc_vcs, line)) {
		dpp::channel* channel = dpp::find_channel(get_jtc_vc(line).channelid);
		if (channel != nullptr) {
			jtc_vc current = get_jtc_vc(line);
			jtc_vcs[current.channelid] = current;
			jtc_channels_map[current.channelid] = *channel;
		}
		else {
			file::delete_line_once(line, file::jtc_vcs);
		}
    }

    last_jtc_vcs.close();
    std::ifstream last_temp_vc_notifications;
    last_temp_vc_notifications.open(file::temp_vc_notifications);

    while (std::getline(last_temp_vc_notifications, line)) {
		dpp::channel* channel = dpp::find_channel(get_ntf_chnl(line).channelid);
		if (channel != nullptr) {
			vc_notification_chnl current = get_ntf_chnl(line);
			ntif_chnls[current.guildid] = current;
		}
        else {
			file::delete_line_once(line, file::temp_vc_notifications);
        }
    }

	last_temp_vc_notifications.close();

    std::ifstream jtc_default_values_file;
    jtc_default_values_file.open(file::jtc_default_values);

    while (std::getline(jtc_default_values_file, line)) {
		dpp::channel* channel = dpp::find_channel(get_jtc_defs(line).channelid);
		if (channel != nullptr) {
			jtc_defaults current = get_jtc_defs(line);
			jtc_default_values[current.channelid] = current;
		}
        else {
			file::delete_line_once(line, file::jtc_default_values);
        }
    }

    jtc_default_values_file.close();

}
