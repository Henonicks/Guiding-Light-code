#include "guiding_light/temp_vc_handler.hpp"

#include "guiding_light/logging.hpp"
#include "guiding_light/exception.hpp"

void temp_vc_create_msg(const temp_vc_query& q, const dpp::channel& channel) {
	const std::string description = fmt::format("A new temporary channel has been created {0}. Join the channel, **{1}** (<#{2}>)!",
		!channel.parent_id.empty() ? fmt::format("in the <#{}> category", channel.parent_id) : "outside the categories",
		channel.name, channel.id.str());
	const dpp::embed temp_vc_create_embed = dpp::embed()
		.set_color(dpp::colors::greenish_blue)
		.set_description(description);
	log("Sending the temp vc creation notification.");
	bot->message_create(dpp::message(temp_vc_notifications[q.guild_id], temp_vc_create_embed), error_callback);
}

void temp_vc_create_owner_msg(const temp_vc_query& q, const dpp::snowflake& channel_id) {
	const dpp::user& user = *q.usr;
	const dpp::channel& channel = *dpp::find_channel(channel_id);
	log(fmt::format("`{0}` ({1}) Created a temp VC. Guild ID: {2}, channel ID: {3}, channel name: `{4}`, notification channel ID: {5}",
		user.format_username(), user.id, channel.guild_id, channel_id,
		channel.name, temp_vc_notifications[channel.guild_id]));
	if (no_temp_ping[user.id]) {
		log(fmt::format("{} has temporary pings off.", user.id));
		return;
	}
	const dpp::embed temp_ping_embed = dpp::embed()
		.set_color(dpp::colors::sti_blue)
		.set_title(fmt::format("Welcome to <#{}>!", channel_id))
		.set_author(fmt::format("This VC belongs to {}.", q.usr->username), q.usr->get_url(), q.usr->get_avatar_url())
		.add_field(
			"You're able to edit the channel!",
			"Use a subcommand of the `/set` command to change the name, limit, or bitrate of your channel to whatever value your soul desires. See `/help` (not to be confused with \"seek help\") for more information."
		)
		.set_footer(
			dpp::embed_footer()
			.set_text("Use the button bellow to toggle the temporary VC creation ping on/off. Have fun!")
		);
	const dpp::message message = dpp::message(channel_id, q.usr->get_mention()).add_embed(temp_ping_embed).add_component(
		dpp::component().add_component(
			dpp::component()
			.set_type(dpp::cot_button)
			.set_emoji("ping", 1271923808739000431)
			.set_style(dpp::cos_danger)
			.set_id("temp_ping_toggle")
		)
	).set_allowed_mentions(true);
	log("Sending the \"You own this channel!\" message.");
	bot->message_create(message, error_callback);
}

void temp_vc_delete_msg(const dpp::user& user, const dpp::channel* channel) {
	log(fmt::format("`{0}` ({1}) left a temp VC. Guild ID: {2}, channel ID: {3}, channel name: `{4}`, notification channel ID: {5}",
		user.format_username(), user.id, channel->guild_id, channel->id,
		channel->name, temp_vc_notifications[channel->guild_id]));
	const dpp::snowflake& channel_id = temp_vc_notifications[channel->guild_id];
	log(fmt::format("Does their server ({}) have a temp vc notification channel though?", channel->guild_id));
	if (!channel_id.empty()) {
		log(fmt::format("Yes, it does, it's {}.", channel->id));
		std::string description = "A temporary channel **" + channel->name + "**";
		log("Is it in a category?");
		const dpp::channel* category = dpp::find_channel(channel->parent_id);
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
		bot->message_create(dpp::message(channel_id, temp_vc_delete_message), error_callback);
	}
	bot->channel_delete(channel->id, error_callback);
}

