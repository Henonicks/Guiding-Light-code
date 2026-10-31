#ifndef GUILD_VOTE_AMOUNTS_H
#define GUILD_VOTE_AMOUNTS_H

#include "guiding_light/resources/base_resource.hpp"
#include "guiding_light/guiding_light.hpp"

struct topgg_guild_vote_amount_t : base_resource_t {
	using base_resource_t::base_resource_t;

	static constexpr std::string_view table_name = "topgg_guild_vote_amounts";
	inline static const std::vector <std::string_view> field_list = {"guild", "votes"};

	dpp::snowflake guild_id;
	int votes{};

	inline static std::recursive_mutex cache_mutex;
	inline static std::unordered_map <guild_snowflake, topgg_guild_vote_amount_t> cache;
	constexpr void put_in_cache(const bool overwrite = true) const {
		std::unique_lock L(cache_mutex);
		if (!overwrite && lookup_in_cache(this->guild_id).is_valid()) {
			return;
		}
		cache[this->guild_id] = *this;
	}
	[[nodiscard]] static constexpr topgg_guild_vote_amount_t lookup_in_cache(const dpp::snowflake guild_id) {
		std::unique_lock L(cache_mutex);
		return cache.contains(guild_id) ? cache[guild_id] : topgg_guild_vote_amount_t(false);
	}
	static constexpr void delete_from_cache(const dpp::snowflake guild_id) {
		std::unique_lock L(cache_mutex);
		cache.erase(guild_id);
	}

	[[nodiscard]] static constexpr bool is_cachable() {
		return
#ifdef GL_BOT_ENVIRONMENT
		ENABLE_TOPGG
#else
		true
#endif
		;
	}

	topgg_guild_vote_amount_t() = default;
	topgg_guild_vote_amount_t(const dpp::snowflake guild_id, const int votes)
		: guild_id(guild_id), votes(votes) {}

	[[nodiscard]] static constexpr std::string where(const dpp::snowflake guild_id) {
		return "guild=" + guild_id.str();
	}

    [[nodiscard]] std::string insert() const;

	[[nodiscard]] constexpr bool exists() const {
		return !this->guild_id.empty();
	}

#ifdef GL_USE_LOCAL_DB
	static topgg_guild_vote_amount_t sql_create(const topgg_guild_vote_amount_t& resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("INSERT INTO {0} VALUES (?, ?); -- {1}", table_name, context)
			<< resource.guild_id.str() << resource.votes
		);
		if (failure) {
			return {false};
		}
		return resource;
	}

	[[nodiscard]] static auto sql_get_callback(topgg_guild_vote_amount_t& resource) {
		return [&resource](const db::BIGINT guild_id, const int votes) {
			resource.guild_id = guild_id;
			resource.votes = votes;
		};
	}

	static topgg_guild_vote_amount_t sql_update(const topgg_guild_vote_amount_t& resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("UPDATE {0} SET votes=? WHERE {1}; -- {2}", table_name, where(resource.guild_id), context)
			<< resource.votes
		);
		if (failure) {
			return {false};
		}
		return resource;
	}
#endif
};

void fill_from_json_impl(topgg_guild_vote_amount_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const topgg_guild_vote_amount_t& topgg_guild_vote_amount);

#endif
