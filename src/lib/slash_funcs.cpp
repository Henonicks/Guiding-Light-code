#include "guiding_light/slash_funcs.hpp"

#include "guiding_light/responses.hpp"

dpp::coroutine <> slash::set::current(const dpp::slashcommand_t &event) {
	get_lang();
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::user user = event.command.get_issuing_user();
	const dpp::snowflake& user_id = user.id;
	const dpp::snowflake& channel_id = vc_statuses[user_id];
	const bool user_is_main = !channel_id.empty();
	if (!user_is_main) {
		event.reply(response_emsg(YOU_ARE_NOT_IN_A_VC_YOU_CAN_EDIT, lang));
	}
	else {
		dpp::channel channel = *dpp::find_channel(channel_id);
		const std::string old_name = channel.name;
		if (cmd.options[0].name == "name") {
			const auto argument = std::get <std::string>(cmd.options[0].options[0].value);
			if (argument == channel.name) {
				event.reply(response_fmtemsg(THE_NAME_OF_THE_VC_IS_ALREADY, lang, {old_name}));
			}
			else {
				channel.set_name(argument);
				bot->channel_edit(channel, [event, old_name, argument, lang](const dpp::confirmation_callback_t& callback) {
					if (callback.is_error()) {
						error_feedback(callback, event);
						return;
					}
					event.reply(response_fmtemsg(THE_NAME_OF_THE_CHANNEL_HAS_CHANGED_FROM, lang, {old_name, argument}));
				});
			}
		}
		else if (cmd.options[0].name == "bitrate") {
			const auto argument = std::get <long>(cmd.options[0].options[0].value);
			const dpp::guild* guild = dpp::find_guild(channel.guild_id);
			if (guild == nullptr) {
				event.reply(response_emsg(GUILD_COULD_NOT_BE_FOUND, lang));
				co_return;
			}
			dpp::message to_reply = dpp::message().set_flags(dpp::m_ephemeral);
			std::string content;
			const int max_bitrate = ((guild->premium_tier == 0) ?
						       96 : (guild->premium_tier == 1) ?
									128 : (guild->premium_tier == 2) ?
										  256 : 384);
			if (channel.bitrate == argument) {
				content = response_fmt(THE_BITRATE_IS_ALREADY, lang, {std::to_string(argument)});
			}
			else if (argument > max_bitrate || argument < 8) {
				content = response_fmt(THE_NUMBER_CAN_ONLY_BE_BETWEEN, lang, {std::to_string(max_bitrate)});
			}
			else {
				content = response_fmt(SET_BITRATE_TO, lang, {std::to_string(argument)});
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
				event.reply(response_emsg(FOR_VOICE_CHANNELS_ITS_IMPOSSIBLE_TO_MAKE_THE_LIMIT_GREATER, lang));
			}
			else if (argument < 0) {
				event.reply(response_emsg(WELL_NOW_WERE_JUST_BEING_SILLY_ARENT_WE, lang));
				bot->guild_member_move(0, temp_vcs[vc_statuses[user_id]].guild_id, user_id, error_callback);
				// easter egg
			}
			else {
				if (channel.user_limit == argument) {
					event.reply(response_fmtemsg(THE_USER_LIMIT_IS_ALREADY, lang, {std::to_string(argument)}));
				}
				else {
					channel.set_user_limit(argument);
					bot->channel_edit(channel, [event, argument, lang](const dpp::confirmation_callback_t& callback) {
						if (callback.is_error()) {
							error_feedback(callback, event);
							return;
						}
						event.reply(response_fmtemsg(USER_LIMIT_IS_SET_TO, lang, {std::to_string(argument)}));
					});
				}
			}
		}
	}
}

