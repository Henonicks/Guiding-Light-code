#include <slash_funcs.h>

std::map <std::string, dpp::slashcommand> slash::created_slashcommands;
dpp::embed slash::help_embed;
bool slash::enabled = false;

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
                event.reply(dpp::message("The maximum length of the channel name is `100`.").set_flags(dpp::m_ephemeral));
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

dpp::coroutine <void> slash::set::default_values(dpp::cluster& bot, const dpp::slashcommand_t& event) {
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
                event.reply(dpp::message(fmt::format("The limit is already + `{}` <a:skullspin:1121414918357389433>", limit)).set_flags(dpp::m_ephemeral));
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
        int8_t limit = ::topgg::jtc::count_jtcs(guildid);
		if (file::count_lines(event.command.guild_id.str(), file::jtc_vcs) >= limit) {
			event.reply(dpp::message(fmt::format("This guild has a JTC limit of {0}.{1}", limit, (limit < 10 ? " You can get more by voting though!" : ""))).set_flags(dpp::m_ephemeral));
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
        bool is_already_set;
        dpp::channel channel;
        channel.set_type(guild.is_community() ? dpp::CHANNEL_ANNOUNCEMENT : dpp::CHANNEL_TEXT);
        if (!guild.is_community()) {
            channel.set_permission_overwrite(guildid, dpp::ot_role, dpp::p_view_channel, dpp::p_send_messages);
        }
        channel.set_guild_id(guildid);
        const bool is_jtc = cmd.options[0].options[0].name == "jtc";
        if (is_jtc) {
            is_already_set = !ntif_chnls[guildid].empty();
            channel.set_name("temp-vc-notifications");
        }
        else {
            is_already_set = !topgg_ntif_chnls[guildid].empty();
            channel.set_name("topgg-notifications");
        }
        if (!is_already_set) {
            bot.channel_create(channel, [is_jtc](const dpp::confirmation_callback_t& callback) -> void {
                const auto newchannel = callback.get <dpp::channel>();
                const std::string to_add = std::to_string(newchannel.id) + ' ' + std::to_string(newchannel.guild_id);
                file::line_append(to_add, is_jtc ? file::temp_vc_notifications : file::topgg_notifications);
                (is_jtc ? ntif_chnls : topgg_ntif_chnls)[newchannel.guild_id] = newchannel.id;
            });
            event.reply(dpp::message("The channel was just set up!").set_flags(dpp::m_ephemeral));
        }
        else {
            event.reply(dpp::message("It's already set up. No more than one per guild!").set_flags(dpp::m_ephemeral));
        }
    }
}

void slash::blocklist::status(const dpp::slashcommand_t& event) {
    const dpp::snowflake channel_id = temp_vcs[vc_statuses[event.command.usr.id]].channelid;
    if (channel_id.empty()) {
        event.reply(dpp::message("You're not in a temporary voice channel!").set_flags(dpp::m_ephemeral));
        return;
    }
    const dpp::snowflake user_id = std::get <dpp::snowflake>(event.get_parameter("user"));
    event.reply(dpp::message(fmt::format("The user is {}in the blocklist of the channel.", (banned[channel_id].count(user_id) ? "" : "not "))));
}

void slash::blocklist::add(const dpp::slashcommand_t& event) {
    const dpp::user& issuer = event.command.usr;
    dpp::cluster* bot = event.from->creator;
    temp_vc issuer_vc = temp_vcs[vc_statuses[issuer.id]];
    if (issuer_vc.creatorid != issuer.id) {
        event.reply(dpp::message("The channel you\'re in does not belong to you!").set_flags(dpp::m_ephemeral));
        return;
    }
    const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
    const dpp::user* requested = dpp::find_user(requested_id);
    if (requested == nullptr) {
        event.reply(dpp::message("Requested user not found.").set_flags(dpp::m_ephemeral));
        return;
    }
    const dpp::channel* channel = dpp::find_channel(issuer_vc.channelid);
    const dpp::permission requested_permission = channel->get_user_permissions(requested);
    if (requested_permission.has(dpp::p_administrator)) {
        event.reply(dpp::message("The user has administrator access to this channel!").set_flags(dpp::m_ephemeral));
        return;
    }
    if (banned[issuer_vc.channelid].count(requested_id)) {
        event.reply(dpp::message("The user is already in the blocklist!").set_flags(dpp::m_ephemeral));
    }
    else {
        banned[issuer_vc.channelid].insert(requested_id);
        dpp::channel* channel = dpp::find_channel(vc_statuses[issuer.id]);
        channel->set_permission_overwrite(requested_id, dpp::ot_member, 0, dpp::p_view_channel);
        if (vc_statuses[issuer.id] == vc_statuses[requested_id]) {
            bot->guild_member_move(0, issuer_vc.guildid, requested_id);
            bot->channel_edit(*channel);
        }
        event.reply(dpp::message("The user was added to the blocklist of the current channel.").set_flags(dpp::m_ephemeral));
    }
}

