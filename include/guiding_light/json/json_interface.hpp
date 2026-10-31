#ifndef JSON_INTERFACE_H
#define JSON_INTERFACE_H

#include <dpp/nlohmann/json.hpp>

using simple_json_types = std::variant <bool, int64_t, uint64_t, float, std::string>;

template <typename T>
[[nodiscard]] T fill_from_json(const nlohmann::json& json) {
	T res;
	fill_from_json_impl(res, json);
	return res;
}

[[nodiscard]] nlohmann::json to_json_impl(const simple_json_types& value);

template <typename T>
[[nodiscard]] nlohmann::json to_json(const T& resource) {
	return to_json_impl(resource);
}

template <typename T>
void fill_from_json_impl(std::vector <T>& res, const nlohmann::json& json) {
	res.reserve(json.size());
	for (const nlohmann::json& x : json) {
		res.push_back(fill_from_json <T>(x));
	}
}

template <typename T>
[[nodiscard]] nlohmann::json to_json_impl(const std::vector <T>& resources) {
	nlohmann::json res = nlohmann::json::array();
	for (const auto& x : resources) {
		res.push_back(to_json(x));
	}
	return res;
}

[[nodiscard]] std::string get_as_string(const nlohmann::json& json);

#endif
