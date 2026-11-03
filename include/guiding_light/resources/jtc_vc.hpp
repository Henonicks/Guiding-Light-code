#ifndef JTC_VC_H
#define JTC_VC_H

#include "guiding_light/resources/base_resource.hpp"
#include "guiding_light/guiding_light.hpp"

struct jtc_vc_t : base_resource_t {
	using base_resource_t::base_resource_t;

	static constexpr std::string_view table_name = "jtc_vcs";
	inline static const std::vector <std::string_view> field_list = {"channel", "guild", "name", "vc_limit", "bitrate"};

	dpp::snowflake channel_id, guild_id;
	std::string name;
	int8_t limit{};
	int16_t bitrate{};

	inline static std::mutex keep_mutex;
	inline static std::unordered_set <guild_snowflake> kept_guilds;

	inline static std::recursive_mutex cache_mutex;
	inline static std::unordered_map <channel_snowflake, jtc_vc_t> cache;
	constexpr void put_in_cache(const bool overwrite = true) const {
		std::unique_lock L(cache_mutex);
		if (!overwrite && lookup_in_cache(this->channel_id).is_valid()) {
			return;
		}
		cache[this->channel_id] = *this;
	}
	[[nodiscard]] static constexpr jtc_vc_t lookup_in_cache(const dpp::snowflake channel_id) {
		std::unique_lock L(cache_mutex);
		return cache.contains(channel_id) ? cache[channel_id] : jtc_vc_t(false);
	}
	static constexpr void delete_from_cache(const dpp::snowflake channel_id) {
		std::unique_lock L(cache_mutex);
		cache.erase(channel_id);
	}

	jtc_vc_t() = default;
	jtc_vc_t(const dpp::snowflake channel_id, const dpp::snowflake guild_id, const std::string_view name, const int8_t limit, const int16_t bitrate)
		: channel_id(channel_id), guild_id(guild_id), name(name), limit(limit), bitrate(bitrate) {}

	[[nodiscard]] static constexpr std::string where(const dpp::snowflake channel_id) {
		return "channel=" + channel_id.str();
	}

    [[nodiscard]] std::string insert() const;

	[[nodiscard]] constexpr bool exists() const {
		return !this->channel_id.empty();
	}

#ifdef GL_USE_LOCAL_DB
	static jtc_vc_t sql_create(const jtc_vc_t& resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("INSERT INTO {0} VALUES (?, ?, ?, ?, ?); -- {1}", table_name, context)
			<< resource.channel_id.str() << resource.guild_id.str() << resource.name << resource.limit << resource.bitrate
		);
		if (failure) {
			return {false};
		}
		return resource;
	}

	[[nodiscard]] static auto sql_get_callback(jtc_vc_t& resource) {
		return [&resource](const db::BIGINT channel_id, const db::BIGINT guild_id, const std::string& name, const db::TINYINT limit, const db::MEDIUMINT bitrate) {
			resource.channel_id = channel_id;
			resource.guild_id = guild_id;
			resource.name = name;
			resource.limit = limit;
			resource.bitrate = bitrate;
		};
	}

	static jtc_vc_t sql_update(const jtc_vc_t& resource, const std::string_view context = "") {
		const bool failure = db::is_error(db::sql <<
			std::format("UPDATE {0} SET name=?, vc_limit=?, bitrate=? WHERE {1}; -- {2}", table_name, where(resource.channel_id), context)
			<< resource.name << resource.limit << resource.bitrate
		);
		if (failure) {
			return {false};
		}
		return resource;
	}
#endif

	struct keep_list_t : base_resource_t {
		using base_resource_t::base_resource_t;

		dpp::snowflake guild_id;
		std::vector <dpp::snowflake> channel_ids;

		constexpr void delete_from_cache() const {
			for (const dpp::snowflake x : channel_ids) {
				jtc_vc_t::delete_from_cache(x);
			}
		}

		keep_list_t() = default;
		keep_list_t(keep_list_t&) = default;
		keep_list_t(const keep_list_t&) = default;
		keep_list_t(const dpp::snowflake guild_id, const std::vector <dpp::snowflake>& channel_ids)
			: guild_id(guild_id), channel_ids(channel_ids) {}

		keep_list_t& operator =(const keep_list_t&) = default;
		keep_list_t& operator =(keep_list_t&& other) noexcept {
			this->guild_id = other.guild_id;
			this->channel_ids = std::move(other.channel_ids);
			return *this;
		}

		[[nodiscard]] static constexpr std::string where(const dpp::snowflake guild_id) {
			return "guild=" + guild_id.str();
		}

#ifdef GL_USE_LOCAL_DB
		[[nodiscard]] static auto sql_get_callback(keep_list_t& resource) {
			return [&resource](const db::BIGINT channel_id, const db::BIGINT, const std::string&, const db::TINYINT, const db::MEDIUMINT) {
				resource.channel_ids.emplace_back(channel_id);
			};
		}

		static keep_list_t sql_keep(const dpp::snowflake guild_id, const std::vector <dpp::snowflake>& channel_ids, const std::string_view context = "") {
			auto result = db::sql << std::format("SELECT channel FROM {0} WHERE {1}; -- {2}", table_name, jtc_vc_t::where(guild_id), context);
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
					db::destroy <jtc_vc_t>(channel_id).sync_wait();
					res.channel_ids.emplace_back(channel_id);
				}
			};
			return res;
		}
#endif
	};

	struct count_t : base_resource_t {
		using base_resource_t::base_resource_t;

#ifdef GL_USE_LOCAL_DB
		static constexpr const std::string_view& table_name = jtc_vc_t::table_name;
#else
		static constexpr std::string_view table_name = "count_jtc_vcs_in_guild";
#endif

		dpp::snowflake guild_id;
		int8_t count{};

		count_t() = default;
		count_t(const dpp::snowflake guild_id, const int8_t count)
			: guild_id(guild_id), count(count) {}

		[[nodiscard]] static constexpr std::string where(const dpp::snowflake guild_id) {
			return "guild=" + guild_id.str();
		}

#ifdef GL_USE_LOCAL_DB
		[[nodiscard]] static auto sql_get_callback(count_t& resource) {
			return [&resource](const db::BIGINT, const db::BIGINT, const std::string&, const db::TINYINT, const db::MEDIUMINT) {
				++resource.count;
			};
		}
#endif
	};
};

void fill_from_json_impl(jtc_vc_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const jtc_vc_t& jtc_vc);

void fill_from_json_impl(jtc_vc_t::keep_list_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const jtc_vc_t::keep_list_t& bulk_list);

void fill_from_json_impl(jtc_vc_t::count_t& res, const nlohmann::json& json);
nlohmann::json to_json_impl(const jtc_vc_t::count_t& count);

#endif