void slash::blocklist::remove(const dpp::slashcommand_t& event) {
    dpp::cluster* bot = event.from->creator;
    const dpp::user& issuer = event.command.usr;
    if (temp_vcs[vc_statuses[issuer.id]].creatorid != issuer.id) {
        event.reply(dpp::message("The channel you\'re in does not belong to you!").set_flags(dpp::m_ephemeral));
        return;
    }
    const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
    const dpp::user* requested = dpp::find_user(requested_id);
    if (requested == nullptr) {
        event.reply(dpp::message("Requested user not found.").set_flags(dpp::m_ephemeral));
        return;
    }
    if (!banned[temp_vcs[vc_statuses[issuer.id]].channelid].count(requested_id)) {
        event.reply(dpp::message("The user was not in the blocklist!").set_flags(dpp::m_ephemeral));
    }
    else {
        banned[temp_vcs[vc_statuses[issuer.id]].channelid].erase(requested_id);
        dpp::channel* channel = dpp::find_channel(vc_statuses[issuer.id]);
        channel->set_permission_overwrite(requested_id, dpp::ot_member, dpp::p_view_channel, 0);
        bot->channel_edit(*channel);
        event.reply(dpp::message("The user was removed from the blocklist of the current channel.").set_flags(dpp::m_ephemeral));
    }
}

void slash::topgg::guild_get(const dpp::slashcommand_t& event) {
    dpp::user user = event.command.usr;
    dpp::snowflake guild_id = ::topgg::guild_choices[user.id];
    dpp::guild* guild = dpp::find_guild(guild_id);
    if (guild == nullptr) {
        event.reply(dpp::message(event.command.channel_id, "Guild not found. If you've already set it, consider trying again. Otherwise, set it with `/guild set`.").set_flags(dpp::m_ephemeral));
    }
    else {
        dpp::embed embed = dpp::embed()
            .set_color(dpp::colors::sti_blue)
            .set_title("You're voting in favour of this guild.")
            .set_author(fmt::format("Hello, {0}, your chosen guild is {1}.", user.username, guild->name), "", guild->get_icon_url())
            .set_thumbnail(guild->get_icon_url()
        );
        if (guild->get_banner_url() != "") {
            embed.set_image(guild->get_banner_url());
        }
        dpp::message message = dpp::message(event.command.channel_id, embed).set_flags(dpp::m_ephemeral);
        event.reply(message);
    }
}

void slash::topgg::guild_set(const dpp::slashcommand_t& event) {
    if (::topgg::guild_choices[event.command.usr.id] == event.command.guild_id) {
        try {
            event.reply(dpp::message(event.command.channel_id, fmt::format("Your chosen guild is already {}!", event.command.get_guild().name)));
        }
        catch (...) {
            event.reply(dpp::message(event.command.channel_id, "Guild not found. Try again?").set_flags(dpp::m_ephemeral));
        }
    }
    else {
        ::topgg::guild_choices[event.command.usr.id] = event.command.guild_id;
        file::delete_line_once(file::getline(event.command.usr.id.str(), file::topgg_guild_choices), file::topgg_guild_choices);
        file::line_append(event.command.usr.id.str() + ' ' + event.command.guild_id.str(), file::topgg_guild_choices);
        dpp::embed embed = dpp::embed()
            .set_color(dpp::colors::sti_blue)
            .set_title("You're now voting in favour of this guild.")
            .set_author(fmt::format("Set your guild to {}.", event.command.get_guild().name), "", event.command.get_guild().get_icon_url())
            .set_thumbnail(event.command.get_guild().get_icon_url()
        );
        if (event.command.get_guild().get_banner_url() != "") {
            embed.set_image(event.command.get_guild().get_banner_url());
        }
        dpp::message message = dpp::message(event.command.channel_id, embed).set_flags(dpp::m_ephemeral);
        event.reply(message);
    }
}

void slash::topgg::get_progress(const dpp::slashcommand_t& event) {
    const int8_t limit = ::topgg::jtc::count_jtcs(event.command.guild_id);
	event.reply(dpp::message(event.command.channel_id, fmt::format("This guild's vote progress is: __{0}__/**{1}**. The maximum amount of JTCs allowed here is {2}.{3}", ::topgg::guild_list[event.command.guild_id], ::topgg::votes_leveling[limit], limit, limit < 10 ? " This is the absolute maximum." : "")).set_flags(dpp::m_ephemeral));
}
