#include "guiding_light/slash_funcs.hpp"

std::map <std::string, std::map <dpp::snowflake, bool>> slash::in_progress;
std::map <std::string, dpp::slashcommand> slash::global_created, slash::guild_created;
std::vector <dpp::embed> slash::help_embeds;
bool slash::enabled = false;

dpp::coroutine <> slash::set::current(const dpp::slashcommand_t &event) {
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::user user = event.command.get_issuing_user();
	const dpp::snowflake& user_id = user.id;
	const dpp::snowflake& channel_id = vc_statuses[user_id];
	const bool user_is_main = !channel_id.empty();
	if (!user_is_main) {
		event.reply(dpp::message("You are not in a VC you can edit. If you are, it's likely that the bot restarted while you were in the channel. In that case, **__rejoin it__**. Note that those commands edit temporary VCs only.").set_flags(dpp::m_ephemeral));
	}
	else {
		dpp::channel channel = *dpp::find_channel(channel_id);
		const std::string& old_name = channel.name;
		if (cmd.options[0].name == "name") {
			const auto argument = std::get <std::string>(cmd.options[0].options[0].value);
			if (argument == channel.name) {
				event.reply(dpp::message("The name of the VC is already \"`" + old_name + "`\"").set_flags(dpp::m_ephemeral));
			}
			else {
				channel.set_name(argument);
				bot->channel_edit(channel, [event, old_name, argument](const dpp::confirmation_callback_t& callback) {
					if (callback.is_error()) {
						error_feedback(callback, event);
						return;
					}
					event.reply(dpp::message("The name of the channel has changed from \"`" + old_name + "` to \"`" + argument + "`\".").set_flags(dpp::m_ephemeral));
				});
			}
		}
		else if (cmd.options[0].name == "bitrate") {
			const auto argument = std::get <long>(cmd.options[0].options[0].value);
			const dpp::guild* guild = dpp::find_guild(channel.guild_id);
			if (guild == nullptr) {
				event.reply("Guild could not be found. Please, try again.");
				co_return;
			}
			dpp::message to_reply = dpp::message().set_flags(dpp::m_ephemeral);
			std::string content;
			const int max_bitrate = ((guild->premium_tier == 0) ?
							   96 : (guild->premium_tier == 1) ?
									128 : (guild->premium_tier == 2) ?
										  256 : 384);
			if (channel.bitrate == argument) {
				content = "The bitrate is already `" + std::to_string(argument) + "`.";
			}
			else if (argument > max_bitrate || argument < 8) {
				content = "The number can only be between `8` and `" + std::to_string(max_bitrate) +
						  "` for this guild. Note that the maximum bitrate number also depends on the boost level.";
			}
			else {
				content = fmt::format("Set bitrate to `{}` successfully.", argument);
				channel.set_bitrate(argument);
				const dpp::confirmation_callback_t& callback = co_await bot->co_channel_edit(channel);
				if (callback.is_error()) {
					error_feedback(callback, event);
					co_return;
				}
			}
			to_reply.set_content(content);
			event.reply(to_reply);
		}
		else if (cmd.options[0].name == "limit") {
			const auto argument = std::get <long>(cmd.options[0].options[0].value);
			if (argument > 99) {
				event.reply(dpp::message("For voice channels, it's impossible to make the limit greater than `99`. You can make it infinite by putting `0` as argument.").set_flags(dpp::m_ephemeral));
			}
			else if (argument < 0) {
				event.reply(dpp::message("Well now we're just being silly, aren't we?").set_flags(dpp::m_ephemeral));
				bot->guild_member_move(0, temp_vcs[vc_statuses[user_id]].guild_id, user_id, error_callback);
				// easter egg
			}
			else {
				if (channel.user_limit == argument) {
					event.reply(dpp::message("The user limit is already `" + std::to_string(argument) + "`.").set_flags(dpp::m_ephemeral));
				}
				else {
					channel.set_user_limit(argument);
					bot->channel_edit(channel, [event, argument](const dpp::confirmation_callback_t& callback) {
						if (callback.is_error()) {
							error_feedback(callback, event);
							return;
						}
						event.reply(dpp::message("User limit is set to `" + std::to_string(argument) + "` successfuly.").set_flags(dpp::m_ephemeral));
					});
				}
			}
		}
	}
}

