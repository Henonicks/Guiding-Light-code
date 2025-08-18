#ifndef TICKET_HANDLER_H
#define TICKET_HANDLER_H

#include "guidingLight/guiding_light.h"
#include "dpp/unicode_emoji.h"
#include "ticket.h"
#include "slash_funcs.h"

/**
 * @brief Puts attachment links inside the message to preserve them when sharing.
 * @param msg The message.
 * @returns The message with the links inserted.
 */
dpp::message preserve_attachments(const dpp::message& msg);

/**
 * @brief Handles a DM from a user.
 * @param event The event object which contains information about the request.
 */
void handle_dm_in(const dpp::message_create_t& event);

/**
 * @brief Sends a DM.
 * @param event The event object which contains information about the request.
 */
void handle_dm_out(const dpp::message_create_t& event);

/**
 * @brief Creates a ticket.
 * @param user_id The ID of the user creating the ticket.
 */
void ticket_create(const dpp::snowflake& user_id);

/**
 * @brief Deletes a ticket.
 * @param user_id The ID of the user deleting the ticket.
 */
void ticket_delete(const dpp::snowflake& user_id);

#endif