#include "guiding_light/temp_vc_handler.hpp"

#include "guiding_light/logging.hpp"

temp_id_t create_temp_vc_request(const dpp::snowflake user_id, const dpp::snowflake channel_id, const dpp::snowflake guild_id) {
	std::lock_guard L(temp_vc_mutex);
	const temp_id_t curr_id = current_temp_vc_request_ID++;
	temp_vcs_queue.emplace(curr_id, user_id, channel_id, guild_id);
	return curr_id;
}

temp_id_t create_temp_vc_id() {
	return current_temp_vc_request_ID++;
}

temp_vc_query get_oldest_temp_vc_request() {
	std::lock_guard L(temp_vc_mutex);
	return !temp_vcs_queue.empty() ? temp_vcs_queue.front() : temp_vc_query();
}

void delete_temp_vc_request() {
	std::lock_guard L(temp_vc_mutex);
	log(fmt::format("Popping query number {} from the queue.", temp_vcs_queue.front().id - LOWEST_QUERY_ID + 1));
	temp_vc_queue_cv.notify_all();
	temp_vcs_queue.pop();
}

void temp_vc_create_msg(const dpp::snowflake& guild_id, const dpp::channel& channel) {
	const std::string description = fmt::format("A new temporary channel has been created {0}. Join the channel, **{1}** (<#{2}>)!",
		!channel.parent_id.empty() ? fmt::format("in the <#{}> category", channel.parent_id) : "outside the categories",
		channel.name, channel.id.str()
	);
	const dpp::embed temp_vc_create_embed = dpp::embed()
		.set_color(dpp::colors::greenish_blue)
		.set_description(description);
	log("Sending the temp vc creation notification.");
	std::lock_guard L(temp_vc_mutex);
	bot->message_create(dpp::message(temp_vc_notifications[guild_id], temp_vc_create_embed), error_callback);
}

void temp_vc_create_owner_msg(const dpp::channel& channel) {
	const dpp::user user = *dpp::find_user(get_oldest_temp_vc_request().user_id);
	std::scoped_lock L(notification_mutex, temp_vc_mutex, cfg_values_mutex);
	log(fmt::format("`{0}` ({1}) Created a temp VC. Guild ID: {2}, channel ID: {3}, channel name: `{4}`, notification channel ID: {5}",
		user.format_username(), user.id, channel.guild_id, channel.id,
		channel.name, temp_vc_notifications[channel.guild_id])
	);
	if (no_temp_ping[user.id]) {
		log(fmt::format("{} has temporary pings off.", user.id));
		return;
	}
	const dpp::embed temp_ping_embed = dpp::embed()
		.set_color(dpp::colors::sti_blue)
		.set_title(fmt::format("Welcome to <#{}>!", channel.id))
		.set_author(fmt::format("This VC belongs to {}.", user.username), user.get_url(), user.get_avatar_url())
		.add_field(
			"You're able to edit the channel!",
			"Use a subcommand of the `/set` command to change the name, limit, or bitrate of your channel to whatever value your soul desires. See `/help` (not to be confused with \"seek help\") for more information."
		)
		.set_footer(
			dpp::embed_footer()
			.set_text("Use the button bellow to toggle the temporary VC creation ping on/off. Have fun!")
		);
	const dpp::message message = dpp::message(channel.id, user.get_mention()).add_embed(temp_ping_embed).add_component(
		dpp::component().add_component(
			dpp::component()
			.set_type(dpp::cot_button)
			.set_emoji(PING_EMOJI.name, PING_EMOJI.id, PING_EMOJI.is_animated())
			.set_style(dpp::cos_danger)
			.set_id("temp_ping_toggle")
		)
	).set_allowed_mentions(true);
	log("Sending the \"You own this channel!\" message.");
	bot->message_create(message, error_callback);
}

