#ifndef SLASH_FUNCS_H
#define SLASH_FUNCS_H

#include "guiding_light/logging.hpp"
#include "guiding_light/temp_vc.hpp"

henifig::value_t get_help_command_page_names(std::string_view lang = "default", const henifig::value_map& commands = cfg::responses["HELP_COMMAND_PAGE_NAMES"]);

/**
 * @brief Every function that's used directly in the slashcommand handler.
 */
namespace slash {
	inline std::atomic enabled = false;

	inline std::unordered_map <std::string, std::unordered_set <dpp::snowflake>> in_progress = {
		{"setup", {}},
		{"ticket", {}}
	}; // finished states of commands
	inline std::shared_mutex in_progress_mutex;

	inline std::unordered_map <std::string, dpp::slashcommand> global_created, guild_created;

	inline dpptgg::v1::slashcommand_array global_vector;

	/**
	 * @brief make an action against a temp VC.
	 */
	namespace tempvc {
		/**
		 * @brief set a value of current temporary VC.
		 * @param event The slashcommand event object which contains information about the request.
		 */
		dpp::coroutine <> set(const dpp::slashcommand_t& event);

		/**
		 * @brief Functions that control or check the restriction lists
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
	}

	/**
	 * @brief set a default JTC value.
	 */
	namespace set {
		/**
		 * @brief set a default value of the VCs that will be created from a JTC.
		 * @param event The slashcommand event object which contains information about the request.
		 */
		dpp::coroutine <> default_values(const dpp::slashcommand_t& event);
	}

	/**
	 * @brief handles a channel setup request.
	 * @param event The slashcommand event object which contains information about the request.
	*/
	dpp::coroutine <> setup(const dpp::slashcommand_t& event);

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