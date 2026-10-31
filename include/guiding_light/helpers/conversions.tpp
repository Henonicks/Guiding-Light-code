#ifndef CONVERSIONS_T
#define CONVERSIONS_T

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

template <typename desired_type, typename key_type, typename value_type>
std::vector <desired_type> map_keys_to_vector(const std::map <key_type, value_type>& map) {
	std::vector <desired_type> result;
	std::transform(map.begin(), map.end(), std::back_inserter(result), [](const std::pair <key_type, value_type>& pair) {
		return desired_type(pair.first);
	});
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
std::vector <key_type> map_keys_to_vector(const std::unordered_map <key_type, value_type>& map) {
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

template <typename Type>
std::vector <uint64_t> to_int_vector(const std::vector <Type>& vector, auto* conversion_rule = nullptr) {
	std::vector <uint64_t> res;
	std::transform(vector.begin(), vector.end(), std::back_inserter(res), [conversion_rule](const Type& x) {
		return conversion_rule == nullptr ? x : conversion_rule(x);
	});
	return res;
}

#endif
