#include "guidingLight/guiding_light.h"

dpp::cluster *bot, *bot_dev, *bot_release;
bool bot_dev_is_starting, bot_release_is_starting, *bot_is_starting;

void get_bot(dpp::cluster*& bot) {
    bot = &*(!IS_DEV ? bot_release : bot_dev);
}

bool is_running() {
    return !bot->me.id.empty();
}

std::string bot_name() {
    return !IS_DEV ? "Guiding Light" : "Curious Light";
}
