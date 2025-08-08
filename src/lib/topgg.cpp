#include "topgg.h"

int topgg::last_collection_time = 0;
std::map <user_snowflake, guild_snowflake> topgg::guild_choices;
std::map <guild_snowflake, int> topgg::guild_votes_amount;
std::vector <int> topgg::votes_leveling = {0, 225, 450, 800, 1300, 2000, 2750, 3350, 4350, 5000};
std::map <user_snowflake, bool> topgg::no_noguild_reminder;

bool topgg::vote(const dpp::snowflake& user_id, const bool& weekend) {
	bool failure = false;
	dpp::snowflake guild_id = topgg::guild_choices[user_id];
	if (dpp::find_guild(guild_id) == nullptr) {
		failure = true;
		return failure;
	}
	guild_votes_amount[guild_id] += weekend;
	db::sql << "DELETE FROM topgg_guild_votes_amount WHERE guild_id=?;" << guild_id.str();
	db::sql << "INSERT INTO topgg_guild_votes_amount VALUES (?, ?);" << guild_id.str() << ++guild_votes_amount[guild_id];
	dpp::snowflake channel_id = topgg_notifications[guild_id];
	bot->message_create(dpp::message(channel_id, fmt::format("<@{0}> has voted.{1}", user_id, weekend ? " A bonus point is granted as today is a weekend!" : "")));
	return failure;
}

int8_t topgg::jtc::count_jtcs(const dpp::snowflake& guild_id) {
	const int8_t& guild_votes_amount = ::topgg::guild_votes_amount[guild_id];
	int8_t i = 1;
	for (;i < 10; i++) {
		if (::topgg::votes_leveling[i] > guild_votes_amount) {
			break;
		}
	}
	return i;
}
