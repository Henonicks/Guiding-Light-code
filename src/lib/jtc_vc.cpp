#include <jtc_vc.h>

bool operator <(jtc_vc vc1, jtc_vc vc2) {
    unsigned long long c1 = vc1.channelid, c2 = vc2.channelid;
    return c1 < c2;
}

jtc_vc get_jtc_vc(std::string line) {
    jtc_vc result_line;
    std::string channelid_line, guildid_line;
    int i = 0;
    for (;i < line.size(); i++) {
        //std::cout << i << std::endl;
        if (line[i] != ' ') {
            channelid_line += line[i];
        }
        else {
            ++i;
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
        else {
            ++i;
            result_line.guildid = (dpp::snowflake)guildid_line;
            //std::cout << "Get_info_line (guildid) " << guildid_line << " snowflake " << (dpp::snowflake)guildid_line << std::endl;
            break;
        }
    }
    return result_line;
}

std::map <dpp::snowflake, dpp::channel> jtc_channels_map;
std::map <dpp::snowflake, jtc_vc> jtc_vcs;