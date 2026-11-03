#include "guiding_light/handlers/responses.hpp"

#include "guiding_light/internal/config_values.hpp"
#include "guiding_light/internal/cfg.hpp"
#include "guiding_light/internal/cli.hpp"

#include <random>

dpp::coroutine <std::string> random_response(const dpp::snowflake user_id) {
	std::random_device rd;
	std::mt19937 gen(rd());

	std::lock_guard L(cfg::config_mutex);

	std::uniform_int_distribution <> dist(0, PING_RESPONSES.size() - 1);
	std::string response = PING_RESPONSES[dist(gen)];
	size_t pos{};
	const dpp::user user = co_await lookup_user(user_id);
	while (pos != std::string::npos) {
		pos = response.find("{username}", pos);
		if (pos != std::string::npos && response.find("\\{username}\\", pos) == std::string::npos) {
			response.replace(pos, user.username.size() + 1, user.username);
		}
	}
	co_return response;
}

henifig::value_t response(const responses_enum response_id, const std::string_view lang, const henifig::value_map& localisation) {
	std::string used_lang = lang.data();

	std::lock_guard L(cfg::config_mutex);

	if (!localisation.contains(lang.data())) {
		used_lang = "default";
	}
	const henifig::value_map& responses = localisation.at(used_lang);
	const std::string_view key = responses_keys[response_id];
	if (responses.contains(key.data())) {
		return responses.at(key.data());
	}
	if (const henifig::value_map& fallback_responses = localisation.at("default");
	fallback_responses.contains(key.data())) {
		return fallback_responses.at(key.data());
	}
	return response(RESPONSE_NOT_FOUND, lang);
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

henifig::value_map cmd_response(const std::string_view name, const std::string_view lang, const henifig::value_map& commands) {
	std::lock_guard L(cfg::config_mutex);

	std::string used_lang = lang.data();
	if (!commands.contains(lang.data())) {
		used_lang = "default";
	}
	const henifig::value_map& responses = commands.at(used_lang);
	if (responses.contains(name.data())) {
		return responses.at(name.data());
	}
	if (const henifig::value_map& fallback_responses = commands.at("default");
	fallback_responses.contains(name.data())) {
		return fallback_responses.at(name.data());
	}
	return henifig::value_map{};
}

std::string error_response(const uint32_t error_code, const std::string_view lang, const henifig::value_map& error_descriptions) {
	std::lock_guard L(cfg::config_mutex);

	std::string used_lang = lang.data();
	if (!error_descriptions.contains(used_lang)) {
		used_lang = "default";
	}
	const henifig::value_map& responses = error_descriptions.at(used_lang);
	const std::string code_str = std::to_string(error_code);
	if (responses.contains(code_str)) {
		return {responses.at(code_str)};
	}
	if (const henifig::value_map& fallback_responses = error_descriptions.at("default");
	fallback_responses.contains(code_str)) {
		return {fallback_responses.at(code_str)};
	}
	return "";
}
