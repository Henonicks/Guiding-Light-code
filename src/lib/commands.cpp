#include "guiding_light/commands.hpp"

#include "guiding_light/cli.hpp"
#include "guiding_light/slash_funcs.hpp"

const std::set <std::string> subcommand_list =
	{"--return", "--dev", "--cli"};

std::set <std::string> logs::list = {"guild_logs", "my_logs", "other_logs", "sql_logs"};

std::map <std::string, dpp::slashcommand> slashcommands::list_global;
std::map <std::string, dpp::slashcommand> slashcommands::list_guild;

/**
 * @brief Checks if a bash command exists.
 */
bool command_exists(std::string_view command) {
	return !system(fmt::format("which {} >>/dev/null 2>>/dev/null", command).c_str());
}

/**
 * @brief Initialises the slashcommands so they can be created in the future.
 */
void slashcommands::init() {
	dpp::slashcommand help("help", "See what I can do!", bot->me.id);
	dpp::slashcommand setup("setup", "Set up a part of JTC feature.", bot->me.id);
	dpp::slashcommand set("set", "Edit an attribute of the temp VC you are in (or of a JTC).", bot->me.id);
	dpp::slashcommand guild("guild", "Get/set the guild you're going to vote in favor of.", bot->me.id);
	dpp::slashcommand get("get", "Get the voting progress of a guild.", bot->me.id);
	dpp::slashcommand vote("vote", "Show the top.gg vote link.", bot->me.id);
	dpp::slashcommand logs("logs", "Drop the logs of choice.", bot->me.id);
	dpp::slashcommand blocklist("blocklist", "Add/Remove a user from your channel's blocklist", bot->me.id);
	dpp::slashcommand ticket("ticket", "Create/Delete a ticket.", bot->me.id);
	dpp::slashcommand select("select", "SELECT everything from one of the tables in the database.", bot->me.id);
	
    set.add_option(
        dpp::command_option(dpp::co_sub_command, "name", "Change the VC name.").
            add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VC to have.", true).set_max_length(100))
    );
    set.add_option(
        dpp::command_option(dpp::co_sub_command, "limit", "Change the member count limit.").
            add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VC to have.", true).set_min_value(0).set_max_value(99))
    );
    set.add_option(
        dpp::command_option(dpp::co_sub_command, "bitrate", "Change the bitrate of the VC.").
            add_option(dpp::command_option(dpp::co_integer, "bitrate", "The bitrate you want the VC to have.", true).set_max_value(384))
    );

    //---------------------------------------------------
    dpp::command_option sub_cmd_group_default = {dpp::co_sub_command_group, "default", "Change a default attribute of temp VCs."};

    dpp::command_option name_sub_cmd = {dpp::co_sub_command, "name", "Change default name of temp VCs."};
    name_sub_cmd.add_option(dpp::command_option(dpp::co_string, "name", "The name you want the VCs to have.", true).set_max_length(100));
    name_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
    sub_cmd_group_default.add_option(name_sub_cmd);

    dpp::command_option limit_sub_cmd = {dpp::co_sub_command, "limit", "Change default limit of temp VCs."};
    limit_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "limit", "The limit you want the VCs to have.", true).set_min_value(0).set_max_value(99));
    limit_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
    sub_cmd_group_default.add_option(limit_sub_cmd);
    
    dpp::command_option bitrate_sub_cmd = {dpp::co_sub_command, "bitrate", "Change default name of temp VCs."};
    bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_integer, "bitrate","The bitrate you want the VCs to have.", true).set_max_value(384));
    bitrate_sub_cmd.add_option(dpp::command_option(dpp::co_channel, "channel", "The default value of this JTC will be changed.", true));
    sub_cmd_group_default.add_option(bitrate_sub_cmd);
    //---------------------------------------------------

    set.add_option(sub_cmd_group_default);

    setup.add_option(
        dpp::command_option(dpp::co_sub_command, "jtc", "Setup a JTC voice channel.").
            add_option(dpp::command_option(dpp::co_integer, "maxmembers", "The max number of members in temporary VCs created from this one.", true).set_min_value(0).set_max_value(99))
    );
    
    dpp::command_option notifications_subcommands = {dpp::co_sub_command_group, "notifications", "Setup a notification channel."};
    notifications_subcommands.add_option(dpp::command_option(dpp::co_sub_command, "jtc", "Set up a notification channel for JTCs."));
    notifications_subcommands.add_option(dpp::command_option(dpp::co_sub_command, "topgg", "Setup a notification channel for top.gg votes."));
    
    setup.add_option(notifications_subcommands);

    setup.set_default_permissions(dpp::permissions::p_manage_channels);

    guild.add_option(dpp::command_option(dpp::co_sub_command, "get", "Get the guild you're going to vote in favor of. Votes go nowhere by default."));
    guild.add_option(dpp::command_option(dpp::co_sub_command, "set", "Set the guild you're going to vote in favor of."));

    get.add_option(dpp::command_option(dpp::co_sub_command, "progress", "Get the voting progress of a guild."));

    blocklist.add_option(
        dpp::command_option(dpp::co_sub_command, "add", "Add a user to the blocklist.").
            add_option(dpp::command_option(dpp::co_user, "user", "The user to be added to the blocklist."))
    );

    blocklist.add_option(
        dpp::command_option(dpp::co_sub_command, "remove", "Remove a user from the blocklist.").
            add_option(dpp::command_option(dpp::co_user, "user", "The user to be removed from the blocklist."))
    );

    blocklist.add_option(
        dpp::command_option(dpp::co_sub_command, "status", "Remove a user from the blocklist.").
            add_option(dpp::command_option(dpp::co_user, "user", "The user to be removed from the blocklist."))
    );

    logs.add_option(dpp::command_option(dpp::co_sub_command, "dpp", "D++ logs, sent by bot->on_log()."));
    logs.add_option(dpp::command_option(dpp::co_sub_command, "mine", "Logs, written by me."));
    logs.add_option(dpp::command_option(dpp::co_sub_command, "guild", "Guild logs."));
    logs.set_default_permissions(dpp::permissions::p_administrator);

    select.add_option(dpp::command_option(dpp::co_sub_command, "jtc-vcs", "The table containing JTC VCs."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "temp_vc_notifications", "The table containing temporary VC notification channels."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "jtc-default-values", "The table containing the default values of the JTCs."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "no-temp-ping", "The table containing the IDs of the users who don't want to be auto-pinged in temporary VCs."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "topgg-guild-choices", "The table containing the IDs of the guilds that the users have chosen for topgg."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "topgg-guild-votes-amount", "The table containing the amount of votes in favour of the guilds."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "no-noguild-reminder", "The table containing the user IDs who have been notified that they're not voting for any guilds."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "topgg-notifications", "The table containing the channels IDs of where it's notified when a user votes for that guild."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "tickets", "The table containing the IDs of tickets."));
    select.add_option(dpp::command_option(dpp::co_sub_command, "temp-vcs", "The table containing the IDs of temporary VCs."));
    logs.set_default_permissions(dpp::permissions::p_administrator);

    ticket.add_option(dpp::command_option(dpp::co_sub_command, "create", "Create a support ticket."));
    ticket.add_option(dpp::command_option(dpp::co_sub_command, "close", "Delete a support ticket."));
    ticket.set_dm_permission(true);

	list_global = { {"help", help}, {"setup", setup}, {"set", set}, {"guild", guild}, {"get", get}, {"vote", vote}, {"blocklist", blocklist}, {"ticket", ticket} };
	list_guild = { {"logs", logs}, {"select", select} };
}

std::string slash::get_mention(std::string_view command) {
	std::string name = command.data();
	if (name.starts_with('/')) {
		name = name.substr(1);
		// If the commands starts with a slash, remove it.
		// We're going to insert a slash on our own.
	}
	const std::string slashcommand_name = cli::tokenise(name)[0];
	const bool	is_global{global_created.contains(slashcommand_name)},
				exists{is_global || guild_created.contains(slashcommand_name)};
	const std::string mention = fmt::format("</{0}:{1}>", name,
		is_global ? global_created[slashcommand_name].id.str() :
		exists ? guild_created[slashcommand_name].id.str() :
		"ERROR COMMAND DOESN'T EXIST");
	return mention;
}
