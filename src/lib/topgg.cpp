#include "guiding_light/topgg.hpp"
#include "guiding_light/database.hpp"
#include "guiding_light/logging.hpp"

bool topgg::vote(const dpp::snowflake& user_id, const bool& weekend) {
	log(fmt::format("User {} just voted, hell yeah!", user_id));
	bool failure = false;
	const dpp::snowflake& guild_id = guild_choices[user_id];
	if (dpp::find_guild(guild_id) == nullptr) {
		log("But they haven't selected a guild!");
		failure = true;
		return failure;
	}
	++guild_votes_amount[guild_id];
	guild_votes_amount[guild_id] += weekend;
	log(fmt::format("Therefore, the guild {0} now has {1} votes!", guild_id, guild_votes_amount[guild_id]));
	// The bot that's getting voted for gets a bonus vote if it's a weekend for every vote.
	db::sql << "DELETE FROM topgg_guild_votes_amount WHERE guild_id=?;" << guild_id.str();
	db::sql << "INSERT INTO topgg_guild_votes_amount VALUES (?, ?);" << guild_id.str() << guild_votes_amount[guild_id];
	const dpp::snowflake& channel_id = topgg_notifications[guild_id];
	if (!channel_id.empty()) {
		bot->message_create(dpp::message(channel_id, fmt::format("<@{0}> has voted.{1}", user_id, weekend ? " A bonus point is granted as today is a weekend!" : "")), error_callback);
	}
	return failure;
}

int8_t topgg::jtc::count_allowed_jtcs(const dpp::snowflake& guild_id) {
	const int& votes = guild_votes_amount[guild_id];
	int8_t i = 1;
	for (;i < cast <int8_t>(votes_leveling.size()); i++) {
		if (votes_leveling[i] > votes) {
			break;
		}
	}
	return i;
}
