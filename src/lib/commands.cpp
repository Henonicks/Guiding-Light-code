#include "guiding_light/commands.hpp"

#include <dpp/unicode_emoji.h>

#include "guiding_light/cli.hpp"
#include "guiding_light/responses.hpp"
#include "guiding_light/slash_funcs.hpp"

bool command_exists(std::string_view command) {
	return !system(fmt::format("which {} >>/dev/null 2>>/dev/null", command).c_str());
}

dpp::command_option localise_command_option(const henifig::value_map& current_options, const dpp::command_option& original_option, const std::string_view lang) {
	dpp::command_option res = original_option;
	if (islower(current_options.rbegin()->first[0])) {
		if (current_options.contains(res.name)) {
			res = localise_command_option(current_options.at(res.name), res, lang);
		}
	}
	else {
		res.add_localization(lang.data(), current_options.at("NAME"), current_options.at("DESCRIPTION"));
		for (dpp::command_option& x : res.options) {
			if (current_options.contains("OPTIONS")) {
				x = localise_command_option(current_options.at("OPTIONS"), x, lang);
			}
			else {
				break;
			}
		}
	}
	return res;
}

dpp::command_option localise_command_options(const dpp::command_option& original_option, const dpp::slashcommand& cmd, const std::string_view option, henifig::value_map commands = cfg::slashcommands["COMMANDS"]) {
	dpp::command_option res = original_option;
	for (const auto& lang : commands | std::views::keys) {
		if (lang != "default") {
			if (commands.at(lang).get <henifig::value_map>().contains(cmd.name) && commands.at(lang)[cmd.name]["OPTIONS"].get <henifig::value_map>().contains(option.data())) {
				res = localise_command_option(commands.at(lang.data())[cmd.name]["OPTIONS"][option], res, lang);
			}
		}
	}
	return res;
}

dpp::slashcommand localise_slashcommand(const dpp::slashcommand& original_slashcommand, const henifig::value_map& commands = cfg::slashcommands["COMMANDS"]) {
	dpp::slashcommand res = original_slashcommand;
	for (const auto& lang : commands | std::views::keys) {
		if (lang != "default") {
			if (commands.at(lang).get <henifig::value_map>().contains(res.name)) {
				res.add_localization(lang, commands.at(lang)[res.name]["NAME"], commands.at(lang)[res.name]["DESCRIPTION"]);
			}
		}
	}
	return res;
}

dpp::slashcommand make_default(const std::string_view name, const henifig::value_map& commands = cfg::slashcommands["COMMANDS"]) {
	return dpp::slashcommand(commands.at("default")[name]["NAME"].get <std::string>(), commands.at("default")[name]["DESCRIPTION"].get <std::string>(), bot->me.id);
}

const henifig::value_map* get_opt_by_path(const std::string_view path, const henifig::value_map& command) {
	const henifig::value_map* res = &command;
	for (size_t i = 0; i < path.size(); i++) {
		const size_t pos = path.find('/', i);
		const size_t n = pos == std::string::npos ? path.size() : pos - i;
		res = &res->at("OPTIONS")[std::string(path.substr(i, n))].get <henifig::map_t>().get();
		if (pos == std::string::npos) {
			break;
		}
		i = pos;
	}
	return res;
}

dpp::command_option make_default(const dpp::command_option_type cot, const dpp::slashcommand& parent, const std::string_view path, const henifig::value_map& commands = cfg::slashcommands["COMMANDS"]) {
	const henifig::value_map& options = *get_opt_by_path(path, commands.at("default")[parent.name]);
	return {cot, options.at("NAME"), options.at("DESCRIPTION")};
}

dpp::command_option make_default(const dpp::command_option_type cot, const dpp::slashcommand& parent, const std::string_view path, const bool required, const henifig::value_map& commands = cfg::slashcommands["COMMANDS"]) {
	const henifig::value_map& options = *get_opt_by_path(path, commands.at("default")[parent.name]);
	return {cot, options.at("NAME"), options.at("DESCRIPTION"), required};
}

