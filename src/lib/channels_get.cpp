#include <channels_get.h>

void channels_get(dpp::cluster& bot) {
    dpp::snowflake guildid;
    std::cin >> guildid;
    bot.log(dpp::loglevel::ll_info, "Getting channels of guild with ID " + std::to_string(guildid) + ":\n 0 is text;" "1 is DM;" "2 is voice;" "3 is group DM;" "4 is category;" "5 is announcement;" "6 is store;" "10 is announcement thread;" "11 is public thread;" "12 is private thread;" "13 is stage;" "14 is directory;" "15 is forum;");
    auto channels = bot.channels_get_sync(guildid);
    for (auto id : channels) {
        std::string name = std::to_string(id.second.get_type()) + "  " + id.second.name;
        std::cout << id.first << "   " + name << std::endl;
    }
}