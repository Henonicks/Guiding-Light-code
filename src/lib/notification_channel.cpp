#include <notification_channel.h>

bool operator <(notification_chnl chnl1, notification_chnl chnl2) {
    return chnl1.channelid < chnl2.channelid;
}

notification_chnl get_ntf_chnl(const std::string& line) {
    notification_chnl result_line;
    std::string channelid_line, guildid_line;
    int i = 0;
    for (;i < line.size(); i++) {
        //std::cout << i << std::endl;
        if (line[i] != ' ') {
            channelid_line += line[i];
        }
        else {
            i++;
            result_line.channelid = (dpp::snowflake)channelid_line;
            //std::cout << "Get_info_line " << result_line.channelid << std::endl;
            break;
        }
    }
    for (;i < line.size(); i++) {
        //std::cout << i << std::endl;
        if (line[i] != ' ') {
            guildid_line += line[i];
        }
    }
    result_line.guildid = (dpp::snowflake)guildid_line;
    //std::cout << "Get_info_line " << result_line.limit << std::endl;
    return result_line;
}

std::map <dpp::snowflake, dpp::snowflake> ntif_chnls;
std::map <dpp::snowflake, dpp::snowflake> topgg_ntif_chnls;
