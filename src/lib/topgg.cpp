#include "topgg.h"

bool operator <(topgg::guild_choice vc1, topgg::guild_choice vc2) {
    unsigned long long c1 = vc1.user_id, c2 = vc2.user_id;
    return c1 < c2;
}

bool operator <(topgg::guild vc1, topgg::guild vc2) {
    int c1 = vc1.votes, c2 = vc2.votes;
    return c1 < c2;
}

int topgg::last_collection_time = 0;
std::map <user_snowflake, guild_snowflake> topgg::guild_choices;
std::map <guild_snowflake, int> topgg::guild_votes_amount;
std::vector <int> topgg::votes_leveling = {0, 225, 450, 800, 1300, 2000, 2750, 3350, 4350, 5000};
std::map <user_snowflake, bool> topgg::no_noguild_reminder;

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

topgg::guild topgg::get_guild_votes_amount(std::string_view line) {
	topgg::guild result;
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
    guild_votes_amount[guild_id] += weekend;
    db::sql << "DELETE FROM topgg_guild_votes_amount WHERE guild_id=?;" << guild_id.str();
    db::sql << "INSERT INTO topgg_guild_votes_amount VALUES (?, ?);" << guild_id.str() << ++guild_votes_amount[guild_id];
    dpp::snowflake channel_id = topgg_notifications[guild_id];
    bot.message_create(dpp::message(channel_id, fmt::format("<@{0}> has voted.{1}", user_id, weekend ? " A bonus point is granted as today is a weekend!" : "")));
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
