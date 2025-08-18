#include "commands.h"

std::set <std::string> subcommand_list =
	{"--return", "--dev", "--cli"};
std::map <std::string, std::string> cli_manual = {
	{"help", "Shows this message.\n"
		"Usage: \"help [cmd1] [cmd2]... [cmdN]\" or \"help ...\""},
	{"quit", "Quit the CLI. If the Discord bot is running, it won't stop simply because of this command.\n"
		"Usage: quit"},
	{"switch", "Switch between release and dev (guidingLight and curiousLight).\n"
		"Usage: switch <mode>"},
	{"init_db", "Initialise the database for the current mode (release or dev). Needs to be done before the bot is launched.\n"
		"Usage: init_db"},
	{"conv_db", "Converts the database from text file-based to SQL-based. Needs to be done before the bot is launched if you have a text file-based database (was used before SQLite was introduced). Also changes the values to match the current standard.\n"
		"Usage: conv_db"},
	{"select", "Make a SELECT query to generate a file in database/select/<mode> where <mode> can either be release or dev.\n"
		"Usage: select <table>"},
	{"globalcreate", "Create a global slashcommand. Multiple arguments can be provided for multiple slashcommands to be created. Commands are created in bulk, so only the commands provided will exist. The rest, if they exist, will be deleted.\n"
		"Usage: \"ccreate <slashcommand> [slashcommand2]... [slashcommandN]\" or \"cdelete ...\""},
	{"guildcreate", "Create a guild slashcommand. Multiple arguments can be provided for multiple slashcommands to be created. Commands are created in bulk, so only the commands provided will exist. The rest, if they exist, will be deleted from the guild.\n"
		"Usage: \"ccreate <slashcommand> [slashcommand2]... [slashcommandN]\" or \"cdelete ...\""},
	{"cdelete", "Delete a slashcommand. Leave no parameters for every slashcommand to be deleted.\n"
		"Usage: cdelete [slashcommand]"},
	{"launch", "Launch the bot with the same mode as the one being used. Stays launched until the CLI has been exited. Switching the mode will NOT shut down the bot.\n"
		"Usage: launch"},
	{"list", "Lists what you ask it to - the log files, database tables or slashcommands that can be created. Acceptable values: logs, slashcommands and tables.\n"
		"Usage: list <type>"},
};

std::set <std::string> logs::list = {"guild_logs", "my_logs", "other_logs", "sql_logs"};

std::map <std::string, dpp::slashcommand> slashcommands::list_global;
std::map <std::string, dpp::slashcommand> slashcommands::list_guild;
std::vector <dpp::slashcommand> slashcommands::list_global_vector;
std::vector <dpp::slashcommand> slashcommands::list_guild_vector;

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
	list_global_vector = { help, setup, set, guild, get, vote, blocklist, ticket };
	list_guild_vector = { logs, select };
}
