#include <jtc_defaults.h>

bool operator <(jtc_defaults jd1, jtc_defaults jd2) {
    return jd1.name.size() < jd2.name.size();
}

jtc_defaults get_jtc_defs(std::string line) {
    jtc_defaults result;
    std::string channelid_line, name, limit, bitrate_line;
    int i = 0;
    for (;i < line.size(); i++) {
        if (line[i] == ' ') {
            break;
        }
        channelid_line += line[i];
    }
    i++;
    for (;i < line.size(); i++) {
        if (line[i] == ' ') {
            break;
        }
        name += line[i];
    }
    i++;
    for (;i < line.size(); i++) {
        if (line[i] == ' ') {
            break;
        }
        limit += line[i];
    }
    i++;
    for (;i < line.size(); i++) {
        if (line[i] == ' ') {
            break;
        }
        bitrate_line += line[i];
    }
    return {channelid_line, name, (int)string_to_ll(limit), (int)string_to_ll(bitrate_line)};
}

std::map <dpp::snowflake, jtc_defaults> jtc_default_values;