dpp::coroutine <> slash::set::default_values(const dpp::slashcommand_t& event) {
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::snowflake& guild_id = event.command.guild_id;
	const dpp::guild guild = *dpp::find_guild(guild_id);
	const auto channel_id = std::get <dpp::snowflake>(cmd.options[0].options[0].options[1].value);
	const jtc_defaults defs = jtc_default_values[channel_id];
	if (defs.channel_id.empty()) {
		co_await event.co_reply(dpp::message("This is not a JTC VC I know of.").set_flags(dpp::m_ephemeral));
		co_return;
	}
	jtc_defaults new_defs;
	if (cmd.options[0].options[0].name == "name") {
		auto name = std::get <std::string>(cmd.options[0].options[0].options[0].value);
		if (defs.name == name) {
			co_await event.co_reply(dpp::message(fmt::format("The name is already + `{}` <a:skullspin:1121414918357389433>", name)).set_flags(dpp::m_ephemeral));
			co_return;
		}
		new_defs.channel_id = defs.channel_id;
		new_defs.name = name;
		new_defs.limit = defs.limit;
		new_defs.bitrate = defs.bitrate;
		jtc_default_values.erase(defs.channel_id);
		jtc_default_values[new_defs.channel_id] = new_defs;
		db::sql << "DELETE FROM jtc_default_values WHERE channel_id=?;" << channel_id.str();
		db::sql << "INSERT INTO jtc_default_values VALUES (?, ?, ?, ?);" << channel_id.str() << name << defs.limit << defs.bitrate;
		event.reply(dpp::message(fmt::format("The default name is set to `{}`!", name)).set_flags(dpp::m_ephemeral));
	}
	else if (cmd.options[0].options[0].name == "limit") {
		const auto limit = std::get <long>(cmd.options[0].options[0].options[0].value);
		if (limit < 0 || limit > 99) {
			event.reply(dpp::message("The limit should be between `0` and `99` included, `0` for infinite").set_flags(dpp::m_ephemeral));
		}
		else {
			if (defs.limit == limit) {
				co_await event.co_reply(dpp::message(fmt::format("The limit is already + `{}` <a:skullspin:1121414918357389433>", limit)).set_flags(dpp::m_ephemeral));
				co_return;
			}
			jtc_default_values.erase(channel_id);
			db::sql << "DELETE FROM jtc_default_values WHERE channel_id=?;" << defs.channel_id.str();
			new_defs.channel_id = defs.channel_id;
			new_defs.name = defs.name;
			new_defs.limit = (int8_t)limit;
			new_defs.bitrate = defs.bitrate;
			jtc_default_values[channel_id] = new_defs;
			db::sql << "INSERT INTO jtc_default_values VALUES (?, ?, ?, ?);" << channel_id.str() << defs.name << limit << defs.bitrate;
			event.reply(dpp::message(fmt::format("The default limit is set to `{}`!", limit)).set_flags(dpp::m_ephemeral));
		}
	}
	else if (cmd.options[0].options[0].name == "bitrate") {
		auto bitrate = std::get <long>(cmd.options[0].options[0].options[0].value);
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
			// Since the maximum bitrate varies between each guild (based on the amount of boosts),
			// this check is needed.
		}
		else {
			content = fmt::format("The default bitrate is set to `{}`!", bitrate);
			new_defs = defs;
			new_defs.bitrate = (int16_t)bitrate;
			jtc_default_values[channel_id] = new_defs;
			db::sql << "DELETE FROM jtc_default_values WHERE channel_id=?;" << channel_id.str();
			db::sql << "INSERT INTO jtc_default_values VALUES (?, ?, ?, ?);" << channel_id.str() << defs.name << defs.limit << bitrate;
		}
		to_reply.set_content(content);
		event.reply(to_reply);
	}
}

