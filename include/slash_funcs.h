#ifndef SLASH_FUNCS_H
#define SLASH_FUNCS_H

#include <set>
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "temp_vc.h"
#include "notification_channel.h"
#include "guidingLight/guiding_light.h"
#include "file_namespace.h"
#include "configuration.h"
#include "logging.h"
#include "topgg.h"

namespace slash {

    extern bool enabled;

	extern std::map <std::string, dpp::slashcommand> created_slashcommands;
    extern dpp::embed help_embed;
    
    /**
     * @brief set a JTC value - either default or current one.
     */
	namespace set {

        /**
         * @brief set a default value of a VCs that belong to a JTC.
         * @param bot cluster which gets the request and handles it.
         * @param event the slashcommand event object which contains information about request.
         */
        dpp::coroutine <void> default_values(dpp::cluster& bot, const dpp::slashcommand_t& event);

        /**
         * @brief set a value of current temporary VC.
         * @param bot cluster which gets the request and handles it.
         * @param event the slashcommand event object which contains information about request.
         */
        void current(dpp::cluster& bot, const dpp::slashcommand_t& event);
    }

    /**
     * @brief handles a channel setup request.
     * @param bot cluster which handles a channel setup request.
     * @param event the slashcommand event object which contains information about request.
     * @param jtc_vcs list of all JTC channels' ID's.
     * @param jtc_channels_map list of all JTC channels keyed by their ID and valued by their channel objects.
     * @param jtc_default_values list of default values for JTC channels (their default name, user limit and bitrate).
     * @param ntif_channels list of all notification channels where the bot would send message about creating a temporary voice channel for someone.
    */
    dpp::coroutine <void> setup(dpp::cluster& bot, const dpp::slashcommand_t& event);

    namespace blocklist {
        dpp::coroutine <void> add(const dpp::slashcommand_t& event);
        dpp::coroutine <void> remove(const dpp::slashcommand_t& event);
        void status(const dpp::slashcommand_t& event);
    }

    namespace topgg {
        void guild_get(const dpp::slashcommand_t& event);
        void guild_set(const dpp::slashcommand_t& event);
        void get_progress(const dpp::slashcommand_t& event);
    }
}

#endif