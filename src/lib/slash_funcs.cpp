#include <slash_funcs.h>

void slash::set::current(dpp::cluster &bot, const dpp::slashcommand_t &event) {
    dpp::command_interaction cmd = event.command.get_command_interaction();
    const dpp::user user = event.command.get_issuing_user();
    dpp::snowflake userid = user.id;
    dpp::snowflake guildid = event.command.guild_id;
    /*dpp::guild guild;
    if (!event.command.channel.is_dm()) {
        guild = *dpp::find_guild(guildid);
    }*/

    dpp::snowflake channelid = vc_statuses[userid];
    bool user_is_main = !channelid.empty();
    if (!user_is_main) {
        event.reply(dpp::message("You are not in a VC you can edit. Note that those commands edit temporary VCs only").set_flags(dpp::m_ephemeral));
    }
    else {
        dpp::channel* channel = dpp::find_channel(channelid);
        if (channel == nullptr) {
            event.reply(dpp::message("Channel could not be found.").set_flags(dpp::m_ephemeral));
            return;
        }
        std::string old_name = channel->name;
        auto argument_str = std::get <std::string>(cmd.options[0].options[0].value);
        int argument = string_to_ll(argument_str);
        if (cmd.options[0].name == "name") {
            if (argument_str.size() > 100) {
                event.reply(dpp::message("The maximum length of channel name is `100`.").set_flags(dpp::m_ephemeral));
            }
            else if (argument_str == channel->name) {
                event.reply(dpp::message("The name of VC is already \"`" + old_name + "`\"").set_flags(dpp::m_ephemeral));
            }
            else {
                channel->set_name(argument_str);
                bot.channel_edit(*channel, [event, old_name, channel](const dpp::confirmation_callback_t& callback) {
                    event.reply(dpp::message("The name of the channel has changed from \"`" + old_name + "` to \"`" + channel->name + "`\".").set_flags(dpp::m_ephemeral));
                });
            }
        }
        else if (cmd.options[0].name == "bitrate") {
            dpp::guild* guild = dpp::find_guild(channel->guild_id);
            if (guild == nullptr) {
                event.reply("Guild could not be found.");
                return;
            }
            dpp::channel curr_channel = *channel;
            dpp::message to_reply = dpp::message().set_flags(dpp::m_ephemeral);
            std::string content;
            int max_bitrate = ((guild->premium_tier == 0) ?
                               96 : (guild->premium_tier == 1) ?
                                    128 : (guild->premium_tier == 2) ?
                                          256 : 384);
            if (channel->bitrate == argument) {
                content = "The bitrate is already `" + std::to_string(argument) + "`.";
            }
            else if (argument > max_bitrate || argument < 8) {
                content = "The number can only be between `8` and `" + std::to_string(max_bitrate) +
                          "` for this guild. Note that the maximum bitrate number also depends on the boost level.";
            }
            else {
                content = "Set bitrate to `" + argument_str + "` successfully.";
                curr_channel.set_bitrate(argument);
                bot.channel_edit(curr_channel);
            }
            to_reply.set_content(content);
            event.reply(to_reply);

        }
        else if (cmd.options[0].name == "limit") {
            if (argument > 99) {
                event.reply(dpp::message("It's impossible to make the limit greater than `99`. You can make it infinite by putting `0` as argument.").set_flags(dpp::m_ephemeral));
            }
            else if (argument < 0) {
                event.reply(dpp::message("Yo what? You wanna make limit negative? Well you shouldn't be able to stay in the channel then. Executing user out of the channel... Executing is done! Enjoy being kicked out!"));
                bot.guild_member_move(0, bot.channel_get_sync(vc_statuses[userid]).guild_id, userid);
            }
            else {
                if (channel->user_limit == argument) {
                    event.reply(dpp::message("The user limit is already `" + std::to_string(argument) + "`.").set_flags(dpp::m_ephemeral));
                }
                else {
                    channel->set_user_limit(argument);
                    bot.channel_edit(*channel);
                    event.reply(dpp::message("User limit is set to `" + std::to_string(argument) + "` successfuly.").set_flags(dpp::m_ephemeral));
                }
            }
        }
    }
}

