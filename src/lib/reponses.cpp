#include "guiding_light/reponses.hpp"
#include "guiding_light/config_values.hpp"
#include "guiding_light/cfg.hpp"

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

std::string premade_response(const responses_enum response_id, std::string& lang) {
    const henifig::value_map& localisation = cfg::responses["LOCALISATION"];
    if (!localisation.contains(lang)) {
        lang = "default";
    }
    const henifig::value_map& responses = localisation.at(lang);
    const std::string_view key = responses_keys[response_id];
    if (responses.contains(key.data())) {
        return responses.at(responses_keys[response_id]);
    }
    else if (const henifig::value_map& fallback_responses = localisation.at("default");
    fallback_responses.contains(key.data())) {
        return fallback_responses.at(key.data());
    }
    else {
        return fallback_responses.at("default");
    }
}

fmt::dynamic_format_arg_store <fmt::format_context> vec_to_fmt(const std::vector <std::string>& vec) {
    fmt::dynamic_format_arg_store <fmt::format_context> fmt;
    for (const std::string& x : vec) {
        fmt.push_back(x);
    }
    return fmt;
}
