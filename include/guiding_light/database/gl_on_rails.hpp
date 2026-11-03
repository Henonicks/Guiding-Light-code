#ifndef GL_ON_RAILS_H
#define GL_ON_RAILS_H

#include "guiding_light/internal/logging.hpp"
#include "guiding_light/internal/config_values.hpp"

#include "guiding_light/json/json_interface.hpp"

#include <expected>

namespace gor {
	inline constexpr char application_json[] = "application/json";

	inline constexpr std::array request_verbs {
		"GET",
		"POST",
		"PUT",
		"PATCH",
		"DELETE"
	};

	struct v1 {
		inline static std::string BASE_API_URL;
		inline static std::string BASE_API_URL_DEV;
	};

	class poker {
		std::string token;
		dpp::cluster* poker_cluster{};
		bool is_user_cluster{};
	public:
		poker() = delete;
		explicit poker(std::string_view tkn, dpp::cluster* cluster = nullptr);

		template <typename T, typename api_version = v1>
		dpp::coroutine <std::expected <T, dpp::http_request_completion_t>> poke(const std::string& path, const dpp::http_method method, const std::string_view request_body = "") {
			const dpp::http_request_completion_t request = co_await poker_cluster->co_request((!IS_DEV ? api_version::BASE_API_URL : api_version::BASE_API_URL_DEV) + path, method, request_body.data(), application_json, {
				{"Authorization", this->token}
			});
			if (request.status / 100 != 2) {
				if (request.status == 0) {
					error_log("Failed to access the database!");
				}
				log(std::format("Error on {0} request to {1}.\nBody: {2}\nStatus: {3}", request_verbs[method], path, request.body, request.status));
				co_return std::unexpected(request);
			}
			if constexpr (std::is_same_v <T, void>) {
				co_return {};
			}
			else if constexpr (std::is_same_v <T, nlohmann::json>) {
				co_return nlohmann::json::parse(request.body);
			}
			else {
				co_return fill_from_json <T>(nlohmann::json::parse(request.body));
			}
		}

		~poker();
	};
}

[[nodiscard]] gor::poker* get_gor_client();

#endif
