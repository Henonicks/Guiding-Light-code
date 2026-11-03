#include "guiding_light/handlers/topgg.hpp"

#include "guiding_light/database/database.hpp"
#include "guiding_light/internal/logging.hpp"
#include "guiding_light/resources/notification_channel.hpp"
#include "guiding_light/resources/topgg_guild_choice.hpp"
#include "guiding_light/resources/topgg_guild_vote_amount.hpp"
#include "guiding_light/resources/noguild_reminder_blocker.hpp"
#include "guiding_light/helpers/format.tpp"

dpp::coroutine <bool> topgg::vote(const dpp::snowflake user_id, const int8_t weight, dpp::snowflake guild_id) {
	log(std::format("User {} just voted, hell yeah!", user_id));
	bool failure = false;
	if (guild_id == 1) {
		guild_id = (co_await db::get <topgg_guild_choice_t>(user_id)).guild_id;
	}
	if (guild_id.empty()) {
		log("But they haven't selected a guild!");

		// If there's a failure in granting a guild a vote point
		// and the user has not been notified about that before,
		// notify them.
		const auto blocker = co_await db::get <noguild_reminder_blocker_t>(user_id);
		if (!blocker.exists() && blocker.is_valid()) {
			bot->direct_message_create(user_id, dpp::message(
	            "You have just voted and missed out on the chance to vote in favour of a guild! "
	            "Choosing a guild with `/guild set` and voting for me on top.gg grants it guild points which can then be turned into JTC VCs!"),
	        error_callback);
			co_await db::create <noguild_reminder_blocker_t>({user_id});
		}
		failure = true;
		co_return failure;
	}
	const int votes = (co_await db::get <topgg_guild_vote_amount_t>(guild_id)).votes;
	const int8_t old_allowed = jtc::count_allowed_jtcs(votes - weight);
	const int8_t new_allowed = jtc::count_allowed_jtcs(votes);
	log(std::format("Therefore, the guild {0} now has {1} votes!", guild_id, votes));
	const dpp::snowflake channel_id = (co_await db::get <notification_channel_t>(guild_id, ns_topgg)).channel_id;
	if (!channel_id.empty()) {
		const int next_req = co_await jtc::get_next_lvl_req(guild_id);
		bot->message_create(dpp::message(channel_id,
			std::format("<@{0}> has voted.{1} {2}{3}",
				user_id,
				(weight == 2 ? " A bonus point has been granted as today is a weekend!" : ""),
				(next_req ? std::format("Current progress: {0}/{1}. ", votes, next_req) : ""),
				(old_allowed < new_allowed ? std::format("The JTC limit in this server is now {}.", new_allowed) : "")
			)
		), error_callback);
	}
	co_return failure;
}

dpp::coroutine <int8_t> topgg::jtc::count_allowed_jtcs(const dpp::snowflake guild_id) {
	const int votes = (co_await db::get <topgg_guild_vote_amount_t>(guild_id)).votes;
	co_return count_allowed_jtcs(votes);
}

int8_t topgg::jtc::count_allowed_jtcs(const int votes) {
	int8_t i = 1;
	for (; i < cast <int8_t>(votes_leveling.size()); i++) {
		if (votes_leveling[i] > votes) {
			break;
		}
	}
	return i;
}

dpp::coroutine <int> topgg::jtc::get_next_lvl_req(const dpp::snowflake guild_id) {
	const int8_t allowed_jtcs = co_await count_allowed_jtcs(guild_id);
	co_return get_next_lvl_req(allowed_jtcs);
}

int topgg::jtc::get_next_lvl_req(const int curr_allowed) {
	return curr_allowed >= cast <int8_t>(votes_leveling.size()) ? 0 : votes_leveling[curr_allowed];
}
