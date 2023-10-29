#pragma once
#include <guidingLight/guiding_light.h>

/**
 * @brief Creates invitation to a channel using its ID inputted into console if the bot has access to the channel. If couldn't, prints "Bozo" on the console.
 * @param bot The bot which will create an invite
 */
void create_invite(dpp::cluster& bot);