dpp::coroutine <> slash::setup(const dpp::slashcommand_t& event) {
	const dpp::snowflake& guild_id = event.command.guild_id;
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::guild& guild = *dpp::find_guild(guild_id);
	if (cmd.options[0].name == "jtc") {
		const int8_t limit = ::topgg::jtc::count_allowed_jtcs(guild_id);
		if (jtc_vc_amount[guild_id] >= limit) {
			co_await event.co_reply(dpp::message(fmt::format("This guild has a JTC limit of {0}.{1}", limit, (limit < 10 ? " You can get more by voting though!" : ""))).set_flags(dpp::m_ephemeral));
			co_return;
		}
		const auto max = (int8_t)std::get <long>(cmd.options[0].options[0].value);
		dpp::channel channel;
		channel.set_type(dpp::channel_type::CHANNEL_VOICE);
		channel.set_name(fmt::format("Join-to-create for {}", max > 0 ? std::to_string(max) : "infinite"));
		channel.set_parent_id(0);
		channel.set_guild_id(event.command.guild_id);
		channel.set_bitrate(64); // The default bitrate in Discord.
		channel.set_user_limit(1); // For the normal users, only one will be able to create a JTC at the same time.
		const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
		if (callback.is_error()) {
			bot->log(dpp::loglevel::ll_error, callback.http_info.body);
			co_await event.co_reply(dpp::message("Tried to create a join-to-create channel but failed. If this happens again, please report the incident by opening a ticket.").set_flags(dpp::m_ephemeral));
			co_return;
		}
		++jtc_vc_amount[guild_id];
		const auto new_channel = std::get <dpp::channel>(callback.value);
		jtc_vcs[new_channel.id] = new_channel.guild_id;
		const jtc_defaults defs = {new_channel.id, "VC for {username}", max, 64};
		jtc_default_values[new_channel.id] = defs;
		db::sql << "INSERT INTO jtc_vcs VALUES (?, ?);" << new_channel.id.str() << new_channel.guild_id.str();
		db::sql << "INSERT INTO jtc_default_values VALUES (?, ?, ?, ?);" << new_channel.id.str() << "VC for {username}" << max << 64;
		co_await event.co_reply(dpp::message("Created a join-to-create channel.").set_flags(dpp::m_ephemeral));
	}
	else {
		bool is_already_set;
		dpp::channel channel;
		channel.set_type(guild.is_community() ? dpp::CHANNEL_ANNOUNCEMENT : dpp::CHANNEL_TEXT);
		if (!guild.is_community()) {
			// In non-community guilds, announcement channels are not available - they can't be created.
			// Therefore, we need to deny everyone from typing in the notification channels.
			channel.set_permission_overwrite(guild_id, dpp::ot_role, dpp::p_view_channel, dpp::p_send_messages);
		}
		channel.set_guild_id(guild_id);
		const bool is_jtc = cmd.options[0].options[0].name == "jtc";
		if (is_jtc) {
			is_already_set = !temp_vc_notifications[guild_id].empty();
			channel.set_name("temp-vc-notifications");
		}
		else {
			is_already_set = !topgg_notifications[guild_id].empty();
			channel.set_name("topgg-notifications");
		}
		if (!is_already_set) {
			const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
			const auto new_channel = callback.get <dpp::channel>();
			const std::string to_add = std::to_string(new_channel.id) + ' ' + std::to_string(new_channel.guild_id);
			try {
				db::sql << "INSERT INTO " + (std::string)(is_jtc ? "temp_vc_notifications" : "topgg_notifications") + " VALUES (?, ?);" << new_channel.id.str() << new_channel.guild_id.str();
			}
			catch (sqlite::sqlite_exception& e) {
				std::cout << e.what() << ' ' << e.get_code() << ' ' << e.get_sql() << '\n';
			}
			(is_jtc ? temp_vc_notifications : topgg_notifications)[new_channel.guild_id] = new_channel.id;
			co_await event.co_reply(dpp::message("The channel has been set up!").set_flags(dpp::m_ephemeral));
		}
		else {
			co_await event.co_reply(dpp::message("It's already been set up. No more than one per guild!").set_flags(dpp::m_ephemeral));
		}
	}
}