dpp::coroutine <> slash::set::default_values(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::snowflake& guild_id = event.command.guild_id;
	const dpp::guild guild = *dpp::find_guild(guild_id);
	const auto channel_id = std::get <dpp::snowflake>(cmd.options[0].options[0].options[1].value);
	const jtc_defaults defs = jtc_default_values[channel_id];
	if (defs.channel_id.empty()) {
		co_await event.co_reply(response_emsg(THIS_IS_NOT_A_JTC_VC_I_KNOW_OF, lang));
		co_return;
	}
	jtc_defaults new_defs;
	if (cmd.options[0].options[0].name == "name") {
		auto name = std::get <std::string>(cmd.options[0].options[0].options[0].value);
		if (defs.name == name) {
			co_await event.co_reply(response_fmtemsg(THE_NAME_IS_ALREADY, lang, {name}));
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
		event.reply(response_fmtemsg(THE_DEFAULT_NAME_IS_SET_TO, lang, {name}));
	}
	else if (cmd.options[0].options[0].name == "limit") {
		const auto limit = std::get <long>(cmd.options[0].options[0].options[0].value);
		if (limit < 0 || limit > 99) {
			event.reply(response_emsg(THE_LIMIT_SHOULD_BE_BETWEEN, lang));
		}
		else {
			if (defs.limit == limit) {
				co_await event.co_reply(response_fmtemsg(THE_LIMIT_IS_ALREADY, lang, {std::to_string(limit)}));
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
			event.reply(response_fmtemsg(THE_DEFAULT_LIMIT_IS_SET_TO, lang, {std::to_string(limit)}));
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
			content = response_fmt(THE_BITRATE_IS_ALREADY, lang, {std::to_string(bitrate)});
		}
		else if (bitrate > max_bitrate || bitrate < 8) {
			content = response_fmt(THE_NUMBER_CAN_ONLY_BE_BETWEEN, lang, {std::to_string(max_bitrate)});
			// Since the maximum bitrate varies between each guild (based on the amount of boosts),
			// this check is needed.
		}
		else {
			content = response_fmt(THE_DEFAULT_BITRATE_IS_SET_TO, lang, {std::to_string(bitrate)});
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
	get_lang();
	const dpp::snowflake& guild_id = event.command.guild_id;
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::guild& guild = *dpp::find_guild(guild_id);
	if (cmd.options[0].name == "jtc") {
		const int8_t limit = ::topgg::jtc::count_allowed_jtcs(guild_id);
		if (jtc_vc_amount[guild_id] >= limit) {
			co_await event.co_reply(response_fmtemsg(THIS_GUILD_HAS_A_JTC_LIMIT_OF, lang,
				{std::to_string(limit), (limit < 10 ? response_str(YOU_CAN_GET_MORE_BY_VOTING_THOUGH, lang) : "")}));
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
			error_callback(callback);
			co_await event.co_reply(response_emsg(TRIED_TO_CREATE_A_JTC_CHANNEL_BUT_FAILED, lang));
			co_return;
		}
		++jtc_vc_amount[guild_id];
		const auto new_channel = std::get <dpp::channel>(callback.value);
		jtc_vcs[new_channel.id] = new_channel.guild_id;
		const jtc_defaults defs = {new_channel.id, "VC for {username}", max, 64};
		jtc_default_values[new_channel.id] = defs;
		db::sql << "INSERT INTO jtc_vcs VALUES (?, ?);" << new_channel.id.str() << new_channel.guild_id.str();
		db::sql << "INSERT INTO jtc_default_values VALUES (?, ?, ?, ?);" << new_channel.id.str() << "VC for {username}" << max << 64;
		co_await event.co_reply(response_emsg(CREATED_A_JTC_CHANNEL, lang));
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
			db::sql << "INSERT INTO " + (std::string)(is_jtc ? "temp_vc_notifications" : "topgg_notifications") + " VALUES (?, ?);" << new_channel.id.str() << new_channel.guild_id.str();
			(is_jtc ? temp_vc_notifications : topgg_notifications)[new_channel.guild_id] = new_channel.id;
			co_await event.co_reply(response_emsg(THE_CHANNEL_HAS_BEEN_SET_UP, lang));
		}
		else {
			co_await event.co_reply(response_emsg(ITS_ALREADY_BEEN_SET_UP, lang));
		}
	}
}

void slash::blocklist::status(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::snowflake channel_id = temp_vcs[vc_statuses[event.command.usr.id]].channel_id;
	if (channel_id.empty()) {
		event.reply(response_emsg(YOURE_NOT_IN_A_TEMP_VC, lang));
		return;
	}
	const dpp::snowflake user_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	event.reply(response_fmtemsg(THE_USER_IS_NOTORIN_THE_BLOCKLIST, lang, {banned[channel_id].contains(user_id) ? "" : response_str(NOT, lang)}));
}

dpp::coroutine <> slash::blocklist::add(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& issuer = event.command.usr;
	const temp_vc issuer_vc = temp_vcs[vc_statuses[issuer.id]];
	if (issuer_vc.creator_id != issuer.id) {
		event.reply(response_emsg(THE_CHANNEL_YOURE_IN_DOES_NOT_BELONG_TO_YOU, lang));
		co_return;
	}
	const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	const dpp::user* requested = dpp::find_user(requested_id);
	if (requested == nullptr) {
		event.reply(response_emsg(REQUESTED_USER_NOT_FOUND, lang));
		co_return;
	}
	dpp::channel* channel = dpp::find_channel(issuer_vc.channel_id);
	const dpp::permission requested_permission = channel->get_user_permissions(requested);
	if (requested_permission.has(dpp::p_administrator)) {
		event.reply(response_emsg(THE_USER_HAS_ADMIN_ACCESS_TO_THIS_CHANNEL, lang));
		co_return;
	}
	if (banned[issuer_vc.channel_id].contains(requested_id)) {
		event.reply(response_emsg(THE_USER_IS_ALREADY_IN_THE_BLOCKLIST, lang));
	}
	else {
		channel->set_permission_overwrite(requested_id, dpp::ot_member, 0, dpp::p_view_channel);
		const dpp::confirmation_callback_t callback = co_await bot->co_channel_edit(*channel);
		if (callback.is_error()) {
			error_feedback(callback, event);
			co_return;
		}
		banned[issuer_vc.channel_id].insert(requested_id);
		if (vc_statuses[issuer.id] == vc_statuses[requested_id]) {
			bot->guild_member_move(0, issuer_vc.guild_id, requested_id);
		}
		event.reply(response_emsg(THE_USER_WAS_ADDED_TO_THE_BLOCKLIST, lang));
	}
}

dpp::coroutine <> slash::blocklist::remove(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& issuer = event.command.usr;
	if (temp_vcs[vc_statuses[issuer.id]].creator_id != issuer.id) {
		event.reply(response_emsg(THE_CHANNEL_YOURE_IN_DOES_NOT_BELONG_TO_YOU, lang));
		co_return;
	}
	const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	const dpp::user* requested = dpp::find_user(requested_id);
	if (requested == nullptr) {
		event.reply(response_emsg(REQUESTED_USER_NOT_FOUND, lang));
		co_return;
	}
	if (!banned[temp_vcs[vc_statuses[issuer.id]].channel_id].contains(requested_id)) {
		event.reply(response_emsg(THE_USER_WAS_NOT_IN_THE_BLOCKLIST, lang));
	}
	else {
		dpp::channel* channel = dpp::find_channel(vc_statuses[issuer.id]);
		channel->set_permission_overwrite(requested_id, dpp::ot_member, dpp::p_view_channel, 0);
		const dpp::confirmation_callback_t& callback = co_await bot->co_channel_edit(*channel);
		if (callback.is_error()) {
			error_feedback(callback, event);
			co_return;
		}
		banned[temp_vcs[vc_statuses[issuer.id]].channel_id].erase(requested_id);
		event.reply(response_emsg(THE_USER_WAS_REMOVED_FROM_THE_BLOCKLIST, lang));
	}
}

void slash::topgg::guild_get(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& user = event.command.usr;
	const dpp::snowflake guild_id = ::topgg::guild_choices[user.id];
	const dpp::guild* guild = dpp::find_guild(guild_id);
	if (guild == nullptr) {
		event.reply(response_fmtemsg(GUILD_NOT_FOUND_IF_YOUVE_ALREADY_SET_IT, lang, {get_mention("guild set")})
			.set_channel_id(event.command.channel_id));
	}
	else {
		dpp::embed embed = dpp::embed()
			.set_color(dpp::colors::sti_blue)
			.set_title(response(YOURE_VOTING_IN_FAVOUR_OF_THIS_GUILD, lang))
			.set_author(response_fmt(HELLO_YOUR_CHOSEN_GUILD_IS, lang, {user.username, guild->name}), "", guild->get_icon_url())
			.set_thumbnail(guild->get_icon_url()
		);
		if (!guild->get_banner_url().empty()) {
			embed.set_image(guild->get_banner_url());
		}
		const dpp::message message = dpp::message(embed).set_flags(dpp::m_ephemeral);
		event.reply(message);
	}
}

void slash::topgg::guild_set(const dpp::slashcommand_t& event) {
	get_lang();
	if (::topgg::guild_choices[event.command.usr.id] == event.command.guild_id) {
		try {
			event.reply(response_fmtemsg(YOUR_CHOSEN_GUILD_IS_ALREADY, lang, {event.command.get_guild().name}));
		}
		catch (...) { // TODO: what the shit is this?
			event.reply(dpp::message(event.command.channel_id, "Guild not found. Try again?").set_flags(dpp::m_ephemeral));
		}
	}
	else {
		::topgg::guild_choices[event.command.usr.id] = event.command.guild_id;
		db::sql << "DELETE FROM topgg_guild_choices WHERE user_id=?;" << event.command.usr.id.str();
		db::sql << "INSERT INTO topgg_guild_choices VALUES (?, ?);" << event.command.usr.id.str() << event.command.guild_id.str();
		dpp::embed embed = dpp::embed()
			.set_color(dpp::colors::sti_blue)
			.set_title(response(YOURE_NOW_VOTING_IN_FAVOUR_OF_THIS_GUILD, lang))
			.set_author(response_fmt(SET_YOUR_GUILD_TO, lang, {event.command.get_guild().name}), "", event.command.get_guild().get_icon_url())
			.set_thumbnail(event.command.get_guild().get_icon_url()
		);
		const std::string& banner = event.command.get_guild().get_icon_url();
		if (!banner.empty()) {
			embed.set_image(event.command.get_guild().get_banner_url());
		}
		const dpp::message message = dpp::message(embed).set_flags(dpp::m_ephemeral);
		event.reply(message);
	}
}

void slash::topgg::get_progress(const dpp::slashcommand_t& event) {
	get_lang();
	const int8_t limit = ::topgg::jtc::count_allowed_jtcs(event.command.guild_id);
	event.reply(response_fmtemsg(THIS_GUILDS_VOTE_PROGRESS_IS, lang,
		{std::to_string(::topgg::guild_votes_amount[event.command.guild_id]), std::to_string(::topgg::votes_leveling[limit]), std::to_string(limit),
			limit == 10 ? response_str(THIS_IS_THE_ABSOLUTE_MAXIMUM, lang) : ""}));
}

dpp::coroutine <> slash::ticket::create(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::snowflake& user_id = event.command.usr.id;
	if (!tickets[user_id].empty()) {
		co_await event.co_reply(response_emsg(YOU_ALREADY_HAVE_A_TICKET, lang));
		co_return;
	}
	dpp::channel channel = dpp::channel()
		.set_name(event.command.usr.username)
		.set_guild_id(TICKETS_GUILD_ID);
	const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
	if (error_feedback(callback, event, response(COULDNT_CREATE_A_CHANNEL_FOR_THE_TICKET, lang))) {
		co_return;
	}
	channel = callback.get <dpp::channel>();
	co_await bot->co_message_create(dpp::message(channel.id, fmt::format("<@{}> is contacting you.", user_id)));
	tickets[user_id] = channel.id;
	ck_tickets[channel.id] = user_id;
	db::sql << "INSERT INTO tickets VALUES (?, ?);" << user_id.str() << channel.id.str();
	co_await event.co_reply(response_emsg(A_TICKET_HAS_BEEN_CREATED, lang));
}

void slash::ticket::close(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::snowflake& user_id = event.command.usr.id;
	if (tickets[user_id].empty()) {
		event.reply(response_emsg(YOU_DONT_HAVE_A_TICKET, lang));
		return;
	}
	db::sql << "DELETE FROM tickets WHERE user_id=?;" << user_id.str();
	bot->channel_delete(tickets[user_id], error_callback);
	ck_tickets.erase(tickets[user_id]);
	tickets.erase(user_id);
	event.reply(response_emsg(YOUR_TICKET_HAS_BEEN_CLOSED, lang));
}
