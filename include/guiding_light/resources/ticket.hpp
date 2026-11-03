#ifndef TICKET_H
#define TICKET_H

#include "guiding_light/guiding_light.hpp"
#include "guiding_light/resources/base_resource.hpp"

struct ticket_t : base_resource_t {
	using base_resource_t::base_resource_t;

	static constexpr std::string_view table_name = "tickets";
	inline static const std::vector <std::string_view> field_list = {"user", "channel", "dm_channel"};

	dpp::snowflake user_id, channel_id, dm_channel_id;

	inline static std::atomic_bool kept{};

	inline static std::recursive_mutex cache_mutex;
	inline static std::unordered_map <dpp::snowflake, ticket_t> cache;
	constexpr void put_in_cache(const bool overwrite = true) const {
		std::unique_lock L(cache_mutex);
		if (!overwrite && lookup_in_cache(this->channel_id).is_valid()) {
			return;
		}
		cache[this->user_id] = *this;
		cache[this->channel_id] = *this;
	}
	[[nodiscard]] static constexpr ticket_t lookup_in_cache(const dpp::snowflake id) {
		std::unique_lock L(cache_mutex);
		return cache.contains(id) ? cache[id] : ticket_t(false);
	}
	static constexpr void delete_from_cache(const dpp::snowflake id) {
		std::unique_lock L(cache_mutex);
		const ticket_t ticket = cache[id];
		cache.erase(ticket.user_id);
		cache.erase(ticket.channel_id);
	}

	ticket_t() = default;
	ticket_t(const dpp::snowflake user_id, const dpp::snowflake channel_id, const dpp::snowflake dm_channel_id)
		: user_id(user_id), channel_id(channel_id), dm_channel_id(dm_channel_id) {}

	[[nodiscard]] static constexpr std::string where_user(const dpp::snowflake user_id) {
		return "user=" + user_id.str();
	}

	[[nodiscard]] static constexpr std::string where_channel(const dpp::snowflake channel_id) {
		return "channel=" + channel_id.str();
	}

    [[nodiscard]] std::string insert() const;

	[[nodiscard]] constexpr bool exists() const {
		return !this->user_id.empty();
	}

#ifdef GL_USE_LOCAL_DB
	static ticket_t sql_create(const ticket_t& resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("INSERT INTO {0} VALUES (?, ?, ?); -- {1}", table_name, context)
			<< resource.user_id.str() << resource.channel_id.str() << resource.dm_channel_id.str()
		);
		if (failure) {
			return {false};
		}
		return resource;
	}

	[[nodiscard]] static auto sql_get_callback(ticket_t& resource) {
		return [&resource](const db::BIGINT user_id, const db::BIGINT channel_id, const db::BIGINT dm_channel_id) {
			resource.user_id = user_id;
			resource.channel_id = channel_id;
			resource.dm_channel_id = dm_channel_id;
		};
	}
#endif

	struct keep_list_t : base_resource_t {
		using base_resource_t::base_resource_t;

		std::vector <dpp::snowflake> channel_ids;

		constexpr void delete_from_cache() const {
			for (const dpp::snowflake x : channel_ids) {
				ticket_t::delete_from_cache(x);
			}
		}

		keep_list_t() = default;
		keep_list_t(keep_list_t&) = default;
		keep_list_t(const keep_list_t&) = default;
		keep_list_t(const std::vector <dpp::snowflake>& channel_ids)
			: channel_ids(channel_ids) {}

		keep_list_t& operator =(const keep_list_t& other) = default;
		keep_list_t& operator =(keep_list_t&& other) noexcept {
			this->channel_ids = std::move(other.channel_ids);
			return *this;
		}

#ifdef GL_USE_LOCAL_DB
		static keep_list_t sql_keep(const std::vector <dpp::snowflake>& channel_ids, const std::string_view context = "") {
			auto result = db::sql << std::format("SELECT channel FROM {0};{1}", table_name, context);
			if (db::is_error(result)) {
				return {false};
			}
			keep_list_t res;
			std::unordered_set <dpp::snowflake> channel_id_set;
			std::ranges::transform(channel_ids.begin(), channel_ids.end(), std::inserter(channel_id_set, channel_id_set.begin()), [](const dpp::snowflake value) {
				return value;
			});
			result >> [&channel_id_set, &res](const db::BIGINT channel_id) {
				if (channel_id_set.contains(channel_id)) {
					db::destroy <ticket_t, "", where_channel>(channel_id).sync_wait();
					res.channel_ids.emplace_back(channel_id);
				}
			};
			return res;
		}
#endif
	};
};

void fill_from_json_impl(ticket_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const ticket_t& ticket);

void fill_from_json_impl(ticket_t::keep_list_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const ticket_t::keep_list_t& bulk_list);

#endif