dpp::coroutine <void> slash::set::default_values(dpp::cluster &bot, const dpp::slashcommand_t &event) {
    dpp::command_interaction cmd = event.command.get_command_interaction();
    const dpp::user user = event.command.get_issuing_user();
    dpp::snowflake userid = user.id;
    dpp::snowflake guildid = event.command.guild_id;
    dpp::guild guild;
    if (!event.command.channel.is_dm()) {
        guild = *dpp::find_guild(guildid);
    }
    if (!(userid == my_id || user.is_team_user())) {
        bool allowed_to_set = guild.owner_id == userid;
        const dpp::confirmation_callback_t& confirmation = co_await bot.co_roles_get(guildid);
        const auto& guild_roles = confirmation.get <dpp::role_map>();
        dpp::guild_member member = event.command.member;
        const auto& roles = member.get_roles();
        if (!allowed_to_set) {
            for (const auto& x : guild_roles) {
                if (x.second.has_manage_channels()) {
                    if (std::find(roles.begin(), roles.end(), x.first) != roles.end()) {
                        allowed_to_set = true;
                        break;
                    }
                }
            }
        }
        if (!allowed_to_set) {
            event.reply(dpp::message("You don't have a role that has the MANAGE_CHANNELS permission.").set_flags(dpp::m_ephemeral));
            co_return;
        }
        else {
            event.reply(dpp::message("This is an in-dev command.").set_flags(dpp::m_ephemeral));
            co_return;
        }
    }
    std::string channelid = std::get <std::string>(cmd.options[0].options[0].options[1].value);
    if (cmd.options[0].options[0].name == "name") {
        std::string name = std::get <std::string>(cmd.options[0].options[0].options[0].value);
        if (name.size() > 50) {
            event.reply(dpp::message("The default name shouldn't contain more symbols than 50.").set_flags(dpp::m_ephemeral));
            co_return;
        }
        for (char& x : name) {
            if (x == ' ') {
                x = '_';
            }
        }
        std::string line = file::getline(channelid, file::jtc_default_values, 0);
        jtc_defaults defs = get_jtc_defs(line), new_defs;
        if (defs.name == name) {
            event.reply(dpp::message("Name is already + `" + name + "` <a:skullspin:1121414918357389433>").set_flags(dpp::m_ephemeral));
            co_return;
        }
        new_defs.channelid = defs.channelid;
        new_defs.name = name;
        new_defs.limit = defs.limit;
        new_defs.bitrate = defs.bitrate;
        jtc_default_values.erase(defs.channelid);
        jtc_default_values[new_defs.channelid] = new_defs;
        file::delete_line_once(line, file::jtc_default_values);
        file::line_append(std::to_string(new_defs.channelid) + ' ' + name + ' ' + std::to_string(new_defs.limit) + ' ' + std::to_string(new_defs.bitrate), file::jtc_default_values);
        event.reply(dpp::message("Default name should be set to `" + name + "`, go check it out!").set_flags(dpp::m_ephemeral));
    }
    else if (cmd.options[0].options[0].name == "limit") {
        std::string limit_str = std::get <std::string>(cmd.options[0].options[0].options[0].value);
        auto limit = (short)string_to_ll(limit_str);
        if (limit < 0 || limit > 99) {
            event.reply(dpp::message("The limit should be between `0` and `99` included, `0` for infinite").set_flags(dpp::m_ephemeral));
        }
        else {
            jtc_defaults defs = jtc_default_values[channelid];
            if (limit == 0) {
                limit = 100;
            }
            if (defs.limit == limit) {
                event.reply(dpp::message("Limit is already + `" + std::to_string(limit) + "` <a:skullspin:1121414918357389433>").set_flags(dpp::m_ephemeral));
                co_return;
            }
            if (limit == 100) {
                limit = 0;
            }
            jtc_default_values.erase(channelid);
            file::delete_line_once(file::getline(std::to_string(defs.channelid), file::jtc_default_values, -1), file::jtc_default_values);
            jtc_defaults new_defs;
            new_defs.channelid = defs.channelid;
            new_defs.name = defs.name;
            new_defs.limit = limit;
            new_defs.bitrate = defs.bitrate;
            if (limit == 0) {
                new_defs.limit = 100;
            }
            jtc_default_values[channelid] = new_defs;
            std::string line = std::to_string(new_defs.channelid) + ' ' + new_defs.name + ' ' + std::to_string(new_defs.limit) + ' ' + std::to_string(new_defs.bitrate);
            file::line_append(line, file::jtc_default_values);
            event.reply(dpp::message("Default limit should be set to `" + std::to_string(limit) + "`, go check it out!").set_flags(dpp::m_ephemeral));
        }
    }
    else if (cmd.options[0].options[0].name == "bitrate") {
        std::string bitrate_str = std::get <std::string>(cmd.options[0].options[0].options[0].value);
        int bitrate = string_to_ll(bitrate_str);
        jtc_defaults defs = jtc_default_values[channelid];
        std::string line = file::getline(std::to_string(defs.channelid), file::jtc_default_values, -1);
        dpp::message to_reply = dpp::message().set_flags(dpp::m_ephemeral);
        std::string content;
        int max_bitrate = ((guild.premium_tier == 0) ?
            96 : (guild.premium_tier == 1) ?
            128 : (guild.premium_tier == 2) ?
            256 : 384);
        if (defs.bitrate == bitrate) {
            content = "The bitrate is already `" + std::to_string(bitrate) + "` <a:skullspin:1121414918357389433>";
        }
        else if (bitrate > max_bitrate || bitrate < 8) {
            content = "The number can only be between `8` and `" + std::to_string(max_bitrate) +
                "` for this guild. Note that the maximum bitrate number also depends on the boost level.";
        }
        else {
            content = "Set bitrate to `" + std::to_string(bitrate) + "` successfully.";
            file::delete_line_once(line, file::jtc_default_values);
            jtc_defaults new_defs = defs;
            new_defs.bitrate = bitrate;
            jtc_default_values[channelid] = new_defs;
            file::line_append(std::to_string(new_defs.channelid) + ' ' + new_defs.name + ' ' + std::to_string(new_defs.limit) + std::to_string(bitrate), file::jtc_default_values);
        }
        to_reply.set_content(content);
        event.reply(to_reply);
    }
}

