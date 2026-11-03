#ifndef DATABASE_H
#define DATABASE_H

#include <unordered_set>
#include <mutex>

#include "no_warns/sqlite.hpp"

#include "guiding_light/guiding_light.hpp"
#include "guiding_light/database/gl_on_rails.hpp"

#include "guiding_light/json/json_interface.hpp"
#include "guiding_light/helpers/format.tpp"
#include "guiding_light/helpers/variant_iterator.tpp"

namespace db {
	inline constexpr char LOCATION[] = "../database";
	inline constexpr char SELECT_LOCATION[] = "../database/select";

	inline constexpr char LOOKUP_SEPARATOR[] =
#ifdef GL_USE_LOCAL_DB
		" AND "
#else
		"&"
#endif
	;
#ifdef GL_USE_LOCAL_DB
	using TINYINT = int16_t; // bro why is there no int8_t which isn't a char in c++ twin icl ts pmo lwk 💔🥀
	using SMALLINT = int16_t;
	using MEDIUMINT = int; // 24-bit ints also don't exist i think 💔🥀
	using BIGINT = uint64_t;
	// SQL types that I use

	class wrapper : public sqlite::database {
	public:
		class operator_with_error {
		public:
			wrapper& _wrapper;
			operator_with_error() = delete; // the whole point of this class is to have a ref to the parent
			explicit operator_with_error(wrapper&);
			sqlite::database_binder operator <<(sqlite::str_ref query) const;
		};
		using database::database;
		using database::operator=;

		operator_with_error with_error{*this};

		/**
		 * @brief Handle an SQL query and catch any exceptions on failure.
		 * @param _query The SQL query to handle.
		 * @return I've honestly got no clue what a database_binder is
		 */
		sqlite::database_binder operator <<(sqlite::str_ref _query);
	};

	[[nodiscard]] bool is_error(sqlite::database_binder& binder);
	[[nodiscard]] bool is_error(sqlite::database_binder&& binder);

	// A set with strings representing the existence of potential
	// pending errors, caught during an SQL query in functions.
	inline std::unordered_set <std::string> errors_pending;
	inline std::recursive_mutex mutex;

	inline wrapper sql("");

#endif
	/**
	 * @brief Tries to connect to the database and reports whether the connection was successful.
	 * @return True if the connection was successful, false otherwise.
	 */
	bool test_conn(std::string_view path);

	template <typename T>
	concept is_cached = requires(T t) {
		t.cache;
	};

	template <typename T>
	concept has_variable_cachability = requires(T t) {
		t.is_cachable;
	};

	template <typename T>
	concept is_generic = requires {
		typename T::possible_types;
	};

	constexpr char EMPTY_STRING[] = "";

	template <typename T, templatable_string_t context = "">
	dpp::coroutine <T> create(const T& resource) {
		T res;
#ifdef GL_USE_LOCAL_DB
		res = T::sql_create(resource, context);
#else
		const std::string request_body = to_json(resource).dump();
		const auto callback = co_await gor_client->poke <T>(std::string(resource.table_name), dpp::m_post, request_body);
		if (!callback) {
			if (!context.empty()) {
				error_log(std::format("Finished erroneous request with the following context: {}.", context));
			}
			log(std::format("Response body: {}", callback.error().body));
			res = T{false};
		}
		else {
			res = callback.value();
		}
#endif
		if (res.is_valid()) {
			if constexpr (has_variable_cachability <T>) {
				if (!res.is_cachable()) {
					co_return res;
				}
			}
			resource.put_in_cache(false);
		}
		res.valid = true;
		co_return res;
	}

	template <typename T, templatable_string_t context = "", auto* where = T::where, typename ...Args>
	[[nodiscard]] dpp::coroutine <T> get(const Args... resource_id) {
		if constexpr (is_cached <T>) {
			const T resource = T::lookup_in_cache(std::forward <const Args>(resource_id)...);
			if (resource.is_valid()) {
				co_return resource;
			}
		}
#ifdef GL_USE_LOCAL_DB
		auto result = sql << std::format("SELECT * FROM {0} WHERE {1}; -- {2}", T::table_name, where(std::forward <const Args>(resource_id)...), context);
		if (is_error(result)) {
			co_return T{false};
		}
		T res;
		result >> T::sql_get_callback(res);
#else
		const auto callback = co_await gor_client->poke <T>(std::string(T::table_name) + '?' + where(std::forward <const Args>(resource_id)...), dpp::m_get);
		if (!callback) {
			if (!context.empty()) {
				error_log(std::format("Finished erroneous request with the following context: {}.", context));
			}
			log(std::format("Response body: {}", callback.error().body));
			co_return T{callback.error().status == 404};
		}
		const T& res = callback.value();
#endif
		if constexpr (is_cached <T>) {
			if constexpr (has_variable_cachability <T>) {
				if (!res.is_cachable()) {
					co_return res;
				}
			}
			res.put_in_cache(true);
		}
		co_return res;
	}

