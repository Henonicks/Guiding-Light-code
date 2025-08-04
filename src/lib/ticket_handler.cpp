#include "ticket_handler.h"

dpp::message preserve_attachments(const dpp::message& msg) {
	dpp::message repost = dpp::message(msg.content);
	const auto& attachments = msg.attachments;
	for (const dpp::attachment& x : attachments) {
		repost.content += '\n' + x.url;
	}
	return repost;
}

void handle_dm_in(const dpp::snowflake& user_id, const dpp::message_create_t& event) {
	dpp::cluster* bot = event.from()->creator;
	if (tickets[user_id].empty()) {
		bot->direct_message_create(user_id, dpp::message(fmt::format("To contact the creator of this bot you need to create a ticket. Issue {0} to allow your messages in this chat to be sent to the creator, and the creator's messages to be sent in this chat. You can always close the ticket by issuing {1}.", fmt::format("</ticket open:{}>", slash::created_slashcommands["ticket"].id), fmt::format("</ticket close:{}>", slash::created_slashcommands["ticket"].id))));
		return;
	}
	dpp::message msg = event.msg;
	msg.content = fmt::format("From {}: ", event.msg.author.get_mention()) + msg.content;
	bot->message_create(preserve_attachments(msg).set_channel_id(tickets[user_id]), [bot, event](const dpp::confirmation_callback_t& callback) -> void {
		if (callback.is_error()) {
			event.reply(fmt::format("Failed to send: `{}`.", callback.get_error().message));
			return;
		}
		bot->message_add_reaction(event.msg, dpp::unicode_emoji::white_check_mark);
	});
}

void handle_dm_out(const dpp::message_create_t& event) {
	dpp::message msg = event.msg;
	const dpp::snowflake& channel_id = msg.channel_id;
	dpp::cluster* bot = event.from()->creator;
	if (!slash::enabled) {
		event.reply("the bluetooth device is ready to pair");
		return;
	}
	if (ck_tickets[channel_id].empty()) {
		bot->message_add_reaction(event.msg, dpp::unicode_emoji::x);
		return;
	}
	msg.content = "From henonicks: " + msg.content;
	bot->direct_message_create(ck_tickets[channel_id], preserve_attachments(msg), [bot, event](const dpp::confirmation_callback_t& callback) -> void {
		if (callback.is_error()) {
			event.reply(fmt::format("Failed to send: `{}`.", callback.get_error().message));
			return;
		}
		bot->message_add_reaction(event.msg, dpp::unicode_emoji::white_check_mark);
	});
}