dpp::coroutine <void> slash::setup(dpp::cluster& bot, const dpp::slashcommand_t& event) {
    dpp::command_interaction cmd = event.command.get_command_interaction();
    const dpp::user user = event.command.get_issuing_user();
    dpp::snowflake userid = user.id;
    dpp::snowflake guildid = event.command.guild_id;
    dpp::guild guild;
    if (!event.command.channel.is_dm()) {
        guild = *dpp::find_guild(guildid);
    }
    
    bool allowed_to_setup = guild.owner_id == userid;
    const dpp::confirmation_callback_t& confirmation = co_await bot.co_roles_get(guildid);
    const auto& role_map = confirmation.get <dpp::role_map>();
    dpp::guild_member member = event.command.member;
    const auto& roles = member.get_roles();
    if (!allowed_to_setup) {
        for (const auto& x : role_map) {
            if (x.second.has_manage_channels()) {
                if (std::find(roles.begin(), roles.end(), x.first) != roles.end()) {
                    allowed_to_setup = true;
                    break;
                }
            }
        }
    }
    if (!allowed_to_setup) {
        event.reply(dpp::message("You don't have a role that has the MANAGE_CHANNELS permission.").set_flags(dpp::m_ephemeral));
        co_return;
    }
    if (cmd.options[0].name == "jtc") {
        std::string arg_string = std::get <std::string>(cmd.options[0].options[0].value);
        auto max = (short)string_to_ll(arg_string);
        if (max < 0 || max > 100) {
            event.reply(dpp::message("Only numbers between 0 and 99 (0 for channel without limit) are allowed.").set_flags(dpp::m_ephemeral));
        }
        else {
            dpp::channel channel;
            channel.set_type(dpp::channel_type::CHANNEL_VOICE);
            if (max > 0) {
                channel.set_name("Join-to-create for " + std::to_string(max));
            }
            else {
                channel.set_name("Join-to-create for infinite");
            }
            channel.set_parent_id(0);
            channel.set_guild_id(event.command.guild_id);
            channel.set_bitrate(64);
            channel.set_user_limit(0);
            event.reply(dpp::message("Created a join-to-create channel.").set_flags(dpp::m_ephemeral));
            bot.channel_create(channel, [&bot, &max](auto const callback) {
                dpp::channel newchannel;
                newchannel = std::get <dpp::channel>(callback.value);
                if (max == 0) {
                    max = 100;
                }
                jtc_vc vc = {newchannel.id, newchannel.guild_id};
                jtc_vcs[newchannel.id] = vc;
                jtc_defaults defs = {newchannel.id, "VC_for_{username}", max, 96};
                jtc_default_values[newchannel.id] = defs;
                const std::string vc_string = std::to_string(vc.channelid) + ' ' + std::to_string(vc.guildid);
                const std::string defs_str = std::to_string(defs.channelid) + ' ' + defs.name + ' ' + std::to_string(defs.limit) + ' ' + std::to_string(defs.bitrate);
                file::line_append(vc_string, file::jtc_vcs);
                file::line_append(defs_str, file::jtc_default_values);
                if (callback.is_error()) {
                    bot.log(dpp::loglevel::ll_error, callback.http_info.body);
                }
                jtc_channels_map[newchannel.id] = newchannel;
            });
        }
    }
    else {
        bool is_already_set = !ntif_chnls[guildid].channelid.empty();
        if (!is_already_set) {
            dpp::channel channel;
            channel.set_type(dpp::CHANNEL_ANNOUNCEMENT);
            channel.set_name("temp-vc-notifications");
            channel.set_guild_id(guildid);
            channel.set_parent_id(0);
            bot.channel_create(channel, [](const dpp::confirmation_callback_t& callback) -> void {
                dpp::channel newchannel = std::get <dpp::channel>(callback.value);
                std::string to_add = std::to_string(newchannel.id) + ' ' + std::to_string(newchannel.guild_id);
                file::line_append(to_add, file::temp_vc_notifications);
                ntif_chnls[newchannel.guild_id] = get_ntf_chnl(to_add);
            });
            event.reply(dpp::message("It's been set. Enjoy!").set_flags(dpp::m_ephemeral));
        }
        else {
            event.reply(dpp::message("It's been already set. No more than one per guild!").set_flags(dpp::m_ephemeral));
        }
    }
}

