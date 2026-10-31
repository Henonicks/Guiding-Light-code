#ifndef TEMP_PING_BLOCKER_H
#define TEMP_PING_BLOCKER_H

#include "guiding_light/guiding_light.hpp"
#include "base_resource.hpp"

struct temp_ping_blocker_t : base_resource_t {
	using base_resource_t::base_resource_t;

	static constexpr std::string_view table_name = "temp_ping_blockers";
	inline static const std::vector <std::string_view> field_list = {"user"};

	dpp::snowflake user_id;

	inline static std::recursive_mutex cache_mutex;
	inline static std::unordered_map <user_snowflake, temp_ping_blocker_t> cache;
	constexpr void put_in_cache(const bool overwrite = true) const {
		std::unique_lock L(cache_mutex);
		if (!overwrite && lookup_in_cache(this->user_id).is_valid()) {
			return;
		}
		cache[this->user_id] = *this;
	}
	[[nodiscard]] static constexpr temp_ping_blocker_t lookup_in_cache(const dpp::snowflake user_id) {
		std::unique_lock L(cache_mutex);
		return cache.contains(user_id) ? cache[user_id] : temp_ping_blocker_t(false);
	}
	static constexpr void delete_from_cache(const dpp::snowflake user_id) {
		std::unique_lock L(cache_mutex);
		cache[user_id] = temp_ping_blocker_t();
	}

	temp_ping_blocker_t() = default;
	temp_ping_blocker_t(const dpp::snowflake user_id)
		: user_id(user_id) {}

	[[nodiscard]] static constexpr std::string where(const dpp::snowflake user_id) {
		return "user=" + user_id.str();
	}

	[[nodiscard]] constexpr bool exists() const {
		return !this->user_id.empty();
	}

#ifdef GL_USE_LOCAL_DB
	static temp_ping_blocker_t sql_create(temp_ping_blocker_t resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("INSERT INTO {0} VALUES (?); -- {1}", table_name, context) <<
			resource.user_id.str()
		);
		if (failure) {
			return {false};
		}
		return resource;
	}

	[[nodiscard]] static auto sql_get_callback(temp_ping_blocker_t& resource) {
		return [&resource](const db::BIGINT user_id) {
			resource.user_id = user_id;
		};
	}
#endif
};

void fill_from_json_impl(temp_ping_blocker_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const temp_ping_blocker_t& temp_ping_blocker);

#endif
