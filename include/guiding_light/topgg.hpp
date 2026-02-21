#ifndef TOPGG_H
#define TOPGG_H

#include "guiding_light/notification_channel.hpp"

std::string cleanup_request_path(std::string_view path);

namespace topgg {
	inline std::map <user_snowflake, guild_snowflake> guild_choices;
	inline std::map <guild_snowflake, int> guild_votes_amount;
	inline constexpr std::array <int, 10> votes_leveling =
		{0, 225, 450, 800, 1300, 2000, 2750, 3350, 4350, 5000};
	inline std::map <user_snowflake, bool> no_noguild_reminder;

	void handle_request_if_topgg(dpp::http_server_request* request);

	int16_t send_vote_info(const nlohmann::json& info_json);

	/**
	 * @brief Handles a top.gg vote for the bot.
	 * @param user_id The ID of the user who voted.
	 * @param weight The weight of the vote, e.g. the amount of points it's worth.
	 * top.gg states that it's always worth 1 except for when it's 2, which is on weekends.
	 * @return True if the user voted in favour of a guild, false otherwise.
	 */
	bool vote(const dpp::snowflake& user_id, int8_t weight);

	namespace jtc {
		/**
		 * @brief Counts the amount of allowed JTCs in a guild.
		 * @param guild_id The ID of the guild where the JTCs are counted.
		 * @return The amount of the JTCs.
		 */
		int8_t count_allowed_jtcs(const dpp::snowflake& guild_id);

		/**
		 * @brief Gets the requirement for the next level in a guild.
		 * @param guild_id The ID of the guild to get the requirement for.
		 * @return The requirement for the next level, or 0 if the maximum level has already been reached.
		 */
		int get_next_lvl_req(const dpp::snowflake& guild_id);
	}
}

#endif
