#include <slash_funcs.h>

void slash::set::current(dpp::cluster &bot, const dpp::slashcommand_t &event) {
    dpp::command_interaction cmd = event.command.get_command_interaction();
    const dpp::user user = event.command.get_issuing_user();
    dpp::snowflake userid = user.id;
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
        if (cmd.options[0].name == "name") {
			auto argument = std::get <std::string>(cmd.options[0].options[0].value);
            if (argument.size() > 100) {
                event.reply(dpp::message("The maximum length of channel name is `100`.").set_flags(dpp::m_ephemeral));
            }
            else if (argument == channel->name) {
                event.reply(dpp::message("The name of VC is already \"`" + old_name + "`\"").set_flags(dpp::m_ephemeral));
            }
            else {
                channel->set_name(argument);
                bot.channel_edit(*channel, [event, old_name, channel](const dpp::confirmation_callback_t& callback) {
                    event.reply(dpp::message("The name of the channel has changed from \"`" + old_name + "` to \"`" + channel->name + "`\".").set_flags(dpp::m_ephemeral));
                });
            }
        }
        else if (cmd.options[0].name == "bitrate") {
			auto argument = std::get <long>(cmd.options[0].options[0].value);
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
                content = fmt::format("Set bitrate to `{}` successfully.", argument);
                curr_channel.set_bitrate(argument);
                bot.channel_edit(curr_channel);
            }
            to_reply.set_content(content);
            event.reply(to_reply);

        }
        else if (cmd.options[0].name == "limit") {
			auto argument = std::get <long>(cmd.options[0].options[0].value);
            if (argument > 99) {
                event.reply(dpp::message("For voice channels, it's impossible to make the limit greater than `99`. You can make it infinite by putting `0` as argument.").set_flags(dpp::m_ephemeral));
            }
            else if (argument < 0) {
                event.reply(dpp::message("Yo what? You wanna make the limit negative? Well you shouldn't be able to stay in the channel then. Executing user out of the channel... Executing is done! Enjoy being kicked out!"));
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
    auto channelid = std::get <dpp::snowflake>(cmd.options[0].options[0].options[1].value);
	jtc_defaults defs = jtc_default_values[channelid];
	if (defs.channelid.empty()) {
		event.reply(dpp::message("This is not a JTC VC I know of.").set_flags(dpp::m_ephemeral));
		co_return;
	}
	jtc_defaults new_defs;
    if (cmd.options[0].options[0].name == "name") {
        auto name = std::get <std::string>(cmd.options[0].options[0].options[0].value);
        if (name.size() > 50) {
            event.reply(dpp::message(fmt::format("The default name shouldn't contain more symbols than 50. For a copy-paste, your failing choice was `{}`", name)).set_flags(dpp::m_ephemeral));
            co_return;
        }
        for (char& x : name) {
            if (x == ' ') {
                x = '_';
            }
        }
        if (defs.name == name) {
            event.reply(dpp::message(fmt::format("The name is already + `{}` <a:skullspin:1121414918357389433>", name)).set_flags(dpp::m_ephemeral));
            co_return;
        }
        new_defs.channelid = defs.channelid;
        new_defs.name = name;
        new_defs.limit = defs.limit;
        new_defs.bitrate = defs.bitrate;
        jtc_default_values.erase(defs.channelid);
        jtc_default_values[new_defs.channelid] = new_defs;
		std::string line = file::getline(channelid.str(), file::jtc_default_values, 0);
        file::delete_line_once(line, file::jtc_default_values);
        file::line_append(new_defs.channelid.str() + ' ' + name + ' ' + std::to_string(new_defs.limit) + ' ' + std::to_string(new_defs.bitrate), file::jtc_default_values);
        event.reply(dpp::message(fmt::format("The default name is set to `{}`!", name)).set_flags(dpp::m_ephemeral));
    }
    else if (cmd.options[0].options[0].name == "limit") {
        auto limit = std::get <long>(cmd.options[0].options[0].options[0].value);
        if (limit < 0 || limit > 99) {
            event.reply(dpp::message("The limit should be between `0` and `99` included, `0` for infinite").set_flags(dpp::m_ephemeral));
        }
        else {
            if (limit == 0) {
                limit = 100;
            }
            if (defs.limit == limit) {
                event.reply(dpp::message(fmt::format("Limit is already + `{}` <a:skullspin:1121414918357389433>", limit)).set_flags(dpp::m_ephemeral));
                co_return;
            }
            if (limit == 100) {
                limit = 0;
            }
            jtc_default_values.erase(channelid);
            file::delete_line_once(file::getline(std::to_string(defs.channelid), file::jtc_default_values, -1), file::jtc_default_values);
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
            event.reply(dpp::message(fmt::format("The default limit is set to `{}`!", limit)).set_flags(dpp::m_ephemeral));
        }
    }
    else if (cmd.options[0].options[0].name == "bitrate") {
        auto bitrate = std::get <long>(cmd.options[0].options[0].options[0].value);
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
            content = fmt::format("The default bitrate is set to `{}`!", bitrate);
            file::delete_line_once(line, file::jtc_default_values);
            new_defs = defs;
            new_defs.bitrate = (int)bitrate;
            jtc_default_values[channelid] = new_defs;
            file::line_append(std::to_string(new_defs.channelid) + ' ' + new_defs.name + ' ' + std::to_string(new_defs.limit) + ' ' + std::to_string(bitrate), file::jtc_default_values);
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
    const auto& member_roles = member.get_roles();
    if (!allowed_to_setup) {
        for (const auto& x : role_map) {
            if (x.second.has_manage_channels()) {
                if (std::find(member_roles.begin(), member_roles.end(), x.first) != member_roles.end()) {
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
		if (file::count_lines(event.command.guild_id.str(), file::jtc_vcs) >= 1) {
			event.reply(dpp::message("You can't have multiple JTCs *at the moment*. Consider editing one instead!").set_flags(dpp::m_ephemeral));
			co_return;
		}
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
            channel.set_user_limit(1);
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
