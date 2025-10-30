#include "guiding_light/slash_funcs.hpp"

#include "guiding_light/responses.hpp"

dpp::coroutine <> slash::set::current(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::user user = event.command.get_issuing_user();
	const dpp::snowflake& user_id = user.id;
	const dpp::snowflake& channel_id = vc_statuses[user_id];
	log(fmt::format("{0} is trying to edit the voice channel {1}.", user_id, channel_id));
	const bool user_is_main = !channel_id.empty();
	if (!user_is_main) {
		log("But the channel is not theirs.");
		event.reply(response_emsg(YOU_ARE_NOT_IN_A_VC_YOU_CAN_EDIT, lang), error_callback);
	}
	else {
		dpp::channel channel = *dpp::find_channel(channel_id);
		const std::string old_name = channel.name;
		if (cmd.options[0].name == "name") {
			const auto argument = std::get <std::string>(cmd.options[0].options[0].value);
			log(fmt::format("They are trying to change the name from `{0}` to `{1}`.", old_name, argument));
			if (argument == channel.name) {
				log("But the name is already that.");
				event.reply(response_fmtemsg(THE_NAME_OF_THE_VC_IS_ALREADY, lang, {old_name}), error_callback);
			}
			else {
				channel.set_name(argument);
				bot->channel_edit(channel, [event, old_name, argument, lang](const dpp::confirmation_callback_t& callback) {
					log("Trying to change the name...");
					if (callback.is_error()) {
						error_feedback(callback, event);
						return;
					}
					log("Success.");
					event.reply(response_fmtemsg(THE_NAME_OF_THE_CHANNEL_HAS_CHANGED_FROM, lang, {old_name, argument}), error_callback);
				});
			}
		}
		else if (cmd.options[0].name == "bitrate") {
			const auto argument = std::get <long>(cmd.options[0].options[0].value);
			log(fmt::format("They are trying to change the bitrate from {0} to {1}.", channel.bitrate, argument));
			const dpp::guild* guild = dpp::find_guild(channel.guild_id);
			if (guild == nullptr) {
				event.reply(response_emsg(GUILD_COULD_NOT_BE_FOUND, lang), error_callback);
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
				log("But the bitrate is already that.");
			}
			else if (argument > max_bitrate || argument < 8) {
				log("But that bitrate is not allowed in the server.");
				content = response_fmt(THE_NUMBER_CAN_ONLY_BE_BETWEEN, lang, {std::to_string(max_bitrate)});
			}
			else {
				log("Trying to change the bitrate...");
				content = response_fmt(SET_BITRATE_TO, lang, {std::to_string(argument)});
				channel.set_bitrate(argument);
				const dpp::confirmation_callback_t& callback = co_await bot->co_channel_edit(channel);
				if (callback.is_error()) {
					error_feedback(callback, event);
					co_return;
				}
				log("Success.");
			}
			to_reply.set_content(content);
			event.reply(to_reply, error_callback);
		}
		else if (cmd.options[0].name == "limit") {
			const auto argument = std::get <long>(cmd.options[0].options[0].value);
			log(fmt::format("They are trying to change the limit from {0} to {1}.", channel.user_limit, argument));
			if (argument > 99) {
				log("But the limit is too high.");
				event.reply(response_emsg(FOR_VOICE_CHANNELS_ITS_IMPOSSIBLE_TO_MAKE_THE_LIMIT_GREATER, lang), error_callback);
			}
			else if (argument < 0) {
				log("lmao");
				event.reply(response_emsg(WELL_NOW_WERE_JUST_BEING_SILLY_ARENT_WE, lang), error_callback);
				bot->guild_member_move(0, temp_vcs[vc_statuses[user_id]].guild_id, user_id, error_callback);
				// easter egg
			}
			else {
				if (channel.user_limit == argument) {
					log("But the limit is already that.");
					event.reply(response_fmtemsg(THE_USER_LIMIT_IS_ALREADY, lang, {std::to_string(argument)}), error_callback);
				}
				else {
					log("Trying to change the limit...");
					channel.set_user_limit(argument);
					bot->channel_edit(channel, [event, argument, lang](const dpp::confirmation_callback_t& callback) {
						if (callback.is_error()) {
							error_feedback(callback, event);
							return;
						}
						log("Success.");
						event.reply(response_fmtemsg(USER_LIMIT_IS_SET_TO, lang, {std::to_string(argument)}), error_callback);
					});
				}
			}
		}
	}
}

