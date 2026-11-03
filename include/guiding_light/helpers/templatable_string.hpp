#ifndef TEMPLATABLE_STRING_H
#define TEMPLATABLE_STRING_H

#include <string_view>

template <size_t N>
struct templatable_string_t {
	char buf[N]{};
	constexpr templatable_string_t(const char (&str)[N]) {
		std::copy_n(str, N, buf);
	}

	[[nodiscard]] static bool empty() {
		return N <= 1;
	}
	operator std::string_view() const {
		return buf;
	}
};

#endif