void slash::blocklist::status(const dpp::slashcommand_t& event) {
	const dpp::snowflake channel_id = temp_vcs[vc_statuses[event.command.usr.id]].channel_id;
	if (channel_id.empty()) {
		event.reply(dpp::message("You're not in a temporary voice channel! Unless you are, in which case simply rejoin.").set_flags(dpp::m_ephemeral));
		return;
	}
	const dpp::snowflake user_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	event.reply(dpp::message(fmt::format("The user is {}in the blocklist of the channel.", banned[channel_id].contains(user_id) ? "" : "not ")).set_flags(dpp::m_ephemeral));
}

dpp::coroutine <> slash::blocklist::add(const dpp::slashcommand_t& event) {
	const dpp::user& issuer = event.command.usr;
	const temp_vc issuer_vc = temp_vcs[vc_statuses[issuer.id]];
	if (issuer_vc.creator_id != issuer.id) {
		event.reply(dpp::message("The channel you\'re in does not belong to you! Unless it does, in which case simply rejoin.").set_flags(dpp::m_ephemeral));
		co_return;
	}
	const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	const dpp::user* requested = dpp::find_user(requested_id);
	if (requested == nullptr) {
		event.reply(dpp::message("Requested user not found. Please, try again.").set_flags(dpp::m_ephemeral));
		co_return;
	}
	dpp::channel* channel = dpp::find_channel(issuer_vc.channel_id);
	const dpp::permission requested_permission = channel->get_user_permissions(requested);
	if (requested_permission.has(dpp::p_administrator)) {
		event.reply(dpp::message("The user has administrator access to this channel!").set_flags(dpp::m_ephemeral));
		co_return;
	}
	if (banned[issuer_vc.channel_id].contains(requested_id)) {
		event.reply(dpp::message("The user is already in the blocklist!").set_flags(dpp::m_ephemeral));
	}
	else {
		channel->set_permission_overwrite(requested_id, dpp::ot_member, 0, dpp::p_view_channel);
		const dpp::confirmation_callback_t callback = co_await bot->co_channel_edit(*channel);
		if (callback.is_error()) {
			event.reply(dpp::message("Error. Couldn't move the user to the blocklist. Tip: the user may have a role that is above my roles.").set_flags(dpp::m_ephemeral));
			error_callback(callback);
			co_return;
		}
		banned[issuer_vc.channel_id].insert(requested_id);
		if (vc_statuses[issuer.id] == vc_statuses[requested_id]) {
			bot->guild_member_move(0, issuer_vc.guild_id, requested_id);
		}
		event.reply(dpp::message("The user was added to the blocklist of the current channel.").set_flags(dpp::m_ephemeral));
	}
}

dpp::coroutine <> slash::blocklist::remove(const dpp::slashcommand_t& event) {
	const dpp::user& issuer = event.command.usr;
	if (temp_vcs[vc_statuses[issuer.id]].creator_id != issuer.id) {
		event.reply(dpp::message("The channel you\'re in does not belong to you! Unless it does, in which case simply rejoin.").set_flags(dpp::m_ephemeral));
		co_return;
	}
	const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	const dpp::user* requested = dpp::find_user(requested_id);
	if (requested == nullptr) {
		event.reply(dpp::message("Requested user not found. Please, try again.").set_flags(dpp::m_ephemeral));
		co_return;
	}
	if (!banned[temp_vcs[vc_statuses[issuer.id]].channel_id].contains(requested_id)) {
		event.reply(dpp::message("The user was not in the blocklist!").set_flags(dpp::m_ephemeral));
	}
	else {
		dpp::channel* channel = dpp::find_channel(vc_statuses[issuer.id]);
		channel->set_permission_overwrite(requested_id, dpp::ot_member, dpp::p_view_channel, 0);
		const dpp::confirmation_callback_t callback = co_await bot->co_channel_edit(*channel);
		if (callback.is_error()) {
			event.reply(dpp::message("Error. Couldn't remove the user from the blocklist. Tip: the user may have a role that is above my roles.").set_flags(dpp::m_ephemeral));
			error_callback(callback);
			co_return;
		}
		banned[temp_vcs[vc_statuses[issuer.id]].channel_id].erase(requested_id);
		event.reply(dpp::message("The user was removed from the blocklist of the current channel.").set_flags(dpp::m_ephemeral));
	}
}