void slash::type(dpp::cluster& bot, const dpp::slashcommand_t& event) {
    dpp::guild_member issuing_member = event.command.member;
    dpp::command_interaction cmd = event.command.get_command_interaction();
    const dpp::user user = event.command.get_issuing_user();
    dpp::snowflake userid = user.id;
    bot.guild_get_member(my_guild_id, userid, [&bot, event, user, cmd, issuing_member, userid](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error()) {
            event.reply("You have to be in our guild of gaymerz (plz iBeg yu :sob:)");
            return;
        }
        const std::string username = user.username;
        const std::string usertag = '#' + std::to_string(user.discriminator);
        std::string usernt = "`" + username + usertag + "`" + elgato;
        if (userid == my_id) {
            usernt = std::string();
        }
        std::string argument = std::get <std::string>(cmd.options[0].value);
        const std::string old = argument;
        argument = convert_characters(argument);
        auto replyto = (dpp::snowflake)std::get <std::string>(cmd.options[1].value);
        dpp::snowflake channelid;
        dpp::snowflake command_channel_id = event.command.channel_id;
        if (cmd.options.size() == 3) {
            channelid = std::get <dpp::snowflake>(cmd.options[2].value);
            dpp::channel* asked_channel = dpp::find_channel(channelid);
            if (asked_channel == nullptr) {
                event.reply(dpp::message("Channel not found.").set_flags(dpp::m_ephemeral));
                return;
            }
            if (!asked_channel->get_user_permissions(issuing_member).has(dpp::p_send_messages)) {
                event.reply(dpp::message("You cannot type here.").set_flags(dpp::m_ephemeral));
                return;
            }
            if (!replyto.empty()) {
                bot.message_get(replyto, channelid, [&bot, argument, replyto, channelid, event, usernt](const dpp::confirmation_callback_t& callback) {
                    if (callback.is_error()) {
                        bot.log(dpp::loglevel::ll_info, callback.http_info.body);
                        event.reply(dpp::message("I couldn't get the original message. If you are sure this is the right channel and so is ID of the message AND the message hasn't been deleted, try again.").set_flags(dpp::m_ephemeral));
                        return;
                    }
                    dpp::message original_msg = std::get <dpp::message>(callback.value);
                    if (original_msg.channel_id != channelid) {
                        event.reply(dpp::message("This message does not exist in that channel!").set_flags(dpp::m_ephemeral));
                        return;
                    }
                    event.reply(dpp::message("Sent!").set_flags(dpp::m_ephemeral));
                    bot.message_create(dpp::message(channelid, usernt + argument).set_reference(replyto));
                });
            }
            else {
                bot.message_create(dpp::message(channelid, usernt + argument).set_reference(replyto));
                event.reply(dpp::message("The message should be sent!").set_flags(dpp::m_ephemeral));
            }
        }
        else {
            if (replyto.empty()) {
                event.reply(dpp::message("Sent!").set_flags(dpp::m_ephemeral));
                bot.message_create(dpp::message(command_channel_id, usernt + argument));
            }
            else {
                bot.message_get((dpp::snowflake)replyto, command_channel_id, [&bot, event, &usernt, argument, replyto](const dpp::confirmation_callback_t& callback) {
                    if (callback.is_error()) {
                        bot.log(dpp::loglevel::ll_info, callback.http_info.body);
                        event.reply(dpp::message("An error occurred. Make sure you typed in the correct ID and the message is in the same channel you are in.").set_flags(dpp::m_ephemeral));
                        return;
                    }
                    event.reply(dpp::message("Sent!").set_flags(dpp::m_ephemeral));
                    bot.message_create(dpp::message(event.command.channel_id, usernt + argument).set_reference(replyto));
                });
            }
        }
        if (userid != my_id) {
            usernt = usernt.substr(1, usernt.size() - (elgato.size() + 1) - 1);
        }
        log(fmt::format("@<{0}> ({1}) \'{2}\' in {3}",
            std::to_string(userid), usernt, old, std::to_string(channelid)));
    });
}

