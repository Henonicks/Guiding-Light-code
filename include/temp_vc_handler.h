#ifndef TEMP_VC_HANDLER_H
#define TEMP_VC_HANDLER_H

#include "guidingLight/guiding_light.h"
#include "temp_vc.h"
#include "notification_channel.h"
#include "jtc_vc.h"
#include "logging.h"

extern user_snowflake handling_user_id; // the ID of the user that a VC is being created for

void temp_vc_create_msg(dpp::cluster* bot, const temp_vc_query& q, const dpp::channel& channel);
void temp_vc_delete_msg(dpp::cluster* bot, const dpp::user& user, const dpp::channel* channel);
void temp_vc_create(dpp::cluster* bot, const temp_vc_query& q);

#endif