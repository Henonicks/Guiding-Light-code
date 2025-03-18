#ifndef FILE_NAMESPACE_H
#define FILE_NAMESPACE_H

#include "guidingLight/guiding_light.h"

namespace file {
    extern std::string temp_vc_notifications;
    extern std::string jtc_vcs;
    extern std::string temp;
    extern std::string jtc_default_values;
    extern std::string no_temp_ping;
    extern std::string topgg_guild_votes_amount;
    extern std::string topgg_guild_choices;
    extern std::string no_noguild_reminder;
    extern std::string topgg_notifications;
    extern std::string tickets;

    void line_append(const std::string& s, const std::string& filename);
    std::string getline(const std::string& s, const std::string& filename, const int& start_position = -1);
    void delete_line_once(const std::string& s, const std::string& filename, const bool& should_contain_at_pos_0 = false);
    void delete_if_line(const std::string& s, const std::string& filename);
    void line_attach(const std::string& s, const std::string& filename, int pos);
    void delete_line_once(int pos, const std::string& filename);
    bool is_in_file(const std::string& s, const std::string& filename);
    int pos_in_file(const std::string& s, const std::string& filename);
	int count_lines(const std::string& s, const std::string& filename);

}

#endif
