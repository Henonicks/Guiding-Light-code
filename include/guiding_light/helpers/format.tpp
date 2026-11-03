#ifndef FORMAT_H
#define FORMAT_H

#include <format>

#include <dpp/snowflake.h>

#include "guiding_light/helpers/templatable_string.hpp"

#include "henifig/types.hpp"

template <>
struct std::formatter <dpp::snowflake> : std::formatter <std::string> {
	auto format(const dpp::snowflake snowflake, std::format_context& ctx) const {
		return std::formatter <std::string>::format(snowflake.str(), ctx);
	}
};

template <size_t N>
struct std::formatter <templatable_string_t <N>> : std::formatter <std::string> {
	auto format(templatable_string_t <N> str, std::format_context& ctx) const {
		return std::formatter <std::string>::format(std::string(str.buf), ctx);
	}
};

template <>
struct std::formatter <henifig::value_t> : std::formatter <std::string> {
	auto format(const henifig::value_t& val, std::format_context& ctx) const {
		return std::formatter <std::string>::format(val.get <std::string>(), ctx);
	}
};

/**
 * @brief Formats a string using std::vformat with the to-be substituted values being stored in an std::vector <std::string>. Fuck fmtlib, I'd rather do this than use it.
 * @param base The base formatted string.
 * @param values The std::vector <std::string> with the substituted values.
 * @return A formated std::string.
 */
[[nodiscard]] inline std::string format_with_vector(const std::string_view base, std::vector <std::string>&& values) {
	switch (values.size()) {
	case 0: return base.data();
	case 1: return std::vformat(base, std::make_format_args(values[0]));
	case 2: return std::vformat(base, std::make_format_args(values[0], values[1]));
	case 3: return std::vformat(base, std::make_format_args(values[0], values[1], values[2]));
	case 4: return std::vformat(base, std::make_format_args(values[0], values[1], values[2], values[3]));
	case 5: return std::vformat(base, std::make_format_args(values[0], values[1], values[2], values[3], values[4]));
	case 6: return std::vformat(base, std::make_format_args(values[0], values[1], values[2], values[3], values[4], values[5]));
	case 7: return std::vformat(base, std::make_format_args(values[0], values[1], values[2], values[3], values[4], values[5], values[6]));
	case 8: return std::vformat(base, std::make_format_args(values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7]));
	default: throw std::runtime_error("Too many arguments... I was pretty sure 8 were going to be enough, but apparently I underestimated my future needs");
	}
}

#endif
