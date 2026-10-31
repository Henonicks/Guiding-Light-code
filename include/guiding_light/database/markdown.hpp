#ifndef MARKDOWN_H
#define MARKDOWN_H

#include "guiding_light/resources/base_resource.hpp"

struct markdown_t : base_resource_t {
	using base_resource_t::base_resource_t;

	std::string content;

	markdown_t() = default;
	markdown_t(const std::string_view content) : content(content) {}

	[[nodiscard]] static dpp::coroutine <markdown_t> generate_md(std::string_view table_name);
};

#endif
