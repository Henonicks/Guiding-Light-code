#ifndef COMMANDS_T
#define COMMANDS_T

#include "guiding_light/commands.hpp"

template <typename Type>
std::vector <Type> concat_vectors(const std::vector <std::vector <Type>>& vectors) {
	std::vector <Type> result;
	for (const auto& x : vectors) {
		for (const Type& y : x) {
			result.push_back(y);
		}
	}
	return result;
}

template <typename key_type, typename value_type>
std::vector <key_type> map_keys_to_vector(const std::map <key_type, value_type>& map) {
	std::vector <key_type> result;
	std::transform(map.begin(), map.end(), std::back_inserter(result), [](const std::pair <key_type, value_type>& pair) {
		return pair.first;
	});
	return result;
}

template <typename key_type, typename value_type>
std::vector <value_type> map_values_to_vector(const std::map <key_type, value_type>& map) {
	std::vector <value_type> result;
	std::transform(map.begin(), map.end(), std::back_inserter(result), [](const std::pair <key_type, value_type>& pair) {
		return pair.second;
	});
	return result;
}

#endif
