#include "guiding_light/json/json_interface.hpp"

#include "guiding_light/helpers/variant_iterator.tpp"

[[nodiscard]] std::string get_as_string(const nlohmann::json& json) {
	if (json.is_boolean()) {
		return json.get <bool>() == true ? "true" : "false";
	}
	if (json.is_number_integer()) {
		return std::to_string(json.get <int64_t>());
	}
	if (json.is_number_unsigned()) {
		return std::to_string(json.get <uint64_t>());
	}
	if (json.is_number_float()) {
		return std::to_string(json.get <float>());
	}
	if (json.is_string()) {
		return json.get <std::string>();
	}
	if (json.is_null()) {
		return "null";
	}
	return json.dump();
}

[[nodiscard]] nlohmann::json to_json_impl(const simple_json_types& value) {
	nlohmann::json res;
	get_variant_type(value,
	[&value, &res] <typename X> {
		res = std::get <X>(value);
	});
	return res;
}