	template <typename T, templatable_string_t context = "">
	dpp::coroutine <T> update(const T& resource) {
#ifdef GL_USE_LOCAL_DB
		T res = T::sql_update(resource, context);
#else
		const std::string request_body = to_json(resource).dump();
		const std::expected <T, dpp::http_request_completion_t> callback = co_await gor_client->poke <T>(std::string(T::table_name), dpp::m_patch, request_body);
		if (!callback) {
			if (!context.empty()) {
				error_log(std::format("Finished erroneous request with the following context: {}.", context));
			}
			log(std::format("Response body: {}", callback.error().body));
			co_return T{false};
		}
		T res = callback.value();
#endif
		if constexpr (is_cached <T>) {
			if constexpr (has_variable_cachability <T>) {
				if (!res.is_cachable()) {
					co_return res;
				}
			}
			if (res.is_valid()) {
				res.put_in_cache(true);
			}
		}
		co_return res;
	}

	template <typename T, templatable_string_t context = "", auto* where = T::where, typename ...Args, typename = std::enable_if_t <!is_generic <T>>>
	dpp::coroutine <> destroy(const Args... resource_id) {
#ifdef GL_USE_LOCAL_DB
		auto result = sql << std::format("DELETE FROM {0} WHERE {1}; -- {2}", T::table_name, where(std::forward <const Args>(resource_id)...), context);
#else
		const auto callback = co_await gor_client->poke <void>(std::string(T::table_name) + '?' + where(std::forward <const Args>(resource_id)...), dpp::m_delete);
		if (!callback) {
			if (!context.empty()) {
				error_log(std::format("Finished erroneous request with the following context: {}.", context));
			}
			log(std::format("Response body: {}", callback.error().body));
			co_return;
		}
#endif
		if constexpr (is_cached <T>) {
			T::delete_from_cache(std::forward <const Args>(resource_id)...);
		}
		co_return;
	}

	template <typename T, templatable_string_t context = "", auto* where = T::where, typename ...Args, typename = std::enable_if_t <is_generic <T>>>
	dpp::coroutine <T> destroy(const Args... resource_id) {
#ifdef GL_USE_LOCAL_DB
		T res;
		iterate_through_variant <typename T::possible_types>(
			[resource_id...]<typename X>() {
				return !get <X, context, where>(std::forward <const Args>(resource_id)...).sync_wait().is_valid();
			},
			[&res, resource_id...]<typename X>() {
				res = { std::forward <const Args...>(resource_id)..., std::string(X::table_name.substr(0, X::table_name.size() - 1)) };
				destroy <X, context, where>(std::forward <const Args>(resource_id)...).sync_wait();
			}
		);
#else
		const auto callback = co_await gor_client->poke <T>(std::format("{0}?{1}", T::table_name, where(std::forward <const Args>(resource_id)...)), dpp::m_delete);
		if (!callback) {
			if (!context.empty()) {
				error_log(std::format("Finished erroneous request with the following context: {}.", context));
			}
			log(std::format("Response body: {}", callback.error().body));
			co_return T{callback.error().status == 404};
		}
		const auto res = callback.value();
		iterate_through_variant <typename T::possible_types>(
			[&res]<typename X>() {
				return res.resource != X::table_name.substr(0, X::table_name.size() - 1);
			},
			[resource_id...]<typename X>() {
				X::delete_from_cache(std::forward <const Args>(resource_id)...);
			}
		);
#endif
		co_return res;
	}

	template <typename T, templatable_string_t context = "", typename ...Args>
	dpp::coroutine <typename T::keep_list_t> keep(const Args&... resource_id) {
		typename T::keep_list_t deleted;
#ifdef GL_USE_LOCAL_DB
		deleted = T::keep_list_t::sql_keep(std::forward <const Args>(resource_id)..., context);
		if (!deleted.is_valid()) {
			co_return deleted;
		}
#else
		const std::string request_body = to_json <typename T::keep_list_t>(typename T::keep_list_t(std::forward <const Args>(resource_id)...)).dump();
		auto callback = co_await gor_client->poke <typename T::keep_list_t>(std::string(T::table_name) + "/keep", dpp::m_delete, request_body);
		if (!callback) {
			if (!context.empty()) {
				error_log(std::format("Finished erroneous request with the following context: {}.", context));
			}
			log(std::format("Response body: {}", callback.error().body));
			co_return typename T::keep_list_t{false};
		}
		deleted = std::move(callback.value());
#endif
		if constexpr (is_cached <T>) {
			deleted.delete_from_cache();
		}
		co_return deleted;
	}

	// This is only used in CLI mode at the moment. No need to cache here.
	template <typename T, templatable_string_t context = "">
	dpp::coroutine <std::vector <T>> upload(const std::vector <T>& resources) {
		const std::string request_body = to_json(resources).dump();
		const auto callback = co_await gor_client->poke <std::vector <T>>(std::string(T::table_name) + "/upload", dpp::m_post, request_body);
		if (!callback) {
			if (!context.empty()) {
				error_log(std::format("Finished erroneous request with the following context: {}.", context));
			}
			log(std::format("Response body: {}", callback.error().body));
			co_return std::vector <T>{};
		}
		co_return callback.value();
	}
}

#endif
