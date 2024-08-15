#ifndef TOPGG_H
#define TOPGG_H

#include <dpp/cluster.h>
#include "file_namespace.h"
#include "notification_channel.h"

namespace topgg {
	extern int last_collection_time;
	extern std::map <dpp::snowflake, dpp::snowflake> guild_choices;
	extern std::map <dpp::snowflake, int> guild_list;
	extern std::vector <int> votes_leveling;
	extern std::map <dpp::snowflake, bool> noguild_reminders;

	struct guild_choice {
		dpp::snowflake user_id, guild_id;
	};

	struct guild_votes_amount {
		dpp::snowflake guild_id;
		int votes;
	};

	bool operator <(topgg::guild_choice vc1, topgg::guild_choice vc2);
	bool operator <(topgg::guild_votes_amount vc1, topgg::guild_votes_amount vc2);

	guild_choice get_guild_choice(std::string_view line);
	guild_votes_amount get_guild_votes_amount(std::string_view line);

	bool vote(const dpp::snowflake& user_id, const bool& weekend, dpp::cluster& bot);

	namespace jtc {
		int8_t count_jtcs(const dpp::snowflake& guild_id);
	}

}

#endif