void slashcommands::init() {
	dpp::slashcommand help(localise_slashcommand(make_default("help")));
	dpp::slashcommand setup(localise_slashcommand(make_default("setup")));
	dpp::slashcommand set(localise_slashcommand(make_default("set")));
	dpp::slashcommand guild(localise_slashcommand(make_default("guild")));
	dpp::slashcommand get(localise_slashcommand(make_default("get")));
	dpp::slashcommand vote(localise_slashcommand(make_default("vote")));
	dpp::slashcommand logs(localise_slashcommand(make_default("logs")));
	dpp::slashcommand blocklist(localise_slashcommand(make_default("blocklist")));
	dpp::slashcommand ticket(localise_slashcommand(make_default("ticket")));
	dpp::slashcommand select(localise_slashcommand(make_default("select")));
	dpp::slashcommand reload(localise_slashcommand(make_default("reload")));

	setup.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, setup, "jtc").add_option(
				make_default(dpp::co_integer, setup, "jtc/maxmembers", true).set_min_value(0).set_max_value(99)
			)
		, setup, "jtc")
	);
	dpp::command_option notifications_subcommands(
		make_default(dpp::co_sub_command_group, setup, "notifications")
	);
	notifications_subcommands.add_option(
		make_default(dpp::co_sub_command, setup, "notifications/jtc")
	);
	notifications_subcommands.add_option(
		make_default(dpp::co_sub_command, setup, "notifications/topgg")
	);
	setup.add_option(
		localise_command_options(notifications_subcommands, setup, "notifications")
	);
	setup.set_default_permissions(dpp::permissions::p_manage_channels);

	set.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, set, "name").add_option(
			make_default(dpp::co_string, set, "name/name", true).set_max_length(100))
		, set, "name")
	);
	set.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, set, "limit").add_option(
			make_default(dpp::co_integer, set, "limit/limit", true).set_min_value(-99).set_max_value(99))
		, set, "limit")
	);
	set.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, set, "bitrate").add_option(
				make_default(dpp::co_integer, set, "bitrate/bitrate", true).set_max_value(384)
			)
		, set, "bitrate")
	);
	dpp::command_option sub_cmd_group_default(
		make_default(dpp::co_sub_command_group, set, "default")
	);
	dpp::command_option name_sub_cmd(
		make_default(dpp::co_sub_command, set, "default/name")
	);
	name_sub_cmd.add_option(
		make_default(dpp::co_string, set, "default/name/name", true).set_max_length(100)
	);
	name_sub_cmd.add_option(
		make_default(dpp::co_channel, set, "default/name/channel", true)
	);
	sub_cmd_group_default.add_option(name_sub_cmd);
	dpp::command_option limit_sub_cmd(
		make_default(dpp::co_sub_command, set, "default/limit")
	);
	limit_sub_cmd.add_option(
		make_default(dpp::co_integer, set, "default/limit/limit", true).set_min_value(0).set_max_value(99)
	);
	limit_sub_cmd.add_option(
		make_default(dpp::co_channel, set, "default/limit/channel", true)
	);
	sub_cmd_group_default.add_option(limit_sub_cmd);
	dpp::command_option bitrate_sub_cmd(
		make_default(dpp::co_sub_command, set, "default/bitrate")
	);
	bitrate_sub_cmd.add_option(
		make_default(dpp::co_integer, set, "default/bitrate/bitrate", true).set_max_value(384)
	);
	bitrate_sub_cmd.add_option(
		make_default(dpp::co_channel, set, "default/bitrate/channel", true)
	);
	sub_cmd_group_default.add_option(bitrate_sub_cmd);
	set.add_option(localise_command_options(sub_cmd_group_default, set, "default"));

	guild.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, guild, "get")
		, guild, "get")
	);
	guild.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, guild, "set")
		, guild, "set")
	);

	get.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, get, "progress")
		, get, "progress")
	);

	logs.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, logs, "dpp")
		, logs, "dpp")
	);
	logs.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, logs, "mine")
		, logs, "mine")
	);
	logs.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, logs, "guild")
		, logs, "guild")
	);
	logs.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, logs, "sqlite")
		, logs, "sqlite")
	);
	logs.set_default_permissions(dpp::permissions::p_administrator);

	blocklist.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, blocklist, "add").add_option(
				make_default(dpp::co_user, blocklist, "add/user")
			)
		, blocklist, "add")
	);

	blocklist.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, blocklist, "remove").add_option(
				make_default(dpp::co_user, blocklist, "remove/user")
			)
		, blocklist, "remove")
	);

	blocklist.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, blocklist, "status").add_option(
				make_default(dpp::co_user, blocklist, "status/user")
			)
		, blocklist, "status")
	);

	ticket.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, ticket, "create")
		, ticket, "create")
	);
	ticket.add_option(
		localise_command_options(
			make_default(dpp::co_sub_command, ticket, "close")
		, ticket, "close")
	);
	ticket.set_dm_permission(true);

	for (const std::string& x : db::table_names) {
		select.add_option(
			make_default(dpp::co_sub_command, select, x)
		);
	}

	logs.set_default_permissions(dpp::permissions::p_administrator);

	reload.set_default_permissions(dpp::permissions::p_administrator);

	list_global = { {"help", help}, {"setup", setup}, {"set", set}, {"guild", guild}, {"get", get}, {"vote", vote}, {"blocklist", blocklist}, {"ticket", ticket} };
	list_guild = { {"logs", logs}, {"select", select}, {"reload", reload} };
}

std::string slash::get_mention(const std::string_view command) {
	std::string name = command.data();
	if (name.starts_with('/')) {
		name = name.substr(1);
		// If the command starts with a slash, remove it.
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

std::vector <std::string> slash::get_mention(const std::vector <std::string>& command) {
	std::vector <std::string> res;
	for (const std::string_view x : command) {
		res.push_back(get_mention(x));
	}
	return res;
}
