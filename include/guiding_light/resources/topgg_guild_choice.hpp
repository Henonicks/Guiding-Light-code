#ifndef GUILD_CHOICES_H
#define GUILD_CHOICES_H

#include "guiding_light/guiding_light.hpp"
#include "guiding_light/resources/base_resource.hpp"

struct topgg_guild_choice_t : base_resource_t {
	using base_resource_t::base_resource_t;

	static constexpr std::string_view table_name = "topgg_guild_choices";
	inline static const std::vector <std::string_view> field_list = {"user", "guild"};

    dpp::snowflake user_id, guild_id;

	inline static std::recursive_mutex cache_mutex;
	inline static std::unordered_map <user_snowflake, topgg_guild_choice_t> cache;
	constexpr void put_in_cache(const bool overwrite = true) const {
		std::unique_lock L(cache_mutex);
		if (!overwrite && lookup_in_cache(this->user_id).is_valid()) {
			return;
		}
		cache[this->user_id] = *this;
	}
	[[nodiscard]] static constexpr topgg_guild_choice_t lookup_in_cache(const dpp::snowflake user_id) {
		std::unique_lock L(cache_mutex);
		return cache.contains(user_id) ? cache[user_id] : topgg_guild_choice_t(false);
	}
	static constexpr void delete_from_cache(const dpp::snowflake user_id) {
		std::unique_lock L(cache_mutex);
		cache.erase(user_id);
	}

	[[nodiscard]] static constexpr bool is_cachable() {
		return
#ifdef GL_BOT_ENVIRONMENT
		ENABLE_TOPGG
#else
		false
#endif
		;
	}

	topgg_guild_choice_t() = default;
	topgg_guild_choice_t(const dpp::snowflake user_id, const dpp::snowflake guild_id)
		: user_id(user_id), guild_id(guild_id) {}

    [[nodiscard]] static constexpr std::string where(const dpp::snowflake user_id) {
    	return "user=" + user_id.str();
    }

    [[nodiscard]] std::string insert() const;

	[[nodiscard]] constexpr bool exists() const {
		return !this->user_id.empty();
	}

#ifdef GL_USE_LOCAL_DB
	static topgg_guild_choice_t sql_create(const topgg_guild_choice_t& resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("INSERT INTO {0} VALUES (?, ?); -- {1}", table_name, context)
			<< resource.user_id.str() << resource.guild_id.str()
		);
		if (failure) {
			return {false};
		}
		return resource;
	}

	[[nodiscard]] static auto sql_get_callback(topgg_guild_choice_t& resource) {
		return [&resource](const db::BIGINT user_id, const db::BIGINT guild_id) {
			resource.user_id = user_id;
			resource.guild_id = guild_id;
		};
	}

	static topgg_guild_choice_t sql_update(const topgg_guild_choice_t& resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("UPDATE {0} SET guild=? WHERE {1}; -- {2}", table_name, where(resource.user_id), context)
			<< resource.guild_id.str()
		);
		if (failure) {
			return {false};
		}
		return resource;
	}
#endif
};

void fill_from_json_impl(topgg_guild_choice_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const topgg_guild_choice_t& guild_choice);

#endif
