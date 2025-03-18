#ifndef TICKET_HANDLER_H
#define TICKET_HANDLER_H

#include "guidingLight/guiding_light.h"
#include "dpp/unicode_emoji.h"
#include "ticket.h"
#include "slash_funcs.h"

dpp::message preserve_attachments(const dpp::message& msg); // put attachment links inside the message to preserve them when sharing
void handle_dm_in(const dpp::snowflake& user_id, const dpp::message_create_t& event); // handle a DM
void handle_dm_out(const dpp::message_create_t& event); // handle a DM
void ticket_create(const dpp::snowflake& user_id); // create a ticket for the user
void ticket_delete(const dpp::snowflake& user_id); // delete the user's ticket

#endif