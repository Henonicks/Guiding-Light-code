#include "guidingLight/guiding_light.h"

dpp::cluster *bot, *bot_dev, *bot_release;
bool bot_dev_is_starting, bot_release_is_starting, *bot_is_starting;

dpp::cluster* get_bot() {
    return !IS_DEV ? bot_release : bot_dev;
}

bool is_running() {
    return !bot->me.id.empty();
    // We get the ID when the bot is ready, so if we have
    // the ID already then the bot is definitely running.
}

std::string bot_name() {
    return !IS_DEV ? "Guiding Light" : "Curious Light";
    // See README.md if you wonder why these names.
}
