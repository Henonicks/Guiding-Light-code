#ifndef VARIANT_ITERATOR_HPP
#define VARIANT_ITERATOR_HPP

#include <variant>

template <typename variant_t, typename callback_t, size_t I = 0>
size_t get_variant_type(const variant_t& value, callback_t&& callback) {
	if constexpr (I < std::variant_size_v <variant_t>) {
		if (I == value.index()) {
			return get_variant_type <variant_t, callback_t, I + 1>(value, std::forward <callback_t>(callback));
		}
		callback.template operator()<std::variant_alternative_t <I, variant_t>>();
	}
	return I;
}

template <typename T, typename predicate_t, typename callback_t, size_t I = 0>
size_t iterate_through_variant(predicate_t&& predicate, callback_t&& callback) {
	if constexpr (I < std::variant_size_v <T>) {
		if (predicate.template operator()<std::variant_alternative_t <I, T>>()) { // holy smokes, this is unholy
			return iterate_through_variant <T, predicate_t, callback_t, I + 1>(std::forward <predicate_t>(predicate), std::forward <callback_t>(callback));
		}
		callback.template operator()<std::variant_alternative_t <I, T>>(); // same thing but for a different function
	}
	return I;
}

#endif
