#include "temp_vc_handler.h"

user_snowflake handling_user_id;

void temp_vc_create_msg(dpp::cluster* bot, const temp_vc_query& q, const dpp::channel& channel) {
    std::string description = "A new temporary channel has been created ";
            description += (!channel.parent_id.empty() ?
            "in the <#" +
            channel.parent_id.str() + "> category"
            : "outside the categories");
            description += ". Join the channel, **" + channel.name + "** (<#" +
            channel.id.str() + ">)!";
            dpp::embed temp_vc_create_embed = dpp::embed().
            set_color(dpp::colors::greenish_blue).
            set_description(description);
    bot->message_create(dpp::message(temp_vc_notifications[q.guild_id], temp_vc_create_embed), error_callback);
}

 void temp_vc_create_owner_msg(dpp::cluster* bot, const temp_vc_query& q, const dpp::snowflake& channel_id) {
    dpp::embed temp_ping_embed = dpp::embed()
    .set_color(dpp::colors::sti_blue)
    .set_title(fmt::format("Welcome to <#{}>!", channel_id))
    .set_author(fmt::format("This VC belongs to {}.", q.usr->username), q.usr->get_url(), q.usr->get_avatar_url())
    .add_field(
        "You're able to edit the channel!",
        "Use a subcommand of the `/set` command to change the name, limit, or bitrate of your channel to whatever value your soul desires. See `/help` (not to be confused with \"seek help\") for more information."
    )
    .set_footer(
        dpp::embed_footer()
        .set_text("Use the button bellow to toggle the temporary VC creation ping on/off. Have fun!")
    );
    dpp::message message = dpp::message(channel_id, q.usr->get_mention()).add_embed(temp_ping_embed).add_component(
        dpp::component().add_component(
            dpp::component()
            .set_type(dpp::cot_button)
            .set_emoji("ping", 1271923808739000431)
            .set_style(dpp::cos_danger)
            .set_id("temp_ping_toggle")
        )
    ).set_allowed_mentions(true);
    bot->message_create(message, error_callback);
}

void temp_vc_delete_msg(dpp::cluster* bot, const dpp::user& user, const dpp::channel* channel) {
    log(fmt::format("{0} left a temp VC. Guild ID: {1}, channel ID: {2}, channel name: `{3}`, notification channel ID: {4}",
        user.format_username(), channel->guild_id, channel->id,
        channel->name, temp_vc_notifications[channel->guild_id]));
    const dpp::snowflake& channel_id = temp_vc_notifications[channel->guild_id];
    if (!channel_id.empty()) {
        std::string description = "A temporary channel **" + channel->name + "**";
        const dpp::channel* category = dpp::find_channel(channel->parent_id);
        if (category != nullptr) {
            description += " in the **" + category->get_mention() + "** category";
        }
        description += " has been deleted deleted.";
        const dpp::embed temp_vc_delete_message = dpp::embed().
        set_color(dpp::colors::blood_night).
        set_description(description);
        bot->message_create(dpp::message(channel_id, temp_vc_delete_message), error_callback);
    }
    bot->channel_delete(channel->id, error_callback);
}

