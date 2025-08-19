#include "guiding_light/ping.hpp"
#include "guiding_light/config_values.hpp"

std::string random_response(const dpp::snowflake& user_id) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution <> dist(0, PING_RESPONSES.size() - 1);
    std::string response = PING_RESPONSES[dist(gen)];
    size_t pos{};
    const dpp::user* user = dpp::find_user(user_id);
    while (pos != std::string::npos) {
        pos = response.find("{username}", pos);
        if (pos != std::string::npos && response.find("\\{username}\\", pos) == std::string::npos) {
            response.replace(pos, user->username.size() + 1, user->username);
        }
    }
    return response;
}
