#ifndef TOPGG_H
#define TOPGG_H

#include <dpp/nlohmann/json.hpp>
#include <dpp/coro/coroutine.h>

namespace topgg {
	inline std::vector <int> votes_leveling;

	/**
	 * @brief Handles a top.gg vote for the bot.
	 * @param user_id The ID of the user who voted.
	 * @param weight The weight of the vote, e.g. the amount of points it's worth.
	 * top.gg states that it's always worth 1 except for when it's 2, which is on weekends.
	 * @param guild_id If already known, pass the guild they're voting in favour of.
	 * @return True if the user voted in favour of a guild, false otherwise.
	 */
	dpp::coroutine <bool> vote(dpp::snowflake user_id, int8_t weight, dpp::snowflake guild_id = 1);

	namespace jtc {
		/**
		 * @brief Counts the amount of allowed JTCs in a guild. Helper function that uses the one taking in the amount of votes.
		 * @param guild_id The ID of the guild where the JTCs are counted.
		 * @return The amount of allowed JTCs.
		 */
		dpp::coroutine <int8_t> count_allowed_jtcs(dpp::snowflake guild_id);

		/**
		 * @brief Counts the amount of allowed JTCs in a guild.
		 * @param votes The amount of votes the guild has
		 * @return The amount of allowed JTCs.
		 */
		int8_t count_allowed_jtcs(int votes);

		/**
		 * @brief Gets the requirement for the next level in a guild.
		 * @param guild_id The ID of the guild to get the requirement for.
		 * @return The requirement for the next level, or 0 if the maximum level has already been reached.
		 */
		dpp::coroutine <int> get_next_lvl_req(dpp::snowflake guild_id);

		/**
		 * @brief Gets the requirement for the next level in a guild.
		 * @param curr_allowed The current amount of allowed JTCs in a guild.
		 * @return The requirement for the next level, or 0 if the maximum level has already been reached.
		 */
		int get_next_lvl_req(int curr_allowed);
	}
}

#endif
