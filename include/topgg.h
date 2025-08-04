#ifndef TOPGG_H
#define TOPGG_H

#include <dpp/cluster.h>
#include "notification_channel.h"

namespace topgg {
	extern int last_collection_time;
	extern std::map <user_snowflake, guild_snowflake> guild_choices;
	extern std::map <guild_snowflake, int> guild_votes_amount;
	extern std::vector <int> votes_leveling;
	extern std::map <user_snowflake, bool> no_noguild_reminder;

	struct guild_choice {
		dpp::snowflake user_id, guild_id;
	};

	struct guild {
		dpp::snowflake guild_id;
		int votes;
	};

	bool operator <(topgg::guild_choice vc1, topgg::guild_choice vc2);
	bool operator <(topgg::guild vc1, topgg::guild vc2);

	guild_choice get_guild_choice(std::string_view line);
	guild get_guild_votes_amount(std::string_view line);

	bool vote(const dpp::snowflake& user_id, const bool& weekend, dpp::cluster& bot);

	namespace jtc {
		int8_t count_jtcs(const dpp::snowflake& guild_id);
	}
}

#endif
