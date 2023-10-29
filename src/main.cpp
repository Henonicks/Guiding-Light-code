#include "guidingLight/guiding_light.h"
#include <random>
#include <thread>
#include "create_invite.h"
#include "channels_get.h"
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "temp_vc.h"
#include "string_convert.h"
#include "slash_funcs.h"
#include "file_namespace.h"
#include "notification_channel.h"
#include <csignal>
#include "configuration.h"
#include "logging.h"

std::unordered_map <dpp::snowflake, dpp::guild> all_bot_guilds;

std::pair <dpp::snowflake, dpp::snowflake> get_vc_status(std::string line) {
    std::pair <dpp::snowflake, dpp::snowflake> res;
    std::string user, vc;
    int i;
    for (i = 0; i < line.size(); i++) {
        if (line[i] == ' ') {
            i++;
            break;
        }
        user += line[i];
    }
    res.first = user;
    for (;i < line.size(); i++) {
        if (line[i] == ' ') {
            break;
        }
        vc += line[i];
    }
    res.second = vc;
    return res;
}

bool to_print_messages = false;

int main() {
    std::cout << "starting the bot" << std::endl;

    configuration::configure_bot();
    dpp::cluster bot(BOT_TOKEN, dpp::i_all_intents);
    configuration::configure_channels(bot);

    /* Register slash command here in on_ready */
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once <struct register_bot_commands>()) {
            bot.set_presence(dpp::presence(dpp::ps_idle, dpp::activity(dpp::activity_type::at_watching, "VCs in " + std::to_string(bot.current_user_get_guilds_sync().size()) + " guilds", "", "")));
//            bot.set_presence(dpp::presence::presence(dpp::presence_status::ps_idle, dpp::activity(dpp::activity_type::at_game, "ROBLOX Doors", "", "https://www.roblox.com/")));

            dpp::slashcommand type("type", "type", bot.me.id);
            dpp::slashcommand help("help", "See what I can do!", bot.me.id);
            dpp::slashcommand setup("setup", "Set up a part of JTC feature", bot.me.id);
            dpp::slashcommand set("set", "Edit an attribute of the temp VC you are in (or of a JTC).", bot.me.id);
            dpp::slashcommand get_emoji("get", "Get a list of emoji in a guild (helpful for /type)", bot.me.id);

            dpp::command_option emoji_option(dpp::co_sub_command, "emoji", "Get a list of emoji in a guild (helpful for /type)");

            emoji_option.add_option({dpp::co_string, "emoji-name", "Emoji name to match with. Leave empty to get a complete list of emoji."});
            emoji_option.add_option({dpp::co_string, "guild-id", "Id of the guild you want to search in. Leave empty for current."});


            get_emoji.add_option(emoji_option);



            set.add_option(
                    dpp::command_option(dpp::co_sub_command, "name", "Change the VC name").
                            add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VC to have", true))
            );
            set.add_option(
                    dpp::command_option(dpp::co_sub_command, "limit", "Change the member count limit").
                            add_option(dpp::command_option(dpp::co_string, "limit", "The limit you want the VC to have", true))
            );
            set.add_option(
                    dpp::command_option(dpp::co_sub_command, "bitrate", "Change the bitrate of the VC").
                        add_option(dpp::command_option(dpp::co_string, "bitrate", "The bitrate you want the VC to have", true))
            );

            //---------------------------------------------------
            dpp::command_option sub_cmd_group_default = dpp::command_option(dpp::co_sub_command_group, "default", "Change a default attribute of temp VCs");

            dpp::command_option name_sub_cmd = dpp::command_option(dpp::co_sub_command, "name", "Change default name of temp VCs");
            name_sub_cmd.add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VCs to have", true));
            name_sub_cmd.add_option(dpp::command_option(dpp::co_string, "channel", "The default value of this JTC will be changed", true));
            sub_cmd_group_default.add_option(name_sub_cmd);

            dpp::command_option limit_sub_cmd = dpp::command_option(dpp::co_sub_command, "limit", "Change default limit of temp VCs");
            limit_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VCs to have", true));
            limit_sub_cmd.add_option(dpp::command_option(dpp::co_string, "channel", "The default value of this JTC will be changed", true));
            sub_cmd_group_default.add_option(limit_sub_cmd);

            dpp::command_option bitrate_sub_cmd = dpp::command_option(dpp::co_sub_command, "bitrate", "Change default name of temp VCs");
            bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "bitrate","The bitrate you want the VCs to have", true));
            bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_string, "channel", "The default value of this JTC will be changed", true));
            sub_cmd_group_default.add_option(bitrate_sub_cmd);
            //---------------------------------------------------

            set.add_option(sub_cmd_group_default);

            setup.add_option(
                    dpp::command_option(dpp::co_sub_command, "jtc", "Setup a JTC voice channel").
                            add_option(dpp::command_option(dpp::co_string, "maxmembers", "The max number of members in temporary VCs created from this one", true))
            );
            setup.add_option(
                    dpp::command_option(dpp::co_sub_command_group, "notification", "Setup a notification channel").
                            add_option(dpp::command_option(dpp::co_sub_command, "channel", "Setup a notification channel"))
            );

            type.add_option(dpp::command_option(dpp::co_string, "text", "forgor", true));
            type.add_option(dpp::command_option(dpp::co_string, "replyto", "msg ID", true));
            type.add_option(dpp::command_option(dpp::co_channel, "channelid", "zabuv"));

            bot.global_bulk_command_create( {type, help, setup, set, get_emoji} );
        }
    });

    bot.on_message_create([&bot](const dpp::message_create_t& event) {
        std::string msg = event.msg.content;
        dpp::snowflake channelid = event.msg.channel_id;
        std::string prefix;
        dpp::snowflake userid = event.msg.author.id;
        const std::string username = event.msg.author.username;
        const std::string usertag = '#' + std::to_string(event.msg.author.discriminator);
        const std::string usernt = username + usertag;
        const std::string str_userid = "<@" + std::to_string(userid) + '>';
        if (event.msg.author.id == my_id) {
            if (msg == "<@" + std::to_string(bot.me.id) + "> ping") {
            }
            if (msg == "!unban") {
                dpp::snowflake guildid, user_id;
                std::cin >> guildid >> user_id;
                bot.guild_ban_delete(guildid, user_id);
            }
            if (msg == "!role-give") {
                dpp::snowflake guildid, user_id, roleid;
                std::cin >> guildid >> user_id >> roleid;
                bot.guild_member_add_role(guildid, user_id, roleid);
            }
            if (msg == "!channels-get") {
                channels_get(bot);
            }
            if (msg == "!create-invite") {
                create_invite(bot);
            }
            if (msg.substr(0, 15) == "!delete-command") {
                std::string cmd_id;
                for (int i = 17; i < msg.size(); i++) {
                    cmd_id += msg[i];
                }
                bot.global_command_delete(cmd_id);
            }
            if (to_print_messages) {
                std::cout << msg << std::endl;
            }
            if (msg == "!print-enable" || msg == "!print-disable") {
                dpp::message msg_to_send;
                if (msg == "!print-enable") {
                    to_print_messages = true;
                    msg_to_send = dpp::message("Auto-printing messages is enabled!");
                }
                else if (msg == "!print-disable") {
                    to_print_messages = false;
                    msg_to_send = dpp::message("Auto-printing messages is disabled!");
                }
                msg_to_send.set_flags(dpp::m_ephemeral);
                event.reply(msg_to_send);
            }
            if (msg == "!start-debugging") {
                std::string line;
                while (std::cin >> line && line != "stop") {
                    if (line == "jtc") {
                        for (const auto& x : jtc_vcs) {
                            std::cout << x.first << ' ' << x.second.guildid << std::endl;
                        }
                    }
                    if (line == "temp") {
                        for (const auto& x : temp_vcs) {
                            std::cout << x.first << ' ' << x.second.guildid << ' ' << x.second.creatorid << std::endl;
                        }
                    }

                }
            }
        }
        if (event.msg.is_dm()) {
            if (userid != dpp::snowflake(bot_id)) {
                bot.message_create(dpp::message(bot_dm_logs, "<@" + std::to_string(userid) + "> (" + event.msg.author.format_username() + ") [to me] " + event.msg.content), [&bot, userid](const dpp::confirmation_callback_t& callback) {
                    bot.direct_message_create(userid, dpp::message("Message is received!"), [&bot, userid](const dpp::confirmation_callback_t& callback) {
                        if (callback.is_error()) {
                            bot.log(dpp::loglevel::ll_error, callback.http_info.body);
                            return;
                        }
                        auto msg = std::get <dpp::message>(callback.value);
                        bot.start_timer([&bot, msg](dpp::timer timer) {
                            bot.message_delete(msg.id, msg.channel_id);
                            bot.stop_timer(timer);
                        }, 2);
                    });
                });
            }
        }
        if (msg.find("!dm ") == 0 && event.msg.author.id == my_id) {
            std::string dm_who_str;
            int i;
            for (i = 4; i < msg.size(); i++) {
                if (msg[i] == ' ') {
                    break;
                }
                dm_who_str += msg[i];
            }
            auto dm_who = dpp::snowflake(dm_who_str);
            dpp::user_identified user = bot.user_get_sync(dm_who);
            std::string content;
            for (; i < msg.size(); i++) {
                content += msg[i];
            }
            if (content.empty()) {
                std::getline(std::cin >> std::ws, content);
            }
            bot.direct_message_create(dm_who, dpp::message(content));
            bot.message_create(dpp::message(bot_dm_logs, fmt::format("[me to] <@{0}> (``{1}``) \'{2}\'", dm_who_str, user.format_username(), content)));
            bot.log(dpp::loglevel::ll_info, fmt::format("dming {0} \'{1}\'", dm_who, content));
        }
        if (msg.size() > 4) {
            for (int i = 1; i <= 4; i++) {
                prefix += msg[i];
            }
        }
        if (msg == "!id") {
            dpp::snowflake guildid = event.msg.guild_id;
            dpp::snowflake channel_id = event.msg.channel_id;
            std::string reply = "Guild ID:\n```" + std::to_string(guildid) + "```\nChannel ID:\n```" + std::to_string(channel_id) + "```";
            event.reply(reply);
            bot.message_create(dpp::message(bot_logs_copy, "Server ID:\n```" + std::to_string(guildid) + "```\nChannel ID:\n```" + std::to_string(channel_id) + "```"));
            std::cout << "POV" << '\n';
        }
        if (msg == "!tag me") {
            bot.message_create(dpp::message(channelid, str_userid));
        }
        else if (msg == "!leave guild") {
            auto guild_roles = bot.roles_get_sync(event.msg.guild_id);
            dpp::guild_member member = event.msg.member;
            const auto& member_roles = member.get_roles();
            bool can_remove = false;
            for (const auto& x : guild_roles) {
                if (x.second.has_kick_members() || x.second.has_ban_members()) {
                    auto begin = member_roles.begin(), end = member_roles.end();
                    if (std::find(begin, end, x.first) != std::end(member_roles)) {
                        can_remove = true;
                        break;
                    }
                }
            }
            if (event.msg.author.id == my_id || can_remove) {
                event.reply("Leaving!");
                bot.current_user_leave_guild(event.msg.guild_id);
            }
            else {
                event.reply("You can't make me leave. No perms.");
            }
        }
        else if (msg == "!DM me") {
            bot.direct_message_create(event.msg.author.id, dpp::message("Hey! As you asked, I DM'd you!"));
            bot.message_delete(event.msg.id, event.msg.channel_id);
        }
    });

    bot.on_channel_update([](const dpp::channel_update_t& event) -> void {
        jtc_channels_map[event.updated->id] = *event.updated;
    });

    bot.on_channel_delete([](const dpp::channel_delete_t& event) -> void {
        dpp::channel_type type = event.deleted.get_type();
        dpp::snowflake channelid = event.deleted.id;
        dpp::snowflake guildid = event.deleted.guild_id;
        if (type == dpp::channel_type::CHANNEL_VOICE) {
            jtc_vc to_erase_jtc;
            to_erase_jtc.channelid = 0;
            temp_vc to_erase_temp;
            to_erase_temp.channelid = 0;
            to_erase_jtc = jtc_vcs[channelid];
            to_erase_temp = temp_vcs[channelid];
            if (!to_erase_jtc.channelid.empty()) {
                jtc_defaults jtc_defs_to_erase;
                jtc_defs_to_erase = jtc_default_values[channelid];
                jtc_default_values.erase(channelid);
                jtc_channels_map.erase(channelid);
                jtc_vcs.erase(channelid);
                const std::string jtc_line = std::to_string(to_erase_jtc.channelid) + ' ' + std::to_string(to_erase_jtc.guildid);
                file::delete_line_once(jtc_line, file::jtc_vcs);
                const std::string jtc_defs_line = file::getline(std::to_string(jtc_defs_to_erase.channelid), file::jtc_default_values, -1);
                file::delete_line_once(jtc_defs_line, file::jtc_default_values);
            }
            if (!to_erase_temp.channelid.empty()) {
                temp_vcs.erase(channelid);
            }
        }
        else {
            bool is_ntf = !ntif_chnls[guildid].channelid.empty();
            if (is_ntf) {
                std::string to_remove = file::getline(std::to_string(event.deleted.guild_id), file::temp_vc_notifications, 0);
                file::delete_line_once(to_remove, file::temp_vc_notifications);
                ntif_chnls.erase(event.deleted.guild_id);
            }
        }
    });

    bot.on_guild_create([&bot](const dpp::guild_create_t& event) -> void {
        all_bot_guilds[event.created->id] = *event.created;
        //bot.set_presence(dpp::presence(dpp::ps_idle, dpp::activity(dpp::activity_type::at_watching, "VCs in " + std::to_string(all_bot_guilds.size()) + " guilds", "", "")));
        bot.message_create(dpp::message(bot_logs_copy, "I have joined a guild. These are its stats:\nName: `" + event.created->name + "`\nID: `" + std::to_string(event.created->id) + "`"));
    });
    bot.on_guild_delete([&bot](const dpp::guild_delete_t& event) {
        all_bot_guilds.erase(event.deleted.id);
        bot.message_create(dpp::message(bot_logs_copy, "I have left a guild. These are its stats:\nName: `" + event.deleted.name + "`\nID: `" + std::to_string(event.deleted.id) + "`"));
    });

    bot.on_voice_state_update([&bot](const dpp::voice_state_update_t& event) -> void {
        dpp::snowflake userid = event.state.user_id;
        dpp::user* ptr = dpp::find_user(userid);
        if (ptr == nullptr) {
            log("User not found");
            return;
        }
        dpp::user user = *ptr;
        dpp::snowflake guildid = event.state.guild_id;
        if (!event.state.channel_id.empty()) {
            dpp::snowflake channelid = event.state.channel_id;
            bool is_jtc = !jtc_vcs[channelid].channelid.empty();
            if (is_jtc) {
                bool to_return = false;
                uint64_t current_time = bot.uptime().to_secs();
                if (current_time - join_time[userid] <= 5) {
                    bot.direct_message_create(userid, dpp::message("You have to wait more before joining the voice channel. To create a temporary VC as soon as possible, disconnect and try again in 5 seconds."));
                    to_return = true;
                }
                join_time[userid] = current_time;
                if (to_return) {
                    return;
                }
                std::string username = user.username;
                std::string new_name;
                dpp::channel new_channel;
                new_channel.set_type(dpp::channel_type::CHANNEL_VOICE);
                jtc_defaults defs = jtc_default_values[channelid];
                for (int i = 0; i < defs.name.size(); i++) {
                    if (defs.name[i] == '{') {
                        if (defs.name.size() - i >= 10) { // text {username}
                            std::string temp_string;      // 0123456789    14
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
                int limit = defs.limit;
                log("Temp channel is being created in " + dpp::find_guild(guildid)->name);
                new_channel.set_name(new_name);
                new_channel.set_guild_id(guildid);
                new_channel.set_bitrate(defs.bitrate);
                dpp::channel current = jtc_channels_map[channelid];
                new_channel.set_parent_id(current.parent_id);
                if (limit == 100) {
                    limit = 0;
                }
                new_channel.set_user_limit(limit);
                bot.channel_create(new_channel,[&bot, current, userid, user, guildid, event](auto const callback) {
                   auto newchannel = std::get <dpp::channel>(callback.value);
                   dpp::snowflake channelid = newchannel.id;
                   temp_vcs[newchannel.id] = {newchannel.id, newchannel.guild_id, userid};
                   bot.guild_member_move(newchannel.id, newchannel.guild_id, userid, [&bot, channelid, userid](const dpp::confirmation_callback_t& callback) -> void {
                       bot.start_timer([&bot, callback, userid, channelid](dpp::timer timer) {
                           if (callback.is_error() || vc_statuses[userid] != channelid) {
                               bot.channel_delete(channelid, [&bot](const dpp::confirmation_callback_t& callback) -> void {
                                   if (callback.is_error()) {
                                       bot.log(dpp::ll_error, callback.get_error().message);
                                   }
                               });
                           }
                           bot.stop_timer(timer);
                       }, 5);
                   });
                   bot.message_create(dpp::message(bot_logs_copy,
                                                   "<@" + std::to_string(userid) +
                                                   "> created <#" + std::to_string(channelid) +
                                                   "> " + std::to_string(channelid)));
                   if (!ntif_chnls[guildid].guildid.empty()) {
                       std::string description = "A new temporary channel has been created ";
                       description += (!newchannel.parent_id.empty() ?
                               "in the <#" +
                               std::to_string(newchannel.parent_id) + "> category"
                               : "outside the categories");
                       description += ". Join the channel, **" + newchannel.name + "** (<#" +
                                      std::to_string(newchannel.id) + ">)!";
                       dpp::embed temp_vc_create_embed = dpp::embed().
                               set_color(dpp::colors::greenish_blue).
                               set_description(description);
                       bot.message_create(dpp::message(ntif_chnls[guildid].channelid, temp_vc_create_embed));

                   }
                   if (callback.is_error()) {
                       std::cout << callback.http_info.body << '\n';
                   }
               });
            }
        }
        dpp::snowflake channelid = vc_statuses[userid];
        bool is_temp = !temp_vcs[channelid].channelid.empty();
        if (is_temp && dpp::find_channel(channelid)->get_voice_members().empty()) {
            dpp::channel* newchannel = dpp::find_channel(channelid);
            log(fmt::format("{0} joined a JTC. Guild ID: {1}, channel ID: {2}, channel name: `{3}`, notification channel ID: {4}",
                            user.format_username(), guildid, channelid,
                            dpp::find_channel(channelid)->name, ntif_chnls[guildid].channelid));
            bot.channel_delete(channelid, [](const dpp::confirmation_callback_t& callback) {
                if (callback.is_error()) {
                    std::cout << callback.http_info.body << std::endl;
                }
            });
            dpp::snowflake ntif_channelid = ntif_chnls[guildid].channelid;
            if (!ntif_channelid.empty()) {
                std::string description = "A temporary channel **" + newchannel->name + "**";
                if (!newchannel->parent_id.empty()) {
                    dpp::channel category = *dpp::find_channel(newchannel->parent_id);
                    description += " in the **" + category.name + "** category";
                }
                description += " is deleted.";
                dpp::embed temp_vc_delete_message = dpp::embed().
                        set_color(dpp::colors::blood_night).
                        set_description(description);
                bot.message_create(dpp::message(ntif_channelid, temp_vc_delete_message));
            }
        }
        vc_statuses[userid] = event.state.channel_id;
        if (vc_statuses[userid].empty()) {
            vc_statuses[userid] = 1;
        }
    });

    dpp::embed help_embed = dpp::embed().
            set_color(dpp::colors::sti_blue).
            set_title("``HELP``").
            set_author("Here is what I can do!\n", "", "").
            set_description("``/help`` - I don't know, I guess you've just issued the command\n"
                            "``/setup`` - create a join-to-create (JTC) voice channel. As soon as you join one of those, a temporary one is being created and you get moved to it, unless you've disconnected\n"
                            "``/edit is a command with 3 subcommands. It changes some of properties of a temporary voice channel. If it wasn't created because of you joining to a JTC voice channel, the command won't work``");
    bot.on_slashcommand([&bot, &help_embed](const dpp::slashcommand_t& event) -> dpp::task <void> {
        dpp::snowflake guildid = event.command.guild_id;
        dpp::guild guild;
        dpp::command_interaction cmd = event.command.get_command_interaction();
        if (!guildid.empty()) {
            guild = *dpp::find_guild(guildid);
        }
        if (event.command.get_command_name() == "help") {
            event.reply(dpp::message(event.command.channel_id, help_embed));
            co_return;
        }
        if (event.command.get_command_name() == "set") {
            if (cmd.options[0].name == "default") {
                co_await slash::set::default_values(bot, event);
            }
            else {
                slash::set::current(bot, event);
            }
        }
        if (event.command.get_command_name() == "setup") {
            co_await slash::setup(bot, event);
        }
        if (event.command.get_command_name() == "type") {
            slash::type(bot, event);
        }
        if (event.command.get_command_name() == "get") {
            co_await slash::get_emoji(bot, event);
        }
    });

    signal(SIGINT, [](int code) {
        std::cout << std::endl << "Ну, все, я пішов спати, бувай, добраніч." << std::endl;
        system("killall guidingLight");
    });

    bot.start(false);

    return 0;
}