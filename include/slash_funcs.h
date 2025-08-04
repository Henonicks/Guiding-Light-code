#ifndef SLASH_FUNCS_H
#define SLASH_FUNCS_H

#include <set>
#include "configuration.h"
#include "ticket.h"
#include "temp_vc.h"

namespace slash {

	extern bool enabled;

	extern std::map <std::string, std::map <dpp::snowflake, bool>> in_progress; // finished states of commands

	extern std::map <std::string, dpp::slashcommand> created_slashcommands;
	extern dpp::embed help_embed_1;
	extern dpp::embed help_embed_2;
	
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
		dpp::coroutine <void> current(dpp::cluster& bot, const dpp::slashcommand_t& event);
	}

	/**
	 * @brief handles a channel setup request.
	 * @param bot cluster which handles a channel setup request.
	 * @param event the slashcommand event object which contains information about request.
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

	namespace ticket {
		dpp::coroutine <void> create(const dpp::slashcommand_t& event);
		void close(const dpp::slashcommand_t& event);
	}

}

#endif