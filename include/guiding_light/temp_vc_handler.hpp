#ifndef TEMP_VC_HANDLER_H
#define TEMP_VC_HANDLER_H

#include "temp_vc.hpp"
#include "notification_channel.hpp"
#include "jtc_defaults.hpp"
#include "logging.hpp"
#include "exception.hpp"

inline user_snowflake handling_user_id; // the ID of the user that a VC is being created for

void temp_vc_create_msg(const temp_vc_query& q, const dpp::channel& channel);
void temp_vc_delete_msg(const dpp::user& user, const dpp::channel* channel);
void temp_vc_create(const temp_vc_query& q);
bool blocklist_updated(const dpp::channel& channel);

#endif