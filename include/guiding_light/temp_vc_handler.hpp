#ifndef TEMP_VC_HANDLER_H
#define TEMP_VC_HANDLER_H

#include "guiding_light/temp_vc.hpp"
#include "guiding_light/notification_channel.hpp"

inline user_snowflake handling_user_id; // the ID of the user that a VC is being created for

void temp_vc_create_msg(const temp_vc_query& q, const dpp::channel& channel);
void temp_vc_delete_msg(const dpp::user& user, const dpp::channel* channel);
void temp_vc_create(const temp_vc_query& q);
bool temp_vc_is_accessible(const dpp::permission& overwrite);
bool temp_vc_is_accessible(const dpp::permission_overwrite& overwrite);
bool temp_vc_is_speakable(const dpp::permission_overwrite& overwrite);
bool temp_vc_is_speakable(const dpp::permission_overwrite& overwrite);
bool blocklist_updated(const dpp::channel& channel);
bool mutelist_updated(const dpp::channel& channel);

#endif