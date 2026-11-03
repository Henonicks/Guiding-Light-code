#include "guiding_light/handlers/slash_funcs.hpp"

#include "guiding_light/handlers/ratelimit.hpp"
#include "guiding_light/handlers/responses.hpp"
#include "guiding_light/handlers/temp_vc_handler.hpp"
#include "guiding_light/resources/jtc_vc.hpp"
#include "guiding_light/resources/notification_channel.hpp"
#include "guiding_light/database/database.hpp"
#include "guiding_light/internal/config_values.hpp"
#include "guiding_light/handlers/topgg.hpp"
#include "guiding_light/resources/topgg_guild_choice.hpp"
#include "guiding_light/resources/topgg_guild_vote_amount.hpp"
#include "guiding_light/helpers/format.tpp"

henifig::value_t get_help_command_page_names(const std::string_view lang, const henifig::value_map& commands) {
	std::string used_lang = lang.data();
	std::lock_guard L(cfg::config_mutex);
	if (!commands.contains(lang.data())) {
		used_lang = "default";
	}
	return commands.at(used_lang);
}

dpp::coroutine <> slash::tempvc::set(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::command_data_option suboption = cmd.options[0].options[0];
	const dpp::user& user = event.command.get_issuing_user();
	const dpp::snowflake guild_id = event.command.guild_id;
	const dpp::guild& guild = event.command.get_guild();
	std::unique_lock L(temp_vc_mutex);
	const dpp::snowflake channel_id = vc_statuses[user.id][guild.id];
	L.unlock();
	log(std::format("{0} is trying to edit the voice channel {1}.", user.id, channel_id));
	const bool user_is_main = !channel_id.empty();
	if (!user_is_main) {
		log("But the channel is not theirs.");
		event.reply(response_emsg(YOU_ARE_NOT_IN_A_VC_YOU_CAN_EDIT, lang), error_callback);
	}
	else {
		dpp::channel channel = co_await lookup_channel(channel_id);
		const std::string old_name = channel.name;
		if (suboption.name == "name") {
			const auto argument = std::get <std::string>(suboption.options[0].value);
			log(std::format("They are trying to change the name from `{0}` to `{1}`.", old_name, argument));
			if (argument == channel.name) {
				log("But the name is already that.");
				event.reply(response_fmtemsg(THE_NAME_OF_THE_VC_IS_ALREADY, lang, old_name), error_callback);
			}
			else {
				const auto temp_vc = co_await db::get <temp_vc_t>(channel_id);
				if (early_for_channel_rename(temp_vc)) {
					log("But it's too early!");
					event.reply(response_fmtemsg(THIS_CHANNEL_HAS_BEEN_RENAMED_WITHIN_THE_LAST_5_MINUTES, lang, dpp::utility::timestamp(temp_vc.name_timer, dpp::utility::tf_relative_time)), error_callback);
					co_return;
				}
				channel.set_name(argument);
				log("Trying to change the name...");
				const dpp::confirmation_callback_t callback = co_await bot->co_channel_edit(channel);
				if (error_feedback(callback, event, lang)) {
					co_return;
				}
				co_await make_channel_rename(channel_id);
				log("Success.");
				event.reply(response_fmtemsg(THE_NAME_OF_THE_CHANNEL_HAS_CHANGED_FROM, lang, old_name, argument), error_callback);
			}
		}
		else if (suboption.name == "bitrate") {
			const auto argument = std::get <int64_t>(suboption.options[0].value);
			log(std::format("They are trying to change the bitrate from {0} to {1}.", channel.bitrate, argument));
			dpp::message to_reply = dpp::message().set_flags(dpp::m_ephemeral);
			std::string content;
			const int max_bitrate = (
				(guild.premium_tier == 0) ? 96 :
				(guild.premium_tier == 1) ? 128 :
				(guild.premium_tier == 2) ? 256 :
				384
			);
			if (channel.bitrate == argument) {
				content = response_fmt(THE_BITRATE_IS_ALREADY, lang, std::to_string(argument));
				log("But the bitrate is already that.");
			}
			else if (argument > max_bitrate || argument < 8) {
				log("But that bitrate is not allowed in the server.");
				content = response_fmt(THE_NUMBER_CAN_ONLY_BE_BETWEEN, lang, std::to_string(max_bitrate));
			}
			else {
				log("Trying to change the bitrate...");
				content = response_fmt(SET_BITRATE_TO, lang, std::to_string(argument));
				channel.set_bitrate(argument);
				const dpp::confirmation_callback_t& callback = co_await bot->co_channel_edit(channel);
				if (error_feedback(callback, event, lang)) {
					co_return;
				}
				log("Success.");
			}
			to_reply.set_content(content);
			event.reply(to_reply, error_callback);
		}
		else if (suboption.name == "limit") {
			const auto argument = std::get <int64_t>(suboption.options[0].value);
			log(std::format("They are trying to change the limit from {0} to {1}.", channel.user_limit, argument));
			if (argument > 99) {
				log("But the limit is too high.");
				event.reply(response_emsg(FOR_VOICE_CHANNELS_ITS_IMPOSSIBLE_TO_MAKE_THE_LIMIT_GREATER, lang), error_callback);
			}
			else if (argument < 0) {
				log("lmao");
				event.reply(response_emsg(WELL_NOW_WERE_JUST_BEING_SILLY_ARENT_WE, lang), error_callback);
				bot->guild_member_move(0, guild_id, user.id, error_callback);
				// easter egg
			}
			else {
				if (channel.user_limit == argument) {
					log("But the limit is already that.");
					event.reply(response_fmtemsg(THE_USER_LIMIT_IS_ALREADY, lang, std::to_string(argument)), error_callback);
				}
				else {
					log("Trying to change the limit...");
					channel.set_user_limit(argument);
					bot->channel_edit(channel, [event, argument, lang](const dpp::confirmation_callback_t& callback) {
						if (error_feedback(callback, event, lang)) {
							return;
						}
						log("Success.");
						event.reply(response_fmtemsg(USER_LIMIT_IS_SET_TO, lang, std::to_string(argument)), error_callback);
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
	const dpp::snowflake guild_id = event.command.guild_id;
	const dpp::guild guild = co_await lookup_guild(guild_id);
	const auto channel_id = std::get <dpp::snowflake>(cmd.options[0].options[0].options[1].value);
	log(std::format("User {0} tried to change a default value of {1}.", user.id, channel_id));
	auto jtc_vc = co_await db::get <jtc_vc_t>(channel_id);
	if (jtc_vc.channel_id.empty()) {
		log("But that's not a JTC VC.");
		event.reply(response_emsg(THIS_IS_NOT_A_JTC_VC_I_KNOW_OF, lang), error_callback);
		co_return;
	}
	if (cmd.options[0].options[0].name == "name") {
		const auto& name = std::get <std::string>(cmd.options[0].options[0].options[0].value);
		log(std::format("They are trying to change the default name from `{0}` to `{1}`.", jtc_vc.name, name));
		if (jtc_vc.name == name) {
			log("But the default name is already that.");
			event.reply(response_fmtemsg(THE_NAME_IS_ALREADY, lang, name), error_callback);
			co_return;
		}
		jtc_vc.name = name;
		co_await db::update(jtc_vc);
		log("Success.");
		event.reply(response_fmtemsg(THE_DEFAULT_NAME_IS_SET_TO, lang, name), error_callback);
	}
	else if (cmd.options[0].options[0].name == "limit") {
		const auto limit = std::get <int64_t>(cmd.options[0].options[0].options[0].value);
		log(std::format("They are trying to change the default limit from {0} to {1}.", jtc_vc.limit, limit));
		if (limit < 0 || limit > 99) {
			log("But the limit is not within the constraints.");
			event.reply(response_emsg(THE_LIMIT_SHOULD_BE_BETWEEN, lang), error_callback);
			co_return;
		}
		if (jtc_vc.limit == limit) {
			log("But the default limit is already that.");
			event.reply(response_fmtemsg(THE_LIMIT_IS_ALREADY, lang, std::to_string(limit)), error_callback);
			co_return;
		}
		jtc_vc.limit = cast <int8_t>(limit);
		co_await db::update(jtc_vc);
		log("Success.");
		event.reply(response_fmtemsg(THE_DEFAULT_LIMIT_IS_SET_TO, lang, std::to_string(limit)), error_callback);
	}
	else if (cmd.options[0].options[0].name == "bitrate") {
		const auto bitrate = std::get <int64_t>(cmd.options[0].options[0].options[0].value);
		log("They are trying to change the default bitrate.");
		dpp::message to_reply = dpp::message().set_flags(dpp::m_ephemeral);
		std::string content;
		const int max_bitrate = (
			(guild.premium_tier == 0) ? 96 :
			(guild.premium_tier == 1) ? 128 :
			(guild.premium_tier == 2) ? 256 :
			384
		);
		if (jtc_vc.bitrate == bitrate) {
			log("But the default bitrate is already that.");
			content = response_fmt(THE_BITRATE_IS_ALREADY, lang, std::to_string(bitrate));
		}
		else if (bitrate > max_bitrate || bitrate < 8) {
			log("But the bitrate is not within the constraints.");
			content = response_fmt(THE_NUMBER_CAN_ONLY_BE_BETWEEN, lang, std::to_string(max_bitrate));
			// Since the maximum bitrate varies between each guild (based on the amount of boosts),
			// this check is needed.
		}
		else {
			content = response_fmt(THE_DEFAULT_BITRATE_IS_SET_TO, lang, std::to_string(bitrate));
			jtc_vc.bitrate = cast <int16_t>(bitrate);
			co_await db::update(jtc_vc);
			log("Success.");
		}
		to_reply.set_content(content);
		event.reply(to_reply, error_callback);
	}
}

dpp::coroutine <> slash::setup(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& user = event.command.usr;
	const dpp::snowflake guild_id = event.command.guild_id;
	log(std::format("User {0} initiated a channel set up in the guild {1}.", user.id, guild_id));
	const dpp::command_interaction cmd = event.command.get_command_interaction();
	const dpp::guild guild = co_await lookup_guild(guild_id);
	if (cmd.options[0].name == "jtc") {
		log("They are trying to set up a JTC VC.");

		if (std::unique_lock keep_lock(jtc_vc_t::keep_mutex);
			!jtc_vc_t::kept_guilds.contains(guild_id)) {
			jtc_vc_t::kept_guilds.insert(guild_id);
			keep_lock.unlock();
			const auto deleted_channels = co_await db::keep <jtc_vc_t>(guild.id, guild.channels);
			if (!deleted_channels.is_valid()) {
				keep_lock.lock();
				jtc_vc_t::kept_guilds.erase(guild_id);
				event.edit_original_response(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
				co_return;
			}
		}

		// the fact that because co_await is a keyword I can hold the space there is funny af so I decided to just keep it like this
		const int8_t limit = co_await::topgg::jtc::count_allowed_jtcs(guild_id);
		const int8_t jtc_vc_amount = (co_await db::get <jtc_vc_t::count_t>(guild_id)).count;
		if (jtc_vc_amount >= limit) {
			log(std::format("But there are already {0} JTC VCs while the limit is {1}.", jtc_vc_amount, limit));
			event.edit_original_response(response_fmtemsg(THIS_GUILD_HAS_A_JTC_LIMIT_OF, lang,
				std::to_string(limit), (limit < 10 ? response_str(YOU_CAN_GET_MORE_BY_VOTING_THOUGH, lang) : "")), error_callback);
			co_return;
		}
		log("Trying to create a JTC VC.");
		dpp::channel channel;
		channel.set_type(dpp::CHANNEL_VOICE);
		channel.set_parent_id(0);
		channel.set_guild_id(guild_id);
		channel.set_bitrate(64); // The default bitrate on Discord.
		channel.set_user_limit(1); // For the normal users, only one will be able to join the JTC at a given time.
		const auto max = cast <int8_t>(std::get <int64_t>(cmd.options[0].options[0].value));
		channel.set_name(std::format("Join-to-create for {}", max > 0 ? std::to_string(max) : "infinite"));
		const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
		if (error_feedback(callback, event, lang)) {
			co_return;
		}
		const auto new_channel = std::get <dpp::channel>(callback.value);
		const auto jtc_vc = co_await db::create <jtc_vc_t>({new_channel.id, new_channel.guild_id, "VC for {username}", max, 64});
		if (jtc_vc.is_valid()) {
			log("Success.");
			event.edit_original_response(response_emsg(CREATED_A_JTC_CHANNEL, lang), error_callback);
		}
		else {
			event.edit_original_response(response_emsg(COULDNT_WRITE_THE_RESOURCE_TO_THE_DATABASE, lang), error_callback);
			co_return;
		}
	}
	else {
		log("They are trying to set up a notification channel.");

		if (std::unique_lock keep_lock(notification_channel_t::keep_mutex);
			!notification_channel_t::kept_guilds.contains(guild_id)) {
			notification_channel_t::kept_guilds.insert(guild_id);
			keep_lock.unlock();
			const auto deleted_channels = co_await db::keep <notification_channel_t>(guild.id, guild.channels);
			if (!deleted_channels.is_valid()) {
				keep_lock.lock();
				notification_channel_t::kept_guilds.erase(guild_id);
				event.edit_original_response(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
				co_return;
			}
		}

		dpp::channel channel;
		channel.set_type(guild.is_community() ? dpp::CHANNEL_ANNOUNCEMENT : dpp::CHANNEL_TEXT);
		channel.set_permission_overwrite(guild_id, dpp::ot_role, dpp::p_view_channel, dpp::p_send_messages);
		channel.set_permission_overwrite(bot->me.id, dpp::ot_member, dpp::p_view_channel | dpp::p_send_messages, 0);
		channel.set_guild_id(guild_id);
		const notification_source_t source = cmd.options[0].options[0].name == "jtc" ? ns_jtc : ns_topgg;
		const auto notif_channel = co_await db::get <notification_channel_t>(guild_id, source);
		if (!notif_channel.is_valid()) {
			event.edit_original_response(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
			co_return;
		}
		const bool is_already_set = notif_channel.exists();
		if (!is_already_set) {
			switch (source) {
			case ns_jtc:
				log("Specifically, a JTC notification channel.");
				channel.set_name("temp-vc-notifications");
			case ns_topgg:
				log("Specifically, a top.gg notification channel.");
				channel.set_name("topgg-notifications");
			}
			log("Trying to create a notification channel.");
			const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
			const auto new_channel = callback.get <dpp::channel>();
			const notification_channel_t ntif_chnl = co_await db::create <notification_channel_t, "setting up a notification channel">({new_channel.id, guild_id, source});
			if (ntif_chnl.is_valid()) {
				log("Success.");
				event.edit_original_response(response_emsg(THE_CHANNEL_HAS_BEEN_SET_UP, lang), error_callback);
			}
			else {
				event.edit_original_response(response_emsg(COULDNT_WRITE_THE_RESOURCE_TO_THE_DATABASE, lang), error_callback);
			}
		}
		else {
			log("It already exists.");
			event.edit_original_response(response_emsg(ITS_ALREADY_BEEN_SET_UP, lang), error_callback);
		}
	}
}

dpp::coroutine <> slash::tempvc::list::status(const dpp::slashcommand_t& event, const restrictions_types rest_type) {
	get_lang();
	get_rest_list();
	const dpp::user& issuer = event.command.usr;
	const dpp::snowflake guild_id = event.command.guild_id;
	std::unique_lock L(temp_vc_mutex);
	const dpp::snowflake channel_id = vc_statuses[issuer.id][guild_id];
	L.unlock();
	log(std::format("{0} is checking the restrictions list status of the channel {1} with the list type {2}.", issuer.id, channel_id, cast <int>(rest_type)));
	if (channel_id.empty()) {
		log("They're not in a channel though.");
		event.reply(response_emsg(CHANNEL_COULD_NOT_BE_FOUND, lang), error_callback);
		co_return;
	}
	const dpp::snowflake st_user_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	log(std::format("The user they're checking is {}.", st_user_id));
	const dpp::channel channel = co_await lookup_channel(channel_id);
	restlist_updated(channel, rest_type);
	event.reply(response_fmtemsg(THE_USER_IS_NOTORIN_THE_LIST, lang, (*list)[channel_id].contains(st_user_id) ? "" : response_str(NOT, lang)), error_callback);
}

dpp::coroutine <> slash::tempvc::list::add(const dpp::slashcommand_t& event, const restrictions_types rest_type) {
	get_lang();
	get_rest_list();
	const dpp::user& issuer = event.command.usr;
	const dpp::snowflake guild_id = event.command.guild_id;
	std::unique_lock L(temp_vc_mutex);
	const dpp::snowflake channel_id = vc_statuses[issuer.id][guild_id];
	L.unlock();
	const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	log(std::format("User {0} is trying to add {1} to the blocklist of the channel {2}.", issuer.id, requested_id, channel_id));
	const temp_vc_t issuer_vc = co_await db::get <temp_vc_t>(channel_id);
	if (!issuer_vc.is_valid()) {
		event.reply(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
		co_return;
	}
	if (issuer_vc.creator_id != issuer.id) {
		log("But the channel does not belong to them.");
		event.reply(response_emsg(YOU_ARE_NOT_IN_A_VC_YOU_CAN_EDIT, lang), error_callback);
		co_return;
	}
	const dpp::user requested = co_await lookup_user(requested_id);
	dpp::channel channel = co_await lookup_channel(issuer_vc.channel_id);
	restlist_updated(channel, rest_type);
	const dpp::permission requested_permission = channel.get_user_permissions(&requested);
	if (requested_permission.has(dpp::p_administrator)) {
		log("But the requested user is an administrator in said channel.");
		event.reply(response_emsg(THE_USER_HAS_ADMIN_ACCESS_TO_THIS_CHANNEL, lang), error_callback);
	}
	else if ((*list)[issuer_vc.channel_id].contains(requested_id)) {
		log("But the requested user is already in the blocklist.");
		event.reply(response_emsg(THE_USER_IS_ALREADY_IN_THE_BLOCKLIST, lang), error_callback);
	}
	else if (requested.id == bot->me.id) {
		log("But I'm the requested user.");
		event.reply(response_emsg(HEY_THATS_ME_NOT_NICE, lang), error_callback);
	}
	else {
		channel.set_permission_overwrite(requested_id, dpp::ot_member, 0, get_restriction_permissions(rest_type));
		log("Trying to edit the channel...");
		const dpp::confirmation_callback_t& callback = co_await bot->co_channel_edit(channel);
		if (error_feedback(callback, event, lang)) {
			co_return;
		}
		log("Success.");
		std::lock_guard L(temp_vc_mutex);
		if (vc_statuses[issuer.id][guild_id] == vc_statuses[requested_id][guild_id]) {
			bot->guild_member_move(0, issuer_vc.guild_id, requested_id);
		}
		event.reply(response_fmtemsg(
			THE_USER_WAS_ADDED_TO_THE_RESTLIST, lang,
				response_str(
					rest_type == RRT_BLOCKLIST ? BLOCKLIST_OF_CHANNEL :
					                             MUTELIST_OF_CHANNEL,
					lang
				)
			),
		error_callback);
	}
}

dpp::coroutine <> slash::tempvc::list::remove(const dpp::slashcommand_t& event, const restrictions_types rest_type) {
	get_lang();
	get_rest_list();
	const dpp::user& issuer = event.command.usr;
	const dpp::snowflake guild_id = event.command.guild_id;
	std::unique_lock L(temp_vc_mutex);
	const dpp::snowflake channel_id = vc_statuses[issuer.id][guild_id];
	L.unlock();
	const dpp::snowflake requested_id = std::get <dpp::snowflake>(event.get_parameter("user"));
	log(std::format("User {0} is trying to remove {1} from the blocklist of the channel {2}.", issuer.id, requested_id, channel_id));
	const temp_vc_t issuer_vc = co_await db::get <temp_vc_t>(channel_id);
	if (!issuer_vc.is_valid()) {
		event.reply(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
		co_return;
	}
	if (issuer_vc.creator_id != issuer.id) {
		log("But the channel does not belong to them.");
		event.reply(response_emsg(YOU_ARE_NOT_IN_A_VC_YOU_CAN_EDIT, lang), error_callback);
		co_return;
	}
	const dpp::user requested = co_await lookup_user(requested_id);
	if (!(*list)[issuer_vc.channel_id].contains(requested_id)) {
		log("But the requested user is already off the blocklist.");
		event.reply(response_emsg(THE_USER_WAS_NOT_IN_THE_BLOCKLIST, lang), error_callback);
	}
	else {
		dpp::channel channel = co_await lookup_channel(channel_id);
		restlist_updated(channel, rest_type);
		channel.set_permission_overwrite(requested_id, dpp::ot_member, get_restriction_permissions(rest_type), 0);
		log("Trying to edit the channel...");
		const dpp::confirmation_callback_t callback = co_await bot->co_channel_edit(channel);
		if (error_feedback(callback, event, lang)) {
			co_return;
		}
		log("Success.");
		event.reply(response_emsg(THE_USER_WAS_REMOVED_FROM_THE_BLOCKLIST, lang), error_callback);
		if (vc_statuses[requested_id][guild_id] == issuer_vc.channel_id) {
			const dpp::guild_member requested_member = co_await lookup_guild_member(event.command.get_guild().id, requested_id);
			co_await re_unmute_member(requested_member, channel);
		}
	}
}

dpp::coroutine <> slash::topgg::guild_get(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& user = event.command.usr;
	log(std::format("User {0} is trying to get their guild.", user.id));
	const dpp::snowflake guild_id = (co_await db::get <topgg_guild_choice_t>(user.id)).guild_id;
	const dpp::guild guild = co_await lookup_guild(guild_id);
	log(std::format("Found guild {0}.", guild.id));
	dpp::embed embed = dpp::embed()
		.set_color(dpp::colors::sti_blue)
		.set_title(response(YOURE_VOTING_IN_FAVOUR_OF_THIS_GUILD, lang))
		.set_author(response_fmt(HELLO_YOUR_CHOSEN_GUILD_IS, lang, user.username, guild.name), "", guild.get_icon_url())
		.set_thumbnail(guild.get_icon_url()
	);
	if (!guild.get_banner_url().empty()) {
		embed.set_image(guild.get_banner_url());
	}
	const dpp::message message = dpp::message(embed).set_flags(dpp::m_ephemeral);
	event.reply(message, error_callback);
}

dpp::coroutine <> slash::topgg::guild_set(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::user& user = event.command.usr;
	const dpp::guild& guild = event.command.get_guild();
	log(std::format("User {0} is trying to set {1} as their guild.", user.id, guild.id));
	const dpp::snowflake guild_choice = (co_await db::get <topgg_guild_choice_t>(user.id)).guild_id;
	if (guild_choice == guild.id) {
		log("But the guild is already that.");
		event.reply(response_fmtemsg(YOUR_CHOSEN_GUILD_IS_ALREADY, lang, guild.name), error_callback);
	}
	else {
		log("Setting the guild.");
		const topgg_guild_choice_t new_choice = co_await (guild_choice.empty() ? db::create <topgg_guild_choice_t> : db::update <topgg_guild_choice_t>)({user.id, guild.id});
		if (!new_choice.is_valid()) {
			event.reply(response_emsg(COULDNT_WRITE_THE_RESOURCE_TO_THE_DATABASE, lang), error_callback);
			co_return;
		}
		dpp::embed embed = dpp::embed()
			.set_color(dpp::colors::sti_blue)
			.set_title(response(YOURE_NOW_VOTING_IN_FAVOUR_OF_THIS_GUILD, lang))
			.set_author(response_fmt(SET_YOUR_GUILD_TO, lang, guild.name), "", guild.get_icon_url())
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

dpp::coroutine <> slash::topgg::get_progress(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::snowflake guild_id = event.command.guild_id;
	log(std::format("User {0} is getting the voting progress of the guild {1}.", event.command.usr.id, guild_id));

	const int8_t limit = co_await ::topgg::jtc::count_allowed_jtcs(guild_id);
	const int votes = (co_await db::get <topgg_guild_vote_amount_t>(guild_id)).votes;
	event.reply(response_fmtemsg(THIS_GUILDS_VOTE_PROGRESS_IS, lang,
		std::to_string(votes), std::to_string(::topgg::votes_leveling[limit]), std::to_string(limit),
			limit == 10 ? response_str(THIS_IS_THE_ABSOLUTE_MAXIMUM, lang) : ""), error_callback);
}

dpp::coroutine <> slash::ticket::create(const dpp::slashcommand_t& event) {
	event.thinking(true);
	get_lang();
	const dpp::snowflake user_id = event.command.usr.id;
	log(std::format("User {} is trying to create a ticket.", user_id));
	const dpp::guild guild = co_await lookup_guild(TICKETS_GUILD_ID);
	if (!ticket_t::kept) {
		ticket_t::kept = true;
		const auto deleted_channels = co_await db::keep <ticket_t>(guild.channels);
		if (!deleted_channels.is_valid()) {
			ticket_t::kept = false;
			event.edit_original_response(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
			co_return;
		}
	}
	auto ticket = co_await db::get <ticket_t, "", ticket_t::where_user>(user_id);
	if (!ticket.is_valid()) {
		event.edit_original_response(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
		co_return;
	}
	if (ticket.exists()) {
		log(std::format("But there's already the channel {}.", ticket.channel_id));
		event.edit_original_response(response_emsg(YOU_ALREADY_HAVE_A_TICKET, lang), error_callback);
		co_return;
	}
	dpp::channel channel = dpp::channel()
		.set_name(event.command.usr.username)
		.set_guild_id(TICKETS_GUILD_ID);
	log("Trying to create a channel for the ticket...");
	const dpp::confirmation_callback_t& callback = co_await bot->co_channel_create(channel);
	if (error_feedback(callback, event, lang, response(COULDNT_CREATE_A_CHANNEL_FOR_THE_TICKET, lang))) {
		co_return;
	}
	channel = callback.get <dpp::channel>();
	log("Trying to DM the user...");
	const dpp::confirmation_callback_t& dm_callback = co_await bot->co_direct_message_create(user_id, response_emsg(CREATING_A_TICKET_FOR_YOU, lang));
	if (error_callback(dm_callback)) {
		event.edit_original_response(response_emsg(COULDNT_SEND_YOU_A_DM, lang));
		co_return;
	}
	const dpp::snowflake dm_channel_id = dm_callback.get <dpp::message>().channel_id;
	co_await bot->co_message_create(dpp::message(channel.id, std::format("<@{}> is contacting you.", user_id)));
	ticket = co_await db::create <ticket_t>({user_id, channel.id, dm_channel_id});
	if (ticket.is_valid()) {
		log("Success.");
		event.edit_original_response(response_emsg(A_TICKET_HAS_BEEN_CREATED, lang), error_callback);
	}
	else {
		event.edit_original_response(response_emsg(COULDNT_WRITE_THE_RESOURCE_TO_THE_DATABASE, lang), error_callback);
	}
}

dpp::coroutine <> slash::ticket::close(const dpp::slashcommand_t& event) {
	get_lang();
	const dpp::snowflake user_id = event.command.usr.id;
	log(std::format("User {} is trying to close their ticket.", user_id));
	const auto ticket = co_await db::get <ticket_t, "", ticket_t::where_user>(user_id);
	if (!ticket.is_valid()) {
		event.reply(response_emsg(COULDNT_READ_THE_RESOURCE_FROM_THE_DATABASE, lang), error_callback);
		co_return;
	}
	if (!ticket.exists()) {
		log("But they don't even have one. Like, literally, what are you trying to achieve here?");
		event.reply(response_emsg(YOU_DONT_HAVE_A_TICKET, lang), error_callback);
		co_return;
	}
	log(std::format("Deleting the ticket channel {}.", ticket.channel_id));
	const dpp::confirmation_callback_t callback = co_await bot->co_channel_delete(ticket.channel_id);
	if (!error_feedback(callback, event, lang)) {
		event.reply(response_emsg(YOUR_TICKET_HAS_BEEN_CLOSED, lang), error_callback);
	}
}