void temp_vc_create(const temp_vc_query& q) {
	if (q.usr == nullptr) {
		log(fmt::format("User not found. Channel: {}", q.channel_id));
		if (!q.channel_id.empty()) {
			bot->message_create(dpp::message(q.channel_id, "Couldn't get user info. Can't create a temp VC."), error_callback);
		}
		return;
	}
	bool to_return = false;
	const uint64_t current_time = bot->uptime().to_secs();
	if (current_time - join_time[q.usr->id] <= 5) {
		bot->message_create(dpp::message(q.channel_id, fmt::format("<@{}> you have to wait more before joining the voice channel. To create a temporary VC as soon as possible, disconnect and try again in 5 seconds.", q.usr->id)).set_allowed_mentions(true), error_callback);
		to_return = true;
	}
	join_time[q.usr->id] = current_time;
	bot->start_timer([q](const dpp::timer& h) {
		join_time.erase(q.usr->id);
		bot->stop_timer(h);
	}, 5);
	if (to_return) {
		bot->guild_member_move(0, q.guild_id, q.usr->id, error_callback);
		return;
	}
	const std::string& username = q.usr->username;
	std::string new_name;
	dpp::channel new_channel;
	new_channel.set_type(dpp::channel_type::CHANNEL_VOICE);
	const jtc_defaults& defs = jtc_default_values[q.channel_id];
	for (int i = 0; i < cast <int>(defs.name.size()); i++) {
		if (defs.name[i] == '{') {
			if (defs.name.size() - i >= 10) { // text {username}
				std::string temp_string;	  // 0123456789	14
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
	new_channel.set_guild_id(q.guild_id);
	new_channel.set_bitrate(defs.bitrate);
	dpp::channel current = *dpp::find_channel(q.channel_id);
	new_channel.set_parent_id(current.parent_id);
	new_channel.set_user_limit(limit);
	for (const dpp::permission_overwrite& x : current.permission_overwrites) {
		if (x.deny.can(dpp::p_view_channel)) {
			new_channel.add_permission_overwrite(x.id, cast <dpp::overwrite_type>(x.type), 0, x.deny);
		}
	}
	const dpp::timer_callback_t timer_function = [new_channel, current, q](const bool called_separately) -> void {
		log("Attempting to create a temporary VC.");
		if (temp_vcs_queue.empty()) {
			log("The temporary VC queue is empty.");
			if (called_separately) {
				log("Called separately, returning.");
				return;
			}
			log("Called in the timer, throwing.");
			throw success_exception{};
			// We'll throw this exception as a sign that we've fully completed what we had to successfully.
		}
		if (temp_vcs_queue.front().usr->id != q.usr->id) {
			log(fmt::format("The currently handled user ({0}) is not the last in the queue ({1}).",
				q.usr->id, temp_vcs_queue.front().usr->id));
			return;
		}
		if (handling_user_id == q.usr->id) {
			log(fmt::format("The current user ({}) is already being created a temporary VC for.", q.usr->id));
			return;
		}
		if (temp_vc_amount[q.guild_id] >= 50) {
			log(fmt::format("The guild {0} already has {1} temporary VCs, over the limit.",
				q.guild_id, temp_vc_amount[q.guild_id]));
			temp_vcs_queue.pop();
			bot->message_create(dpp::message(q.channel_id, fmt::format("<@{0}> There are too many temporary VCs in this guild ({1}/50).", q.usr->id, temp_vc_amount[q.guild_id])).set_allowed_mentions(true), error_callback);
			bot->guild_member_move(0, q.guild_id, q.usr->id, error_callback);
			return;
		}
		handling_user_id = q.usr->id;
		log(fmt::format("Creating a temporary VC for {}", handling_user_id));
		bot->channel_create(new_channel, [current, q](const dpp::confirmation_callback_t& callback) -> void {
			log(fmt::format("A callback has arrived! Popping {} from the queue.", q.usr->id));
			temp_vcs_queue.pop();
			handling_user_id = 0;
			++temp_vc_amount[q.guild_id];
			if (error_callback(callback)) {
				return;
			}
			const auto channel = std::get <dpp::channel>(callback.value);
			temp_vc_create_owner_msg(q, channel.id);
			temp_vcs[channel.id] = {channel.id, channel.guild_id, q.usr->id, q.channel_id};
			bot->guild_member_move(channel.id, channel.guild_id, q.usr->id, [channel, q](const dpp::confirmation_callback_t& callback) -> void {
				if (error_callback(callback)) {
					log(fmt::format("Above is the reason I couldn't move {} into the new VC, deleting it.", q.usr->id));
					bot->channel_delete(channel.id, error_callback);
					return;
				}
				db::sql << "INSERT INTO temp_vcs VALUES (?, ?, ?, ?);" << channel.id.str() << channel.guild_id.str() << q.usr->id.str() << q.channel_id.str();
				log("Does this guild have a temp vc notification channel?");
				if (!temp_vc_notifications[q.guild_id].empty()) {
					log(fmt::format("Yes, it does, it's {}.", channel.id));
					temp_vc_create_msg(q, channel);
				}
				else {
					log("It does not.");
				}
			});
		});
		if (called_separately) {
			return;
		}
		throw success_exception{};
		// We'll throw this exception as a sign that we've fully completed what we had to successfully.
	};
	log(fmt::format("Attempting to create a temporary VC for {}, separately.", q.usr->id));
	timer_function(true);
	bot->start_timer([timer_function, q](const dpp::timer& h) -> void {
		try {
			log(fmt::format("Waiting for {}'s turn to have a temporary VC created for them.", q.usr->id));
			timer_function(false);
		}
		catch (const std::exception& e) { // Now make sure to catch the success exception or else it's not going to be so successful.
			if (e.what() == std::string("success")) {
				log(fmt::format("Finished the temporary VC creation process for {}. Stopping the timer.", q.usr->id));
				bot->stop_timer(h);
			}
			else {
				log(fmt::format("Exception at the temp VC timer function: ", e.what()));
			}
		}
	}, 1);
}

bool blocklist_updated(const dpp::channel& channel) {
	auto unbanned = banned[channel.id];
	// TODO: should unbanned be a reference?
	bool flag{};
	for (const auto& x : channel.permission_overwrites) {
		if (banned[channel.id].contains(x.id) && (x.allow.can(dpp::p_view_channel) || !x.deny.can(dpp::p_view_channel))) {
			flag = true;
			banned[channel.id].erase(x.id);
		}
		if (unbanned.contains(x.id)) {
			flag = true;
			unbanned.erase(x.id);
		}
	}
	return flag;
}