void temp_vc_delete_with_msg(const dpp::snowflake channel_id) {
	std::lock_guard L(temp_vc_mutex);
	const dpp::user user = *dpp::find_user(temp_vcs[channel_id].creator_id);
	const dpp::channel channel = *dpp::find_channel(channel_id);
	log(fmt::format("`{0}` ({1}) left a temp VC. Guild ID: {2}, channel ID: {3}, channel name: `{4}`, notification channel ID: {5}",
		user.format_username(), user.id, channel.guild_id, channel.id,
		channel.name, temp_vc_notifications[channel.guild_id])
	);
	const dpp::snowflake& notification_channel_id = temp_vc_notifications[channel.guild_id];
	log(fmt::format("Does their server ({}) have a temp vc notification channel though?", channel.guild_id));
	if (!notification_channel_id.empty()) {
		log(fmt::format("Yes, it does, it's {}.", channel.id));
		std::string description = "A temporary channel **" + channel.name + "**";
		log("Is it in a category?");
		const dpp::channel* category = dpp::find_channel(channel.parent_id);
		if (category != nullptr) {
			log(fmt::format("Yes, it is, in the category {}.", category->id));
			description += " in the **" + category->get_mention() + "** category";
		}
		else {
			log("No, it's not.");
		}
		description += " has been deleted.";
		const dpp::embed temp_vc_delete_message = dpp::embed().
			set_color(dpp::colors::blood_night).
			set_description(description);
		log("Sending the temp vc deletion announcement.");
		bot->message_create(dpp::message(notification_channel_id, temp_vc_delete_message), error_callback);
	}
	bot->channel_delete(channel.id, error_callback);
}

