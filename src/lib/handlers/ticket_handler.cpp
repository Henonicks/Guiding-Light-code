#include "guiding_light/handlers/ticket_handler.hpp"

#include "guiding_light/handlers/slash_funcs.hpp"
#include "guiding_light/internal/commands.hpp"
#include "guiding_light/database/database.hpp"
#include "guiding_light/helpers/format.tpp"

#include <dpp/unicode_emoji.h>

dpp::message preserve_attachments(const dpp::message& msg) {
	auto repost = dpp::message(msg.content);
	for (const dpp::attachment& x : msg.attachments) {
		repost.content += '\n' + x.url;
	}
	return repost;
}

dpp::coroutine <> handle_dm_in(const dpp::message_create_t& event) {
	const dpp::snowflake user_id = event.msg.author.id;
	log(std::format("Received a DM from the user {}.", user_id));
	const auto ticket = (co_await db::get <ticket_t, "", ticket_t::where_user>(user_id));
	if (!ticket.is_valid()) {
		bot->direct_message_create(user_id, dpp::message(
			"Failed to check the database for the existence of a ticket. Please try again later."
		), error_callback);
		co_return;
	}
	if (!ticket.exists()) {
		log("But they don't have a ticket.");
		bot->direct_message_create(user_id, dpp::message(
			std::format("To contact the creator of this bot you need to create a ticket. Issue {0} to allow your messages in this "
			"chat to be sent to the creator, and the creator's messages to be sent in this chat. You can always close the ticket by issuing {1}.",
			slash::get_mention("ticket create"), slash::get_mention("ticket close"))
		), error_callback);
		co_return;
	}
	dpp::message msg = event.msg;
	msg.content = std::format("From {}: ", event.msg.author.get_mention()) + msg.content;
	bot->message_create(preserve_attachments(msg).set_channel_id(ticket.channel_id), [event](const dpp::confirmation_callback_t& callback) -> void {
		if (error_feedback(callback, event, "Failed to send")) {
			bot->message_add_reaction(event.msg, dpp::unicode_emoji::x, error_callback);
			return;
		}
		bot->message_add_reaction(event.msg, dpp::unicode_emoji::white_check_mark, error_callback);
	});
}

dpp::coroutine <> handle_dm_out(const dpp::message_create_t& event) {
	dpp::message msg = event.msg;
	const dpp::snowflake channel_id = msg.channel_id;
	const dpp::snowflake user_id = (co_await db::get <ticket_t, "", ticket_t::where_channel>(channel_id)).user_id;
	log(std::format("Sending a DM to the user {}.", user_id));
	if (user_id.empty()) {
		error_log("But their ticket is closed. Are you going to delete the channel at some point?");
		bot->message_add_reaction(event.msg, dpp::unicode_emoji::x, error_callback);
		co_return;
	}
	msg.content = "From henonicks: " + msg.content;
	bot->direct_message_create(user_id, preserve_attachments(msg), [event](const dpp::confirmation_callback_t& callback) -> void {
		if (error_feedback(callback, event, "Failed to send")) {
			return;
		}
		bot->message_add_reaction(event.msg, dpp::unicode_emoji::white_check_mark, error_callback);
	});
}
