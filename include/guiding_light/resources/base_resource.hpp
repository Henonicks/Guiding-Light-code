#ifndef BASE_RESOURCE_H
#define BASE_RESOURCE_H

#include "guiding_light/database/database.hpp"

struct base_resource_t {
	bool valid{true};

	base_resource_t() = default;
	base_resource_t(const bool valid) : valid(valid) {}

	/**
	 * @brief Returns whether the resource is valid (was looked up successfully) or not.
	 * @return true if the resource is valid, false otherwise.
	 */
	[[nodiscard]] bool is_valid() const {
		return this->valid;
	}

	virtual ~base_resource_t() = default;
};

#endif
