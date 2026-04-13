#include "guiding_light/ticket.hpp"

bool operator <(const ticket& t1, const ticket& t2) {
	if (t1.user_id == t2.user_id) {
		if (t1.channel_id == t2.channel_id) {
			return t1.dm_channel_id < t2.dm_channel_id;
		}
		return t1.channel_id < t2.channel_id;
	}
	return t1.user_id < t2.user_id;
}
