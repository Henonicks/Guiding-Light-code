#include "guiding_light/handlers/temp_vc_handler.hpp"

#include "guiding_light/internal/logging.hpp"
#include "guiding_light/resources/temp_ping_blocker.hpp"
#include "guiding_light/resources/notification_channel.hpp"
#include "guiding_light/internal/cfg.hpp"
#include "guiding_light/internal/config_values.hpp"
#include "guiding_light/resources/jtc_vc.hpp"
#include "guiding_light/database/database.hpp"
#include "guiding_light/handlers/topgg.hpp"
#include "guiding_light/helpers/format.tpp"

temp_id_t create_temp_vc_request(const dpp::snowflake user_id, const dpp::snowflake channel_id, const dpp::snowflake guild_id) {
	std::lock_guard L(temp_vc_mutex);
	const temp_id_t curr_id = current_temp_vc_request_ID++;
	temp_vc_queue.emplace(curr_id, user_id, channel_id, guild_id);
	return curr_id;
}

temp_id_t create_temp_vc_id() {
	return current_temp_vc_request_ID++;
}

temp_vc_query get_oldest_temp_vc_request() {
	std::lock_guard L(temp_vc_mutex);
	return !temp_vc_queue.empty() ? temp_vc_queue.front() : temp_vc_query();
}

void delete_temp_vc_request() {
	std::lock_guard L(temp_vc_mutex);
	log(std::format("Popping query number {} from the queue.", temp_vc_queue.front().id - LOWEST_QUERY_ID + 1));
	temp_vc_queue_cv.notify_all();
	temp_vc_queue.pop();
}

dpp::coroutine <> temp_vc_create_msg(const dpp::snowflake guild_id, const dpp::channel& channel) {
	const std::string description = std::format("A new temporary channel has been created {0}. Join the channel, **{1}** (<#{2}>)!",
		!channel.parent_id.empty() ? std::format("in the <#{}> category", channel.parent_id) : "outside the categories",
		channel.name, channel.id.str()
	);
	const dpp::embed temp_vc_create_embed = dpp::embed()
		.set_color(dpp::colors::greenish_blue)
		.set_description(description);
	log("Sending the temp vc creation notification.");
	bot->message_create(dpp::message((co_await db::get <notification_channel_t>(guild_id, ns_jtc)).channel_id, temp_vc_create_embed), error_callback);
}