void temp_vc_create(const dpp::voice_state_update_t& event) {
	const dpp::user* const usr = dpp::find_user(event.state.user_id);
	dpp::snowflake jtc_channel_id = event.state.channel_id;
	dpp::snowflake guild_id = event.state.guild_id;
	if (usr == nullptr) {
		log(fmt::format("User not found. Channel: {}", jtc_channel_id));
		if (!jtc_channel_id.empty()) {
			bot->message_create(dpp::message(jtc_channel_id, "Couldn't get user info. Can't create a temp VC."), error_callback);
		}
		return;
	}
	const dpp::user user = *usr;
	log(fmt::format("Starting the setup of a temporary channel for {}.", user.id));
	bool to_return = false;
	const uint64_t current_time = bot->uptime().to_secs();
	std::unique_lock L1(temp_vc_mutex);
	if (current_time - join_time[user.id] <= 5) {
		bot->message_create(dpp::message(jtc_channel_id,
			fmt::format("<@{}> you have to wait more before joining the voice channel. To create a temporary VC as soon as possible, disconnect and try again in 5 seconds.", user.id)
		).set_allowed_mentions(true), error_callback);
		to_return = true;
	}
	join_time[user.id] = current_time;
	bot->start_timer([user](const dpp::timer& h) {
		std::lock_guard L(temp_vc_mutex);
		join_time.erase(user.id);
		bot->stop_timer(h);
	}, 5);
	L1.unlock();
	if (to_return) {
		bot->guild_member_move(0, guild_id, user.id, error_callback);
		return;
	}
	log("The user has passed the basic checks. Pushing to the queue.");
	const temp_id_t curr_id = create_temp_vc_request(user.id, jtc_channel_id, guild_id);
	bot->queue_work(curr_id, [user, jtc_channel_id, guild_id, curr_id]() -> void {
		const std::string& username = user.username;
		std::string new_name;
		dpp::channel new_channel;
		new_channel.set_type(dpp::channel_type::CHANNEL_VOICE);
		std::unique_lock L2(jtc_mutex);
		const jtc_defaults& defs = jtc_default_values[jtc_channel_id];
		L2.unlock();
		for (int i = 0; i < cast <int>(defs.name.size()); i++) {
			if (defs.name[i] == '{') {
				if (defs.name.size() - i >= 10) { // text {username}
					std::string temp_string;      // 0123456789	14
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
			new_name += defs.name[i];
		}
		if (new_name.size() > 100) {
			// Usernames come in different shapes and sizes which means
			// they can make the size of the name higher than 100.
			// In that case we're gonna replace each occurrence with
			// Its first 10 letters.
			std::string newer_name;
			for (int i = 0; i < cast <int>(defs.name.size()); i++) {
				if (defs.name[i] == '{') {
					if (defs.name.size() - i >= 10) { // text {username}
						std::string temp_string;	  // 0123456789	14
						for (int j = i; j < i + 10; j++) {
							temp_string += defs.name[j];
						}
						if (temp_string == "{username}") {
							newer_name += username.substr(0, 10);
							i += 9;
							continue;
						}
					}
				}
				newer_name += defs.name[i];
			}
			new_name = newer_name;
		}
		const int8_t limit = defs.limit;
		new_channel.set_name(new_name);
		new_channel.set_guild_id(guild_id);
		new_channel.set_bitrate(defs.bitrate);
		const dpp::channel current_channel = *dpp::find_channel(jtc_channel_id);
		new_channel.set_parent_id(current_channel.parent_id);
		new_channel.set_user_limit(limit);
		std::mutex curr_query_mutex;
		std::unique_lock curr_query_lock(curr_query_mutex);
		log(fmt::format("Waiting for {}'s turn to have a temporary VC created for them.", user.id));
		temp_vc_queue_cv.wait(curr_query_lock, [curr_id] {
			return get_oldest_temp_vc_request().id == curr_id;
		});
		curr_query_lock.unlock();
		std::unique_lock L3(temp_vc_mutex);
		if (temp_vc_amount[guild_id] >= 50) {
			log(fmt::format("The guild {0} already has {1} temporary VCs, over the limit.",
				guild_id, temp_vc_amount[guild_id])
			);
			delete_temp_vc_request();
			bot->message_create(dpp::message(jtc_channel_id, fmt::format("<@{0}> There are too many temporary VCs in this guild ({1}/50).",
				user.id, temp_vc_amount[guild_id]))
			.set_allowed_mentions(true), error_callback);
			bot->guild_member_move(0, guild_id, user.id, error_callback);
			return;
		}
		L3.unlock();
		const dpp::guild guild = *dpp::find_guild(guild_id);
		if (guild.permission_overwrites(dpp::find_guild_member(guild_id, user.id), current_channel).can(dpp::p_manage_roles)) {
			const dpp::role self_highest_role = get_highest_role(bot->me.id, guild_id);
			for (const dpp::permission_overwrite& x : current_channel.permission_overwrites) {
				if (x.deny.can(dpp::p_view_channel) && x.id != bot->me.id) {
					new_channel.add_permission_overwrite(x.id, cast <dpp::overwrite_type>(x.type), 0, dpp::p_view_channel);
				}
			}
			new_channel.add_permission_overwrite(bot->me.id, dpp::ot_member, dpp::p_view_channel, 0);
			new_channel.add_permission_overwrite(user.id, dpp::ot_member, dpp::p_view_channel, 0);
		}
		log(fmt::format("Creating a temporary VC for {}", get_oldest_temp_vc_request().user_id));
		bot->channel_create(new_channel, [user, current_channel, guild_id, jtc_channel_id](const dpp::confirmation_callback_t& channel_callback) -> void {
			std::unique_lock L4(temp_vc_mutex);
			log("A callback has arrived!");
			if (error_pingback(channel_callback, current_channel.id, user.id)) {
				delete_temp_vc_request();
				log("It happens to be erroneous.");
				return;
			}
			++temp_vc_amount[guild_id];
			const auto channel = channel_callback.get <dpp::channel>();
			temp_vcs[channel.id] = {get_oldest_temp_vc_request().id, channel.id, channel.guild_id, user.id, jtc_channel_id};
			temp_vc_create_owner_msg(channel);
			delete_temp_vc_request();
			L4.unlock();
			bot->guild_member_move(channel.id, channel.guild_id, user.id, [channel, guild_id, user, jtc_channel_id](const dpp::confirmation_callback_t& move_callback) -> void {
				if (error_callback(move_callback)) {
					log(fmt::format("Above is the reason I couldn't move {} into the new VC, deleting it.", user.id));
					bot->channel_delete(channel.id, error_callback);
					return;
				}
				std::unique_lock L5(temp_vc_mutex);
				db::sql << "INSERT INTO temp_vcs VALUES (?, ?, ?, ?);" << channel.id.str() << channel.guild_id.str() << user.id.str() << jtc_channel_id.str();
				log("Does this guild have a temp vc notification channel?");
				if (!temp_vc_notifications[guild_id].empty()) {
					log(fmt::format("Yes, it does, it's {}.", channel.id));
					temp_vc_create_msg(guild_id, channel);
				}
				else {
					log("It does not.");
				}
				log(fmt::format("Finished the temporary VC creation process for {}.", user.id));
			});
		});
	});
}

bool temp_vc_is_accessible(const dpp::permission& overwrite) {
	return overwrite.can(dpp::p_view_channel) && overwrite.can(dpp::p_connect);
}

bool temp_vc_is_accessible(const dpp::permission_overwrite& overwrite) {
	return !overwrite.deny.can(dpp::p_view_channel) && !overwrite.deny.can(dpp::p_connect);
}

bool temp_vc_is_speakable(const dpp::permission& overwrite) {
	return overwrite.can(dpp::p_speak);
}

bool temp_vc_is_speakable(const dpp::permission_overwrite& overwrite) {
	return !overwrite.deny.can(dpp::p_speak);
}

dpp::role get_highest_role(const dpp::snowflake user_id, const dpp::snowflake guild_id) {
	std::vector <dpp::role> own_roles;
	std::ranges::transform(dpp::find_guild_member(guild_id, user_id).get_roles(), std::back_inserter(own_roles), [](const dpp::snowflake role_id) -> dpp::role {
		return *dpp::find_role(role_id);
	});
	return *std::max_element(own_roles.begin(), own_roles.end());
}

dpp::coroutine <> re_unmute_member(dpp::guild_member member, const dpp::channel& channel) {
	const dpp::guild_member bot_member = dpp::find_guild_member(member.guild_id, bot->me.id);
	const dpp::permission own_perms = dpp::find_guild(member.guild_id)->permission_overwrites(bot_member, channel);
	if (!member.is_muted() && own_perms.can(dpp::p_mute_members, dpp::p_deafen_members)) {
		log("Trying to re-unmute the member...");
		member.set_mute(true);
		const dpp::confirmation_callback_t mute_callback = co_await bot->co_guild_edit_member(member);
		if (error_callback(mute_callback)) {
			log("Failed to mute.");
			co_return;
		}
		member = dpp::find_guild_member(member.guild_id, member.user_id);
		member.set_mute(false);
		const dpp::confirmation_callback_t unmute_callback = co_await bot->co_guild_edit_member(member);
		if (error_callback(unmute_callback)) {
			log("Failed to unmute.");
			co_return;
		}
		log("Success.");
	}
}

bool blocklist_updated(const dpp::channel& channel) {
	std::lock_guard L(restriction_mutex);
	bool flag{};
	for (const auto& x : channel.permission_overwrites) {
		if (banned[channel.id].contains(x.id)) {
			if (temp_vc_is_accessible(x)) {
				flag = true;
				banned[channel.id].erase(x.id);
			}
		}
		else if (x.type == dpp::ot_member) {
			if (!temp_vc_is_accessible(x)) {
				flag = true;
				banned[channel.id].insert(x.id);
			}
		}
	}
	return flag;
}

bool mutelist_updated(const dpp::channel& channel) {
	std::lock_guard L(restriction_mutex);
	bool flag{};
	for (const auto& x : channel.permission_overwrites) {
		if (muted[channel.id].contains(x.id)) {
			if (temp_vc_is_speakable(x)) {
				flag = true;
				muted[channel.id].erase(x.id);
			}
		}
		else if (x.type == dpp::ot_member) {
			if (!temp_vc_is_speakable(x)) {
				flag = true;
				muted[channel.id].insert(x.id);
			}
		}
	}
	return flag;
}