void slash::topgg::guild_get(const dpp::slashcommand_t& event) {
	const dpp::user& user = event.command.usr;
	const dpp::snowflake guild_id = ::topgg::guild_choices[user.id];
	const dpp::guild* guild = dpp::find_guild(guild_id);
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
		if (!guild->get_banner_url().empty()) {
			embed.set_image(guild->get_banner_url());
		}
		const dpp::message message = dpp::message(event.command.channel_id, embed).set_flags(dpp::m_ephemeral);
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
		db::sql << "DELETE FROM topgg_guild_choices WHERE user_id=?;" << event.command.usr.id.str();
		db::sql << "INSERT INTO topgg_guild_choices VALUES (?, ?);" << event.command.usr.id.str() << event.command.guild_id.str();
		dpp::embed embed = dpp::embed()
			.set_color(dpp::colors::sti_blue)
			.set_title("You're now voting in favour of this guild.")
			.set_author(fmt::format("Set your guild to {}.", event.command.get_guild().name), "", event.command.get_guild().get_icon_url())
			.set_thumbnail(event.command.get_guild().get_icon_url()
		);
		const std::string& banner = event.command.get_guild().get_icon_url();
		if (!banner.empty()) {
			embed.set_image(event.command.get_guild().get_banner_url());
		}
		const dpp::message message = dpp::message(event.command.channel_id, embed).set_flags(dpp::m_ephemeral);
		event.reply(message);
	}
}

void slash::topgg::get_progress(const dpp::slashcommand_t& event) {
	const int8_t limit = ::topgg::jtc::count_allowed_jtcs(event.command.guild_id);
	event.reply(dpp::message(event.command.channel_id, fmt::format("This guild's vote progress is: __{0}__/**{1}**. The maximum amount of JTCs allowed here is {2}.{3}", ::topgg::guild_votes_amount[event.command.guild_id], ::topgg::votes_leveling[limit], limit, limit == 10 ? " This is the absolute maximum." : "")).set_flags(dpp::m_ephemeral));
}

dpp::coroutine <> slash::ticket::create(const dpp::slashcommand_t& event) {
	const dpp::snowflake& user_id = event.command.usr.id;
	if (!tickets[user_id].empty()) {
		co_await event.co_reply(dpp::message("You already have a ticket! DM the bot to contact the creator!").set_flags(dpp::m_ephemeral));
		co_return;
	}
	dpp::channel channel = dpp::channel()
		.set_name(event.command.usr.username)
		.set_guild_id(TICKETS_GUILD_ID);
	const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
	channel = callback.get <dpp::channel>();
	co_await bot->co_message_create(dpp::message(channel.id, fmt::format("<@{}> is contacting you.", user_id)));
	tickets[user_id] = channel.id;
	ck_tickets[channel.id] = user_id;
	db::sql << "INSERT INTO tickets VALUES (?, ?);" << user_id.str() << channel.id.str();
	co_await event.co_reply(dpp::message("A ticket has been created.").set_flags(dpp::m_ephemeral));
}

void slash::ticket::close(const dpp::slashcommand_t& event) {
	const dpp::snowflake& user_id = event.command.usr.id;
	if (tickets[user_id].empty()) {
		event.reply(dpp::message("You don't have a ticket, so no tickets were closed.").set_flags(dpp::m_ephemeral));
		return;
	}
	db::sql << "DELETE FROM tickets WHERE user_id=?;" << user_id.str();
	bot->channel_delete(tickets[user_id], error_callback);
	ck_tickets.erase(tickets[user_id]);
	tickets.erase(user_id);
	event.reply(dpp::message("Your ticket has been closed! You can create a new one at any point.").set_flags(dpp::m_ephemeral));
}