dpp::coroutine <> slash::set::default_values(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& user = event.command.usr;
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::snowflake& guild_id = event.command.guild_id;
	const dpp::guild guild = *dpp::find_guild(guild_id);
	const auto channel_id = std::get <dpp::snowflake>(cmd.options[0].options[0].options[1].value);
	log(fmt::format("User {0} tried to change a default value of {1}.", user.id, channel_id));
	const jtc_defaults defs = jtc_default_values[channel_id];
	if (defs.channel_id.empty()) {
		log("But that's not a JTC VC.");
		co_await event.co_reply(response_emsg(THIS_IS_NOT_A_JTC_VC_I_KNOW_OF, lang));
		co_return;
	}
	jtc_defaults new_defs;
	if (cmd.options[0].options[0].name == "name") {
		auto name = std::get <std::string>(cmd.options[0].options[0].options[0].value);
		log(fmt::format("They are trying to change the default name from `{0}` to `{1}`.", defs.name, name));
		if (defs.name == name) {
			log("But the default name is already that.");
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
		log("Success.");
		event.reply(response_fmtemsg(THE_DEFAULT_NAME_IS_SET_TO, lang, {name}), error_callback);
	}
	else if (cmd.options[0].options[0].name == "limit") {
		const auto limit = std::get <long>(cmd.options[0].options[0].options[0].value);
		log(fmt::format("They are trying to change the default limit from {0} to {1}.", defs.limit, limit));
		if (limit < 0 || limit > 99) {
			log("But the limit is not within the constraints.");
			event.reply(response_emsg(THE_LIMIT_SHOULD_BE_BETWEEN, lang), error_callback);
		}
		else {
			if (defs.limit == limit) {
				log("But the default limit is already that.");
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
			log("Success.");
			event.reply(response_fmtemsg(THE_DEFAULT_LIMIT_IS_SET_TO, lang, {std::to_string(limit)}), error_callback);
		}
	}
	else if (cmd.options[0].options[0].name == "bitrate") {
		auto bitrate = std::get <long>(cmd.options[0].options[0].options[0].value);
		log("They are trying to change the default bitrate.");
		dpp::message to_reply = dpp::message().set_flags(dpp::m_ephemeral);
		std::string content;
		int max_bitrate = ((guild.premium_tier == 0) ?
			96 : (guild.premium_tier == 1) ?
			128 : (guild.premium_tier == 2) ?
			256 : 384);
		if (defs.bitrate == bitrate) {
			log("But the default bitrate is already that.");
			content = response_fmt(THE_BITRATE_IS_ALREADY, lang, {std::to_string(bitrate)});
		}
		else if (bitrate > max_bitrate || bitrate < 8) {
			log("But the bitrate is not within the constraints.");
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
			log("Success.");
		}
		to_reply.set_content(content);
		event.reply(to_reply, error_callback);
	}
}

dpp::coroutine <> slash::setup(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& user = event.command.usr;
	const dpp::snowflake& user_id = user.id;
	const dpp::snowflake& guild_id = event.command.guild_id;
	log(fmt::format("User {0} initiated a channel set up in the guild {1}.", user_id, guild_id));
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::guild& guild = *dpp::find_guild(guild_id);
	if (cmd.options[0].name == "jtc") {
		log("They are trying to set up a JTC VC.");
		const int8_t limit = ::topgg::jtc::count_allowed_jtcs(guild_id);
		if (jtc_vc_amount[guild_id] >= limit) {
			log(fmt::format("But there are already {0} JTC VCs while the limit is {1}.", jtc_vc_amount[guild_id], limit));
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
		channel.set_bitrate(64); // The default bitrate on Discord.
		channel.set_user_limit(1); // For the normal users, only one will be able to create a JTC at the same time.
		log("Trying to create a JTC VC.");
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
		log("Success.");
		co_await event.co_reply(response_emsg(CREATED_A_JTC_CHANNEL, lang));
	}
	else {
		log("They are trying to set up a notification channel.");
		bool is_already_set;
		dpp::channel channel;
		channel.set_type(guild.is_community() ? dpp::CHANNEL_ANNOUNCEMENT : dpp::CHANNEL_TEXT);
		channel.set_permission_overwrite(guild_id, dpp::ot_role, dpp::p_view_channel, dpp::p_send_messages);
		channel.set_permission_overwrite(bot->me.id, dpp::ot_member, dpp::p_view_channel | dpp::p_send_messages, 0);
		channel.set_guild_id(guild_id);
		const bool is_jtc = cmd.options[0].options[0].name == "jtc";
		if (is_jtc) {
			log("Specifically, a JTC notification channel.");
			is_already_set = !temp_vc_notifications[guild_id].empty();
			channel.set_name("temp-vc-notifications");
		}
		else {
			log("Specifically, a top.gg notification channel.");
			is_already_set = !topgg_notifications[guild_id].empty();
			channel.set_name("topgg-notifications");
		}
		if (!is_already_set) {
			log("Trying to create a notification channel.");
			const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
			const auto new_channel = callback.get <dpp::channel>();
			const std::string to_add = std::to_string(new_channel.id) + ' ' + std::to_string(new_channel.guild_id);
			db::sql << "INSERT INTO " + (std::string)(is_jtc ? "temp_vc_notifications" : "topgg_notifications") + " VALUES (?, ?);" << new_channel.id.str() << new_channel.guild_id.str();
			(is_jtc ? temp_vc_notifications : topgg_notifications)[new_channel.guild_id] = new_channel.id;
			log("Success.");
			co_await event.co_reply(response_emsg(THE_CHANNEL_HAS_BEEN_SET_UP, lang));
		}
		else {
			log("It already exists.");
			co_await event.co_reply(response_emsg(ITS_ALREADY_BEEN_SET_UP, lang));
		}
	}
}

void slash::blocklist::status(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& issuer = event.command.usr;
	const dpp::snowflake channel_id = temp_vcs[vc_statuses[issuer.id]].channel_id;
	log(fmt::format("{0} is checking the blocklist status of the channel {1}.", issuer.id, channel_id));
	if (channel_id.empty()) {
		log("They're not in a channel though.");
		event.reply(response_emsg(YOURE_NOT_IN_A_TEMP_VC, lang), error_callback);
		return;
	}
	const dpp::snowflake st_user_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	log(fmt::format("The user they're checking is {}.", st_user_id));
	event.reply(response_fmtemsg(THE_USER_IS_NOTORIN_THE_BLOCKLIST, lang, {banned[channel_id].contains(st_user_id) ? "" : response_str(NOT, lang)}), error_callback);
}

dpp::coroutine <> slash::blocklist::add(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& issuer = event.command.usr;
	const temp_vc issuer_vc = temp_vcs[vc_statuses[issuer.id]];
	const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	log(fmt::format("User {0} is trying to add {1} to the blocklist of the channel {2}.", issuer.id, requested_id, issuer_vc.channel_id));
	if (issuer_vc.creator_id != issuer.id) {
		log("But the channel does not belong to them.");
		event.reply(response_emsg(THE_CHANNEL_YOURE_IN_DOES_NOT_BELONG_TO_YOU, lang), error_callback);
		co_return;
	}
	const dpp::user* requested = dpp::find_user(requested_id);
	if (requested == nullptr) {
		log("But the requested user could not be found.");
		event.reply(response_emsg(REQUESTED_USER_NOT_FOUND, lang), error_callback);
		co_return;
	}
	dpp::channel* channel = dpp::find_channel(issuer_vc.channel_id);
	const dpp::permission requested_permission = channel->get_user_permissions(requested);
	if (requested_permission.has(dpp::p_administrator)) {
		log("But the requested user is an administrator in said channel.");
		event.reply(response_emsg(THE_USER_HAS_ADMIN_ACCESS_TO_THIS_CHANNEL, lang), error_callback);
	}
	else if (banned[issuer_vc.channel_id].contains(requested_id)) {
		log("But the requested user is already in the blocklist.");
		event.reply(response_emsg(THE_USER_IS_ALREADY_IN_THE_BLOCKLIST, lang), error_callback);
	}
	else {
		channel->set_permission_overwrite(requested_id, dpp::ot_member, 0, dpp::p_view_channel);
		log("Trying to edit the channel...");
		const dpp::confirmation_callback_t callback = co_await bot->co_channel_edit(*channel);
		if (callback.is_error()) {
			error_feedback(callback, event);
			co_return;
		}
		log("Success.");
		banned[issuer_vc.channel_id].insert(requested_id);
		if (vc_statuses[issuer.id] == vc_statuses[requested_id]) {
			bot->guild_member_move(0, issuer_vc.guild_id, requested_id);
		}
		event.reply(response_emsg(THE_USER_WAS_ADDED_TO_THE_BLOCKLIST, lang), error_callback);
	}
}

dpp::coroutine <> slash::blocklist::remove(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& issuer = event.command.usr;
	const temp_vc issuer_vc = temp_vcs[vc_statuses[issuer.id]];
	const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	log(fmt::format("User {0} is trying to remove {1} from the blocklist of the channel {2}.", issuer.id, requested_id, issuer_vc.channel_id));
	if (issuer_vc.creator_id != issuer.id) {
		log("But the channel does not belong to them.");
		event.reply(response_emsg(THE_CHANNEL_YOURE_IN_DOES_NOT_BELONG_TO_YOU, lang), error_callback);
		co_return;
	}
	const dpp::user* requested = dpp::find_user(requested_id);
	if (requested == nullptr) {
		log("But the requested user could not be found.");
		event.reply(response_emsg(REQUESTED_USER_NOT_FOUND, lang), error_callback);
	}
	else if (!banned[issuer_vc.channel_id].contains(requested_id)) {
		log("But the requested user is already off the blocklist.");
		event.reply(response_emsg(THE_USER_WAS_NOT_IN_THE_BLOCKLIST, lang), error_callback);
	}
	else {
		dpp::channel* channel = dpp::find_channel(vc_statuses[issuer.id]);
		channel->set_permission_overwrite(requested_id, dpp::ot_member, dpp::p_view_channel, 0);
		log("Trying to edit the channel...");
		const dpp::confirmation_callback_t& callback = co_await bot->co_channel_edit(*channel);
		if (callback.is_error()) {
			error_feedback(callback, event);
			co_return;
		}
		banned[issuer_vc.channel_id].erase(requested_id);
		log("Success.");
		event.reply(response_emsg(THE_USER_WAS_REMOVED_FROM_THE_BLOCKLIST, lang), error_callback);
	}
}

void slash::topgg::guild_get(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& user = event.command.usr;
	log(fmt::format("User {0} is trying to get their guild.", user.id));
	const dpp::snowflake guild_id = ::topgg::guild_choices[user.id];
	const dpp::guild* guild = dpp::find_guild(guild_id);
	if (guild == nullptr) {
		log("Not found.");
		event.reply(response_fmtemsg(GUILD_NOT_FOUND_IF_YOUVE_ALREADY_SET_IT, lang, {get_mention("guild set")})
			.set_channel_id(event.command.channel_id), error_callback);
	}
	else {
		log(fmt::format("Found guild {0}.", guild->id));
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
		event.reply(message, error_callback);
	}
}

void slash::topgg::guild_set(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& user = event.command.usr;
	const dpp::guild& guild = event.command.get_guild();
	log(fmt::format("User {0} is trying to set {1} as their guild.", user.id, guild.id));
	if (::topgg::guild_choices[user.id] == guild.id) {
		log("But the guild is already that.");
		event.reply(response_fmtemsg(YOUR_CHOSEN_GUILD_IS_ALREADY, lang, {guild.name}), error_callback);
	}
	else {
		log("Setting the guild.");
		::topgg::guild_choices[user.id] = guild.id;
		db::sql << "DELETE FROM topgg_guild_choices WHERE user_id=?;" << user.id.str();
		db::sql << "INSERT INTO topgg_guild_choices VALUES (?, ?);" << user.id.str() << guild.id.str();
		dpp::embed embed = dpp::embed()
			.set_color(dpp::colors::sti_blue)
			.set_title(response(YOURE_NOW_VOTING_IN_FAVOUR_OF_THIS_GUILD, lang))
			.set_author(response_fmt(SET_YOUR_GUILD_TO, lang, {guild.name}), "", guild.get_icon_url())
			.set_thumbnail(guild.get_icon_url()
		);
		const std::string& banner = guild.get_icon_url();
		if (!banner.empty()) {
			embed.set_image(guild.get_banner_url());
		}
		const dpp::message message = dpp::message(embed).set_flags(dpp::m_ephemeral);
		event.reply(message, error_callback);
	}
}

void slash::topgg::get_progress(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::snowflake& guild_id = event.command.guild_id;
	log(fmt::format("User {0} is getting the voting progress of the guild {1}.", event.command.usr.id, guild_id));
	const int8_t limit = ::topgg::jtc::count_allowed_jtcs(guild_id);
	event.reply(response_fmtemsg(THIS_GUILDS_VOTE_PROGRESS_IS, lang,
		{std::to_string(::topgg::guild_votes_amount[guild_id]), std::to_string(::topgg::votes_leveling[limit]), std::to_string(limit),
			limit == 10 ? response_str(THIS_IS_THE_ABSOLUTE_MAXIMUM, lang) : ""}), error_callback);
}

dpp::coroutine <> slash::ticket::create(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::snowflake& user_id = event.command.usr.id;
	log(fmt::format("User {} is trying to create a ticket.", user_id));
	if (!tickets[user_id].empty()) {
		log(fmt::format("But there's already the channel {}.", tickets[user_id]));
		co_await event.co_reply(response_emsg(YOU_ALREADY_HAVE_A_TICKET, lang));
		co_return;
	}
	dpp::channel channel = dpp::channel()
		.set_name(event.command.usr.username)
		.set_guild_id(TICKETS_GUILD_ID);
	log("Trying to create a channel for the ticket...");
	const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
	if (error_feedback(callback, event, response(COULDNT_CREATE_A_CHANNEL_FOR_THE_TICKET, lang))) {
		co_return;
	}
	channel = callback.get <dpp::channel>();
	co_await bot->co_message_create(dpp::message(channel.id, fmt::format("<@{}> is contacting you.", user_id)));
	tickets[user_id] = channel.id;
	ck_tickets[channel.id] = user_id;
	db::sql << "INSERT INTO tickets VALUES (?, ?);" << user_id.str() << channel.id.str();
	log("Success.");
	co_await event.co_reply(response_emsg(A_TICKET_HAS_BEEN_CREATED, lang));
}

void slash::ticket::close(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::snowflake& user_id = event.command.usr.id;
	log(fmt::format("User {} is trying to close their ticket.", user_id));
	if (tickets[user_id].empty()) {
		log("But they don't even have one. Like, literally, what are you trying to achieve here?");
		event.reply(response_emsg(YOU_DONT_HAVE_A_TICKET, lang), error_callback);
		return;
	}
	log(fmt::format("Deleting the ticket channel {}.", tickets[user_id]));
	db::sql << "DELETE FROM tickets WHERE user_id=?;" << user_id.str();
	bot->channel_delete(tickets[user_id], error_callback);
	ck_tickets.erase(tickets[user_id]);
	tickets.erase(user_id);
	event.reply(response_emsg(YOUR_TICKET_HAS_BEEN_CLOSED, lang), error_callback);
}
