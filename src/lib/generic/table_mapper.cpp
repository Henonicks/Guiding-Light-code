#include "guiding_light/generic/table_mapper.hpp"

#include "guiding_light/resources/jtc_vc.hpp"
#include "guiding_light/resources/noguild_reminder_blocker.hpp"
#include "guiding_light/resources/notification_channel.hpp"
#include "guiding_light/resources/temp_ping_blocker.hpp"
#include "guiding_light/resources/temp_vc.hpp"
#include "guiding_light/resources/ticket.hpp"
#include "guiding_light/resources/topgg_guild_choice.hpp"
#include "guiding_light/resources/topgg_guild_vote_amount.hpp"

template <typename T>
void map_field_list_impl() {
	if constexpr (!std::is_same_v <T, void>) {
		field_lists[T::table_name] = &T::field_list;
	}
}

template <typename ...T>
void map_field_lists_impl() {
	(map_field_list_impl <T>(), ...);
}

void map_field_lists() {
	map_field_lists_impl <
		jtc_vc_t,
		noguild_reminder_blocker_t,
		notification_channel_t,
		temp_ping_blocker_t,
		temp_vc_t,
		ticket_t,
		topgg_guild_choice_t,
		topgg_guild_vote_amount_t,
	void>();
}