dpp::coroutine <> temp_vc_create_owner_msg(const dpp::channel& channel) {
	const dpp::user user = co_await lookup_user(get_oldest_temp_vc_request().user_id);
	log(std::format("`{0}` ({1}) Created a temp VC. Guild ID: {2}, channel ID: {3}, channel name: `{4}`, notification channel ID: {5}",
		user.format_username(), user.id, channel.guild_id, channel.id,
		channel.name, (co_await db::get <notification_channel_t>(channel.guild_id, ns_jtc)).channel_id)
	);
	const auto tp_blocker = co_await db::get <temp_ping_blocker_t>(user.id);
	if (tp_blocker.exists()) {
		log(std::format("{} has temporary pings off.", user.id));
		co_return;
	}
	const dpp::embed temp_ping_embed = dpp::embed()
		.set_color(dpp::colors::sti_blue)
		.set_title(std::format("Welcome to <#{}>!", channel.id))
		.set_author(std::format("This VC belongs to {}.", user.username), user.get_url(), user.get_avatar_url())
		.add_field(
			"You're able to edit the channel!",
			std::format("Use a subcommand of the `/tempvc` command to change the name, limit, or bitrate of your channel to whatever value your soul desires. "
			   " You can also use it to restrict users' access to them. See {} (not to be confused with \"seek help\") for more information.", slash::get_mention("help"))
		)
		.set_footer(
			dpp::embed_footer()
			.set_text("Use the button below to toggle the temporary VC creation ping on/off. Have fun!")
		);
	std::lock_guard L(cfg_values_mutex);
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

dpp::coroutine <> temp_vc_delete_with_msg(const dpp::snowflake channel_id) {
	const dpp::snowflake user_id = (co_await db::get <temp_vc_t>(channel_id)).creator_id;
	const dpp::user user = co_await lookup_user(user_id);
	const dpp::channel channel = co_await lookup_channel(channel_id);
	const dpp::snowflake notification_channel_id = (co_await db::get <notification_channel_t>(channel.guild_id, ns_jtc)).channel_id;
	std::unique_lock L(temp_vc_mutex);
	log(std::format("`{0}` ({1}) left a temp VC. Guild ID: {2}, channel ID: {3}, channel name: `{4}`, notification channel ID: {5}",
		user.format_username(), user.id, channel.guild_id, channel.id,
		channel.name, notification_channel_id)
	);
	L.unlock();
	log(std::format("Does their server ({}) have a temp vc notification channel though?", channel.guild_id));
	if (!notification_channel_id.empty()) {
		log(std::format("Yes, it does, it's {}.", channel.id));
		std::string description = "A temporary channel **" + channel.name + "**";
		log("Is it in a category?");
		const dpp::channel category = co_await lookup_channel(channel.parent_id);
		if (!category.id.empty()) {
			log(std::format("Yes, it is, in the category {}.", category.id));
			description += " in the **" + category.get_mention() + "** category";
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

dpp::coroutine <> temp_vc_create(const dpp::voice_state_update_t& event) {
	const dpp::user user = co_await lookup_user(event.state.user_id);
	dpp::snowflake jtc_channel_id = event.state.channel_id;
	dpp::snowflake guild_id = event.state.guild_id;
	log(std::format("Starting the setup of a temporary channel for {}.", user.id));
	bool to_return = false;
	const uint64_t current_time = bot->uptime().to_secs() + 5;
	std::unique_lock L(temp_vc_mutex);
	if (current_time - join_time[user.id] <= 5) {
		log("This user's last join time was less than 5 seconds ago! Get out!");
		bot->message_create(dpp::message(jtc_channel_id,
			std::format("<@{}> you have to wait longer before joining the voice channel. To create a temporary VC as soon as possible, disconnect and try again in 5 seconds.", user.id)
		).set_allowed_mentions(true), error_callback);
		to_return = true;
	}
	// Sometimes the code above gets triggered for seemingly no reason
	log("DEBUG: Setting the join time");
	join_time[user.id] = current_time;
	bot->start_timer([user](const dpp::timer& h) {
		std::lock_guard L(temp_vc_mutex);
		join_time.erase(user.id);
		bot->stop_timer(h);
	}, 5);
	L.unlock();
	if (to_return) {
		bot->guild_member_move(0, guild_id, user.id, error_callback);
		co_return;
	}
	log("The user has passed the basic checks. Pushing to the queue.");
	const temp_id_t curr_id = create_temp_vc_request(user.id, jtc_channel_id, guild_id);
	bot->queue_work(curr_id, std::bind_front([](const dpp::user user, const dpp::snowflake jtc_channel_id, const dpp::snowflake guild_id, const temp_id_t curr_id) -> dpp::job {
		const std::string& username = user.username;
		std::string new_name;
		dpp::channel new_channel;
		new_channel.set_type(dpp::channel_type::CHANNEL_VOICE);
		const auto jtc_vc = co_await db::get <jtc_vc_t>(jtc_channel_id);
		for (int i = 0; i < cast <int>(jtc_vc.name.size()); i++) {
			if (jtc_vc.name[i] == '{') {
				if (jtc_vc.name.size() - i >= 10) { // text {username}
					std::string temp_string;      // 0123456789	14
					for (int j = i; j < i + 10; j++) {
						temp_string += jtc_vc.name[j];
					}
					if (temp_string == "{username}") {
						new_name += username;
						i += 9;
						continue;
					}
				}
			}
			new_name += jtc_vc.name[i];
		}
		if (new_name.size() > 100) {
			// Usernames come in different shapes and sizes which means
			// they can make the size of the name higher than 100.
			// In that case we're gonna replace each occurrence with
			// Its first 10 letters.
			std::string newer_name;
			for (int i = 0; i < cast <int>(jtc_vc.name.size()); i++) {
				if (jtc_vc.name[i] == '{') {
					if (jtc_vc.name.size() - i >= 10) { // text {username}
						std::string temp_string;	  // 0123456789	14
						for (int j = i; j < i + 10; j++) {
							temp_string += jtc_vc.name[j];
						}
						if (temp_string == "{username}") {
							newer_name += username.substr(0, 10);
							i += 9;
							continue;
						}
					}
				}
				newer_name += jtc_vc.name[i];
			}
			new_name = newer_name;
		}
		const int8_t limit = jtc_vc.limit;
		new_channel.set_name(new_name);
		new_channel.set_guild_id(guild_id);
		new_channel.set_bitrate(jtc_vc.bitrate);
		const dpp::channel current_channel = co_await lookup_channel(jtc_channel_id);
		new_channel.set_parent_id(current_channel.parent_id);
		new_channel.set_user_limit(limit);
		std::mutex curr_query_mutex;
		std::unique_lock curr_query_lock(curr_query_mutex);
		log(std::format("Waiting for {}'s turn to have a temporary VC created for them.", user.id));
		temp_vc_queue_cv.wait(curr_query_lock, [curr_id] {
			return get_oldest_temp_vc_request().id == curr_id;
		});
		curr_query_lock.unlock();
		const dpp::guild guild = co_await lookup_guild(guild_id);
		if (std::unique_lock keep_lock(temp_vc_t::keep_mutex);
		!temp_vc_t::kept_guilds.contains(guild_id)) {
			temp_vc_t::kept_guilds.insert(guild_id);
			keep_lock.unlock();
			const auto deleted_channels = co_await db::keep <temp_vc_t>(guild_id, guild.channels);
			if (!deleted_channels.is_valid()) {
				keep_lock.lock();
				temp_vc_t::kept_guilds.erase(guild_id);
				bot->message_create(dpp::message(jtc_channel_id, std::format("<@{0}> Couldn't read the resource from the database. Please try again later.",
					user.id))
				.set_allowed_mentions(true), error_callback);
				bot->guild_member_move(0, guild_id, user.id, error_callback);
				co_return;
			}
		}
		const int temp_vc_amount = (co_await db::get <temp_vc_t::count_t>(guild_id)).count;
		if (temp_vc_amount >= 50) {
			log(std::format("The guild {0} already has {1} temporary VCs, over the limit.",
				guild_id, temp_vc_amount)
			);
			delete_temp_vc_request();
			bot->message_create(dpp::message(jtc_channel_id, std::format("<@{0}> There are too many temporary VCs in this guild ({1}/50).",
				user.id, temp_vc_amount))
			.set_allowed_mentions(true), error_callback);
			bot->guild_member_move(0, guild_id, user.id, error_callback);
			co_return;
		}
		if (guild.permission_overwrites(co_await lookup_guild_member(guild_id, bot->me.id), current_channel).can(dpp::p_manage_roles)) {
			for (const dpp::permission_overwrite& x : current_channel.permission_overwrites) {
				constexpr dpp::permission useful_perms[] = {
					dpp::p_view_channel,
					dpp::p_connect,
					dpp::p_speak,
				};
				for (const dpp::permission perm : useful_perms) {
					if (x.deny.can(perm) && x.id != bot->me.id) {
						new_channel.add_permission_overwrite(x.id, cast <dpp::overwrite_type>(x.type), 0, perm);
					}
					else if (x.allow.can(dpp::p_view_channel) && x.id != bot->me.id) {
						new_channel.add_permission_overwrite(x.id, cast <dpp::overwrite_type>(x.type), perm, 0);
					}
				}
			}
			new_channel.add_permission_overwrite(bot->me.id, dpp::ot_member, dpp::p_view_channel, 0);
		}
		log(std::format("Creating a temporary VC for {}", get_oldest_temp_vc_request().user_id));
		const dpp::confirmation_callback_t channel_callback = co_await bot->co_channel_create(new_channel);
		log("A callback has arrived!");
		if (error_pingback(channel_callback, current_channel.id, user.id)) {
			delete_temp_vc_request();
			log("It happens to be erroneous.");
			co_return;
		}
		const auto channel = channel_callback.get <dpp::channel>();
		co_await temp_vc_create_owner_msg(channel);
		delete_temp_vc_request();
		const dpp::confirmation_callback_t& move_callback = co_await bot->co_guild_member_move(channel.id, channel.guild_id, user.id);
		if (error_callback(move_callback)) {
			log(std::format("Above is the reason I couldn't move {} into the new VC, deleting it.", user.id));
			bot->channel_delete(channel.id, error_callback);
			co_return;
		}
		const auto temp_vc = co_await db::create <temp_vc_t>({channel.id, channel.guild_id, user.id, jtc_channel_id, 0});
		if (!temp_vc.is_valid()) {
			bot->message_create(dpp::message(jtc_channel_id,
				std::format("<@{}> Warning: failed to write the VC to the database. "
				                          "The bot won't be able to automatically clean up the channel.",
					user.id)
			).set_allowed_mentions(true), error_callback);
		}
		log("Does this guild have a temp vc notification channel?");
		if ((co_await db::get <notification_channel_t>(guild_id, ns_jtc)).exists()) {
			log(std::format("Yes, it does, it's {}.", channel.id));
			co_await temp_vc_create_msg(guild_id, channel);
		}
		else {
			log("It does not.");
		}
		log(std::format("Finished the temporary VC creation process for {}.", user.id));
	}, user, jtc_channel_id, guild_id, curr_id));
}

bool temp_vc_is_accessible(dpp::permission const overwrite) {
	return overwrite.can(dpp::p_view_channel) && overwrite.can(dpp::p_connect);
}

bool temp_vc_is_accessible(const dpp::permission_overwrite& overwrite) {
	return !overwrite.deny.can(dpp::p_view_channel) && !overwrite.deny.can(dpp::p_connect);
}

bool temp_vc_is_speakable(const dpp::permission overwrite) {
	return overwrite.can(dpp::p_speak);
}

bool temp_vc_is_speakable(const dpp::permission_overwrite& overwrite) {
	return !overwrite.deny.can(dpp::p_speak);
}

dpp::coroutine <dpp::role> get_highest_role(const dpp::snowflake user_id, const dpp::snowflake guild_id) {
	dpp::role res;
	const auto member = co_await lookup_guild_member(guild_id, user_id);
	const auto& roles = member.get_roles();
	for (const dpp::snowflake x : roles) {
		res = std::max(res, co_await lookup_role(x, guild_id));
	}
	co_return res;
}

dpp::coroutine <> re_unmute_member(dpp::guild_member member, const dpp::channel& channel) {
	const dpp::guild_member bot_member = co_await lookup_guild_member(member.guild_id, bot->me.id);
	const dpp::permission own_perms = (co_await lookup_guild(member.guild_id)).permission_overwrites(bot_member, channel);
	if (!member.is_muted() && own_perms.can(dpp::p_mute_members, dpp::p_deafen_members)) {
		log("Trying to re-unmute the member...");
		member.set_mute(true);
		const dpp::confirmation_callback_t mute_callback = co_await bot->co_guild_edit_member(member);
		if (error_callback(mute_callback)) {
			log("Failed to mute.");
			co_return;
		}
		member = co_await lookup_guild_member(member.guild_id, member.user_id);
		member.set_mute(false);
		const dpp::confirmation_callback_t unmute_callback = co_await bot->co_guild_edit_member(member);
		if (error_callback(unmute_callback)) {
			log("Failed to unmute.");
			co_return;
		}
		log("Success.");
	}
}

bool restlist_updated(const dpp::channel& channel, const restrictions_types rest_type) {
	switch (rest_type) {
	case RRT_BLOCKLIST:
		return blocklist_updated(channel);
	case RRT_MUTELIST:
		return mutelist_updated(channel);
	default:
		return false;
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
