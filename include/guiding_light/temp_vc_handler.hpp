#ifndef TEMP_VC_HANDLER_H
#define TEMP_VC_HANDLER_H

#include "guiding_light/temp_vc.hpp"

// 1000 is the ID of the first temp VC created during this runtime. 0 is used to see if a request/VC exists.
// Numbers lower than a certain amount are widely used as the priority of work in D++, hopefully 1000 is far away enough.
inline constexpr temp_id_t LOWEST_QUERY_ID = 1'000;
inline std::atomic current_temp_vc_request_ID = LOWEST_QUERY_ID;

/**
 * @brief Creates a temp VC query and pushes it into the temp VCs queue, automatically increments the ID to be assigned to another request, all in a thread-safe manner.
 * @param user_id The user to create a temporary channel for.
 * @param channel_id The parent (JTC) channel where the event was caused.
 * @param guild_id The guild where the event was caused.
 * @return The ID of this new request.
 */
temp_id_t create_temp_vc_request(dpp::snowflake user_id, dpp::snowflake channel_id, dpp::snowflake guild_id);

/**
 * @brief Returns the current to-be-assigned ID and automatically increments it to be assigned to another request, all in a thread-safe manner.
 * @return The current ID.
 */
temp_id_t create_temp_vc_id();

/**
 * @brief Get the frontmost query from the temp VC queue and notify the condition variable, all in a thread-safe manner.
 * @return An empty query if no requests exist or the frontmost query.
 */
temp_vc_query get_oldest_temp_vc_request();

/**
 * @brief Pop the temp VC queue in a thread-safe manner.
 */
void delete_temp_vc_request();

// TODO: document
void temp_vc_create_msg(const dpp::snowflake& guild_id, const dpp::channel& channel);
void temp_vc_delete_with_msg(dpp::snowflake channel_id);
void temp_vc_create(const dpp::voice_state_update_t& event);
bool temp_vc_is_accessible(const dpp::permission& overwrite);
bool temp_vc_is_accessible(const dpp::permission_overwrite& overwrite);
bool temp_vc_is_speakable(const dpp::permission& overwrite);
bool temp_vc_is_speakable(const dpp::permission_overwrite& overwrite);
bool blocklist_updated(const dpp::channel& channel);
bool mutelist_updated(const dpp::channel& channel);

#endif