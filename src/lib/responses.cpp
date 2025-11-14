#include "guiding_light/responses.hpp"
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

henifig::value_t response(const responses_enum response_id, const std::string_view lang, const henifig::value_map& localisation) {
	std::string used_lang = lang.data();
	if (!localisation.contains(lang.data())) {
		used_lang = "default";
	}
	const henifig::value_map& responses = localisation.at(used_lang);
	const std::string_view key = responses_keys[response_id];
	if (responses.contains(key.data())) {
		return responses.at(key.data());
	}
	else if (const henifig::value_map& fallback_responses = localisation.at("default");
	fallback_responses.contains(key.data())) {
		return fallback_responses.at(key.data());
	}
	else {
		return response(RESPONSE_NOT_FOUND, lang);
	}
}

dpp::message response_msg(const responses_enum response_id, const std::string_view lang, const henifig::value_map& localisation) {
	return dpp::message(response(response_id, lang, localisation).get <std::string>());
}

dpp::message response_emsg(const responses_enum response_id, const std::string_view lang, const henifig::value_map& localisation) {
	return dpp::message(response(response_id, lang, localisation).get <std::string>()).set_flags(dpp::m_ephemeral);
}

std::string response_str(const responses_enum response_id, const std::string_view lang, const henifig::value_map& localisation) {
	return response(response_id, lang, localisation);
}

std::string response_fmt(const responses_enum response_id, const std::string_view lang, const std::vector<std::string>& values, const henifig::value_map& localisation) {
	return format_if_filled(response(response_id, lang, localisation), values);
}

dpp::message response_fmtmsg(const responses_enum response_id, const std::string_view lang, const std::vector<std::string>& values, const henifig::value_map& localisation) {
	return dpp::message(response_fmt(response_id, lang, values, localisation));
}

dpp::message response_fmtemsg(const responses_enum response_id, const std::string_view lang, const std::vector<std::string>& values, const henifig::value_map& localisation) {
	return dpp::message(response_fmt(response_id, lang, values, localisation)).set_flags(dpp::m_ephemeral);
}

henifig::value_map cmd_response(const std::string_view name, const std::string_view lang, const henifig::value_map& commands) {
	std::string used_lang = lang.data();
	if (!commands.contains(lang.data())) {
		used_lang = "default";
	}
	const henifig::value_map& responses = commands.at(used_lang);
	if (responses.contains(name.data())) {
		return responses.at(name.data());
	}
	else if (const henifig::value_map& fallback_responses = commands.at("default");
	fallback_responses.contains(name.data())) {
		return fallback_responses.at(name.data());
	}
	else {
		return henifig::value_map{};
	}
}

fmt::dynamic_format_arg_store <fmt::format_context> vec_to_fmt(const std::vector <std::string>& vec) {
	fmt::dynamic_format_arg_store <fmt::format_context> fmt;
	for (const std::string& x : vec) {
		fmt.push_back(x);
	}
	return fmt;
}
