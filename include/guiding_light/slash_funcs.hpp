#ifndef SLASH_FUNCS_H
#define SLASH_FUNCS_H

#include "guiding_light/logging.hpp"
#include "guiding_light/temp_vc.hpp"

/**
 * @brief Every function that's used directly in the slashcommand handler.
 */
namespace slash {
	inline bool enabled = false;

	inline std::map <std::string, std::map <dpp::snowflake, bool>> in_progress; // finished states of commands

	inline std::map <std::string, dpp::slashcommand> global_created, guild_created;
	inline std::vector <dpp::embed> help_embeds;

	/**
	 * @brief set a JTC value - either default or current one.
	 */
	namespace set {
		/**
		 * @brief set a default value of a VCs that belong to a JTC.
		 * @param event The slashcommand event object which contains information about the request.
		 */
		dpp::coroutine <> default_values(const dpp::slashcommand_t& event);

		/**
		 * @brief set a value of current temporary VC.
		 * @param event The slashcommand event object which contains information about the request.
		 */
		dpp::coroutine <> current(const dpp::slashcommand_t& event);
	}

	/**
	 * @brief handles a channel setup request.
	 * @param event The slashcommand event object which contains information about the request.
	*/
	dpp::coroutine <> setup(const dpp::slashcommand_t& event);

	/**
	 * @brief Functions that control or check the blocklists
	 */
	namespace list {
		/**
		 * @brief Adds a user to a list.
		 * @param event The slashcommand event object which contains information about the request.
		 * @param rest_type The type of the list to add the user to. Refer to the rest_types enum.
		 */
		dpp::coroutine <> add(const dpp::slashcommand_t& event, restrictions_types rest_type);

		/**
		 * @brief Removes a user from a blocklist.
		 * @param event The slashcommand event object which contains information about the request.
		 * @param rest_type The type of the list to add the user to. Refer to the rest_types enum.
		 */
		dpp::coroutine <> remove(const dpp::slashcommand_t& event, restrictions_types rest_type);

		/**
		 * @brief Checks if a user is in a blocklist.
		 * @param event The slashcommand event object which contains information about the request.
		 * @param rest_type The type of the list to add the user to. Refer to the rest_types enum.
		 */
		void status(const dpp::slashcommand_t& event, restrictions_types rest_type);
	}

	/**
	 * @brief Functions, related to top.gg votes.
	 */
	namespace topgg {
		/**
		 * @brief Gets the guild in favour of which the user is voting for the bot.
		 * @param event The slashcommand event object which contains information about the request.
		*/
		void guild_get(const dpp::slashcommand_t& event);

		/**
		 * @brief Sets the guild in favour of which the user is voting for the bot.
		 * @param event The slashcommand event object which contains information about the request.
		 */
		void guild_set(const dpp::slashcommand_t& event);

		/**
		 * @brief Gets the combined voting progress of a guild.
		 * @param event The slashcommand event object which contains information about the request.
		 */
		void get_progress(const dpp::slashcommand_t& event);
	}

	/**
	 * Functions for handling ticket opening and closing.
	 */
	namespace ticket {
		/**
		 * @brief Creates a ticket.
		 * @param event The slashcommand event object which contains information about the request.
		 */
		dpp::coroutine <> create(const dpp::slashcommand_t& event);

		/**
		 * @brief Closes a ticket.
		 * @param event The slashcommand event object which contains information about the request.
		 */
		void close(const dpp::slashcommand_t& event);
	}
}

#endif