dpp::coroutine <void> slash::get_emoji(dpp::cluster& bot, const dpp::slashcommand_t& event) {
    dpp::snowflake guild_id;
    if (std::holds_alternative <std::string>(event.get_parameter("guild-id"))) {
        guild_id = (dpp::snowflake)std::get <std::string>(event.get_parameter("guild-id"));
    }
    dpp::guild* guild = dpp::find_guild(guild_id);
    std::string description;
    if (guild == nullptr) {
        description = "Guild was not found. Matching in current.\n";
        guild = dpp::find_guild(event.command.guild_id);
    }
    std::string name;
    if (std::holds_alternative <std::string>(event.get_parameter("emoji-name"))) {
        name = std::get <std::string>(event.get_parameter("emoji-name"));
    }
    const dpp::confirmation_callback_t& callback = co_await bot.co_guild_emojis_get(guild->id);
    const auto& emojis = callback.get <dpp::emoji_map>();
    std::string res;
    for (const auto& x : emojis) {
        if (x.second.name.find(name) != std::string::npos || name.empty()) {
            res += fmt::format("{0}: `{0}`", x.second.get_mention()) + '\n';
        }
    }
    description += res.empty() ? "No emoji were found." : res;
    dpp::embed embed = dpp::embed()
        .set_title(guild->name)
        .set_url(guild->get_icon_url())
        .set_author((name.empty() ? "List of emoji in the requested guild:" : fmt::format("Matching emoji with name `{}`:", name)), "", "")
        .set_description(description)
        .set_color(dpp::colors::cyan);
    event.reply(dpp::message(event.command.channel_id, embed).set_flags(dpp::m_ephemeral));
}