void temp_vc_create(dpp::cluster* bot, const temp_vc_query& q) {
    if (q.usr == nullptr) {
        bot->log(dpp::ll_error, fmt::format("User {0} not found. Channel: {1}", q.usr->id, q.channel_id));
        if (!q.channel_id.empty()) {
            bot->message_create(dpp::message(q.channel_id, fmt::format("<@{}> I could not find your user information. Leave and try again in a few seconds.", q.usr->id)).set_allowed_mentions(true), error_callback);
        }
        return;
    }
    bool to_return = false;
    const uint64_t current_time = bot->uptime().to_secs();
    if (current_time - join_time[q.usr->id] <= 5) {
        bot->message_create(dpp::message(q.channel_id, fmt::format("<@{}> you have to wait more before joining the voice channel. To create a temporary VC as soon as possible, disconnect and try again in 5 seconds.", q.usr->id)).set_allowed_mentions(true), error_callback);
        to_return = true;
    }
    join_time[q.usr->id] = current_time;
    if (to_return) {
        bot->guild_member_move(0, q.guild_id, q.usr->id, error_callback);
        return;
    }
    std::string username = q.usr->username;
    std::string new_name;
    dpp::channel new_channel;
    new_channel.set_type(dpp::channel_type::CHANNEL_VOICE);
    jtc_defaults defs = jtc_default_values[q.channel_id];
    for (int i = 0; i < defs.name.size(); i++) {
        if (defs.name[i] == '{') {
            if (defs.name.size() - i >= 10) { // text {username}
                std::string temp_string;	  // 0123456789	14
                for (int j = i; j < i + 10; j++) {
                    temp_string += defs.name[j];
                }
                if (temp_string == "{username}") {
                    new_name += username;
                    i += 9;
                    continue;
                }
            }
        }
        else if (defs.name[i] == '_') {
            new_name += ' ';
            continue;
        }
        new_name += defs.name[i];
    }
    int limit = (int)defs.limit;
    new_channel.set_name(new_name);
    new_channel.set_guild_id(q.guild_id);
    new_channel.set_bitrate(defs.bitrate);
    dpp::channel current = jtc_channels_map[q.channel_id];
    new_channel.set_parent_id(current.parent_id);
    if (limit == 100) {
        limit = 0;
    }
    new_channel.set_user_limit(limit);
    const dpp::timer_callback_t timer_function = [new_channel, bot, current, q](const bool& called_separately) -> void {
        if (temp_vcs_queue.empty()) {
            if (called_separately) {
                return;
            }
            throw 0;
        }
        if (temp_vcs_queue.front().usr->id != q.usr->id) {
            return;
        }
        if (handling_user_id == q.usr->id) {
            return;
        }
        if (temp_vc_amount[q.guild_id] >= 50) {
            temp_vcs_queue.pop();
            bot->message_create(dpp::message(q.channel_id, fmt::format("<@{0}> There are too many temporary VCs in this guild ({1}/50).", q.usr->id, temp_vc_amount[q.guild_id])).set_allowed_mentions(true), error_callback);
            bot->guild_member_move(0, q.guild_id, q.usr->id, error_callback);
            return;
        }
        handling_user_id = q.usr->id;
        bot->channel_create(new_channel, [bot, current, q](const dpp::confirmation_callback_t& callback) -> void {
            temp_vcs_queue.pop();
            handling_user_id = 0;
            ++temp_vc_amount[q.guild_id];
            const auto channel = std::get <dpp::channel>(callback.value);
            if (!no_temp_ping[q.usr->id]) {
                temp_vc_create_owner_msg(bot, q, channel.id);
            }
            temp_vcs[channel.id] = {channel.id, channel.guild_id, q.usr->id};
            bot->guild_member_move(channel.id, channel.guild_id, q.usr->id, [bot, channel, q](const dpp::confirmation_callback_t& callback) -> void {
                if (callback.is_error()) {
                    bot->channel_delete(channel.id, error_callback);
                    return;
                }
                db::sql << "INSERT INTO temp_vcs VALUES (?, ?, ?, ?);" << channel.id.str() << channel.guild_id.str() << q.usr->id.str() << q.channel_id.str();
                if (!temp_vc_notifications[q.guild_id].empty()) {
                    temp_vc_create_msg(bot, q, channel);
                }
            });
            error_callback(callback);
        });
        if (called_separately) {
            return;
        }
        throw 0;
    };
    timer_function(true);
    bot->start_timer([new_channel, bot, current, q, timer_function](const dpp::timer& h) -> void {
        try {
            timer_function(false);
        }
        catch (...) {
            bot->stop_timer(h);
        }
    }, 1);
}
