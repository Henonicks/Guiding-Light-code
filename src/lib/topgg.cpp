#include "topgg.h"

bool operator <(topgg::guild_choice vc1, topgg::guild_choice vc2) {
    unsigned long long c1 = vc1.user_id, c2 = vc2.user_id;
    return c1 < c2;
}

bool operator <(topgg::guild_votes_amount vc1, topgg::guild_votes_amount vc2) {
    int c1 = vc1.votes, c2 = vc2.votes;
    return c1 < c2;
}

int topgg::last_collection_time = 0;
std::map <dpp::snowflake, dpp::snowflake> topgg::guild_choices;
std::map <dpp::snowflake, int> topgg::guild_list;
std::vector <int> topgg::votes_leveling = {0, 2, 450, 800, 1300, 2000, 2750, 3350, 4350, 5000};
std::map <dpp::snowflake, bool> topgg::noguild_reminders;

topgg::guild_choice topgg::get_guild_choice(std::string_view line) {
	topgg::guild_choice result;
    std::string user_id_line, guild_id_line;
    int i = 0;
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            user_id_line += line[i];
        }
        else {
            ++i;
            result.user_id = (dpp::snowflake)user_id_line;
            break;
        }
    }
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            guild_id_line += line[i];
        }
        else {
            ++i;
            break;
        }
    }
    result.guild_id = (dpp::snowflake)guild_id_line;
    //std::cout << result.user_id << ' ' << result.guild_id << ' ' << '\n';
    return result;
}

topgg::guild_votes_amount topgg::get_guild_votes_amount(std::string_view line) {
	topgg::guild_votes_amount result;
    std::string guild_id_line, votes_line;
    int i = 0;
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            guild_id_line += line[i];
        }
        else {
            ++i;
            result.guild_id = (dpp::snowflake)guild_id_line;
            break;
        }
    }
    for (;i < line.size(); i++) {
        if (line[i] != ' ') {
            votes_line += line[i];
        }
        else {
            ++i;
            break;
        }
    }
    result.votes = (dpp::snowflake)votes_line;
    return result;
}

bool topgg::vote(const dpp::snowflake& user_id, const bool& weekend, dpp::cluster& bot) {
    bool failure = false;
    dpp::snowflake guild_id = topgg::guild_choices[user_id];
    if (dpp::find_guild(guild_id) == nullptr) {
        failure = true;
        return failure;
    }
    file::delete_line_once(fmt::format("{0} {1}", guild_id, guild_list[guild_id]), file::topgg_guild_votes_amount);
    guild_list[guild_id] += weekend;
    file::line_append(fmt::format("{0} {1}", guild_id, ++guild_list[guild_id]), file::topgg_guild_votes_amount);
    dpp::snowflake channel_id = topgg_ntif_chnls[guild_id];
    bot.message_create(dpp::message(channel_id, fmt::format("<@{0}> has voted.{1}", user_id, weekend ? " A bonus point is granted as today is a weekend!" : "")));
    return failure;
}

int8_t topgg::jtc::count_jtcs(const dpp::snowflake& guild_id) {
    const int& guild_votes_amount = ::topgg::guild_list[guild_id];
    for (int i = 1; i < 10; i++) {
        if (::topgg::votes_leveling[i] > guild_votes_amount) {
			return i;
        }
    }
    return 10;
}
