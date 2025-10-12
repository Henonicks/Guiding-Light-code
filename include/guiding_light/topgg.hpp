#ifndef TOPGG_H
#define TOPGG_H

#include "notification_channel.hpp"

namespace topgg {
	inline int last_collection_time = 0;
	inline std::map <user_snowflake, guild_snowflake> guild_choices;
	inline std::map <guild_snowflake, int> guild_votes_amount;
	inline constexpr std::array <int, 10> votes_leveling =
		{0, 225, 450, 800, 1300, 2000, 2750, 3350, 4350, 5000};
	inline std::map <user_snowflake, bool> no_noguild_reminder;

	/**
	 * @brief Handles a top.gg vote for the bot.
	 * @param user_id The ID of the user who voted.
	 * @param weekend True if it's a weekend, false otherwise.
	 * @return True if the user voted in favour of a guild, false otherwise.
	 */
	bool vote(const dpp::snowflake& user_id, const bool& weekend);

	namespace jtc {
		/**
		 * @brief Counts the amount of allowed JTCs in a guild.
		 * @param guild_id The ID of the guild where the JTCs are counted.
		 * @return The amount of the JTCs.
		 */
		int8_t count_allowed_jtcs(const dpp::snowflake& guild_id);
	}
}

#endif
