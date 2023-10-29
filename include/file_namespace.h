#pragma once
#include <guidingLight/guiding_light.h>


namespace file {
    const std::string temp_vc_notifications = "../src/temp_vc_notifications.txt";
    const std::string jtc_vcs = "../src/jtc_vcs.txt";
    const std::string temp = "../src/temp.txt";
    const std::string jtc_default_values = "../src/jtc_default_values.txt";

    void line_append(const std::string& s, std::string filename);
    std::string getline(const std::string& s, const std::string& filename, int start_position);
    void delete_line_once(const std::string& s, const std::string& filename, const bool& should_contain_at_pos_0 = false);
    void delete_if_line(const std::string& s, std::string filename);
    void line_attach(const std::string& s, const std::string& filename, int pos);
    void delete_line_once(int pos, std::string filename);
    bool is_in_file(const std::string& s, std::string filename);
    int pos_in_file(const std::string& s, const std::string& filename);
}