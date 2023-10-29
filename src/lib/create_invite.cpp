#include <create_invite.h>

void create_invite(dpp::cluster& bot) {
    dpp::invite invite;
    dpp::snowflake channelid;
    std::cin >> channelid;
    dpp::channel channel = bot.channel_get_sync(channelid);
    bot.channel_invite_create(channel, invite, [&bot](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error()) {
            bot.log(dpp::loglevel::ll_error, "Bozo");
            return;
        }
        dpp::invite invite = std::get <dpp::invite>(callback.value);
        bot.log(dpp::loglevel::ll_info, fmt::format("https://discord.gg/{}", invite.code));
    });
}