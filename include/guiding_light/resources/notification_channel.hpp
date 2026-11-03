#ifndef NOTIFICATION_CHANNEL_H
#define NOTIFICATION_CHANNEL_H

#include "guiding_light/database/database.hpp"

#include "guiding_light/guiding_light.hpp"
#include "base_resource.hpp"

enum notification_source_t {
	ns_jtc = 1,
	ns_topgg = 2,
};

struct notification_channel_t : base_resource_t {
	using base_resource_t::base_resource_t;

	static constexpr std::string_view table_name = "notification_channels";
	inline static const std::vector <std::string_view> field_list = {"channel", "guild", "source"};

	dpp::snowflake channel_id, guild_id;
	notification_source_t source{};

	inline static std::mutex keep_mutex;
	inline static std::unordered_set <guild_snowflake> kept_guilds{};

	struct cache_key_variant {
		using tuple_t = std::tuple <guild_snowflake, notification_source_t>;

		std::variant <tuple_t, channel_snowflake> key;

		cache_key_variant(const dpp::snowflake guild_id, const notification_source_t source) : key(std::make_tuple(guild_id, source)) {}
		cache_key_variant(const dpp::snowflake channel_id) : key(channel_id) {}

		bool operator <(const cache_key_variant other) const {
			return this->key < other.key;
		}
	};

	inline static std::recursive_mutex cache_mutex;
	inline static std::map <cache_key_variant, notification_channel_t> cache;
	constexpr void put_in_cache(const bool overwrite = true) const {
		std::unique_lock L(cache_mutex);
		if (!overwrite && lookup_in_cache(this->guild_id, this->source).is_valid()) {
			return;
		}
		cache[{this->guild_id, this->source}] = *this;
	}
	[[nodiscard]] static constexpr notification_channel_t lookup_in_cache(const dpp::snowflake guild_id, const notification_source_t source) {
		std::unique_lock L(cache_mutex);
		return cache.contains({guild_id, source}) ? cache[{guild_id, source}] : notification_channel_t(false);
	}
	[[nodiscard]] static constexpr notification_channel_t lookup_in_cache(const dpp::snowflake channel_id) {
		std::unique_lock L(cache_mutex);
		return cache.contains(channel_id) ? cache[channel_id] : notification_channel_t(false);
	}
	static constexpr void delete_from_cache(const dpp::snowflake guild_id, const notification_source_t source) {
		std::unique_lock L(cache_mutex);
		const auto channel = lookup_in_cache(guild_id, source);
		cache.erase(channel.channel_id);
		cache.erase({channel.guild_id, channel.source});
	}
	static constexpr void delete_from_cache(const dpp::snowflake channel_id) {
		std::unique_lock L(cache_mutex);
		const auto channel = lookup_in_cache(channel_id);
		cache.erase(channel.channel_id);
		cache.erase({channel.guild_id, channel.source});
	}

	[[nodiscard]] constexpr bool is_cachable() const {
		return source != ns_topgg || ENABLE_TOPGG;
	}

	notification_channel_t() = default;
	notification_channel_t(const dpp::snowflake channel_id, const dpp::snowflake guild_id, const notification_source_t source)
		: channel_id(channel_id), guild_id(guild_id), source(source) {}

	[[nodiscard]] static constexpr std::string where_channel(const dpp::snowflake channel_id) {
		return "channel=" + channel_id.str();
	}

	[[nodiscard]] static constexpr std::string where_guild(const dpp::snowflake guild_id) {
		return "guild=" + guild_id.str();
	}

	[[nodiscard]] static constexpr std::string where(const dpp::snowflake guild_id, const notification_source_t source) {
		return "guild=" + guild_id.str() + db::LOOKUP_SEPARATOR + "source=" + std::to_string(source);
	}

    [[nodiscard]] std::string insert() const;

	[[nodiscard]] constexpr bool exists() const {
		return !this->guild_id.empty();
	}

#ifdef GL_USE_LOCAL_DB
	static notification_channel_t sql_create(const notification_channel_t& resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("INSERT INTO {0} VALUES (?, ?, ?); -- {1}", table_name, context)
			<< resource.channel_id.str() << resource.guild_id.str() << resource.source
		);
		if (failure) {
			return {false};
		}
		return resource;
	}

	[[nodiscard]] static auto sql_get_callback(notification_channel_t& resource) {
		return [&resource](const db::BIGINT channel_id, const db::BIGINT guild_id, const db::TINYINT source) {
			resource.channel_id = channel_id;
			resource.guild_id = guild_id;
			resource.source = cast <notification_source_t>(source);
		};
	}
#endif

	struct keep_list_t : base_resource_t {
		using base_resource_t::base_resource_t;

		dpp::snowflake guild_id;
		std::vector <dpp::snowflake> channel_ids;

		constexpr void delete_from_cache() const {
			for (const dpp::snowflake x : channel_ids) {
				notification_channel_t::delete_from_cache(x);
			}
		}

		keep_list_t() = default;
		keep_list_t(keep_list_t&) = default;
		keep_list_t(const keep_list_t&) = default;
		keep_list_t(const dpp::snowflake guild_id, const std::vector <dpp::snowflake>& channel_ids)
			: guild_id(guild_id), channel_ids(channel_ids) {}

		keep_list_t& operator =(const keep_list_t& other) = default;
		keep_list_t& operator =(keep_list_t&& other) noexcept {
			this->guild_id = other.guild_id;
			this->channel_ids = std::move(other.channel_ids);
			return *this;
		}

#ifdef GL_USE_LOCAL_DB
		[[nodiscard]] static auto sql_get_callback(keep_list_t& resource) {
			return [&resource](const db::BIGINT channel_id, const db::BIGINT, const db::TINYINT) {
				resource.channel_ids.push_back(channel_id);
			};
		}

		static keep_list_t sql_keep(const dpp::snowflake guild_id, const std::vector <dpp::snowflake>& channel_ids, const std::string_view context = "") {
			auto result = db::sql << std::format("SELECT channel FROM {0} WHERE {1}; -- {2}", table_name, notification_channel_t::where_guild(guild_id), context);
			if (db::is_error(result)) {
				return {false};
			}
			keep_list_t res = {guild_id, {}};
			std::unordered_set <dpp::snowflake> channel_id_set;
			std::ranges::transform(channel_ids.begin(), channel_ids.end(), std::inserter(channel_id_set, channel_id_set.begin()), [](const dpp::snowflake value) {
				return value;
			});
			result >> [&channel_id_set, &res](const db::BIGINT channel_id) {
				if (channel_id_set.contains(channel_id)) {
					db::destroy <notification_channel_t, "", where_channel>(channel_id).sync_wait();
					res.channel_ids.emplace_back(channel_id);
				}
			};
			return res;
		}
#endif
	};
};

void fill_from_json_impl(notification_channel_t& notification_channel, const nlohmann::json& json);
nlohmann::json to_json_impl(const notification_channel_t& notification_channel);

void fill_from_json_impl(notification_channel_t::keep_list_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const notification_channel_t::keep_list_t& bulk_list);

#endif
