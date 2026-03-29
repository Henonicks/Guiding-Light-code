#ifndef JTC_VC_H
#define JTC_VC_H

#include "guiding_light/guiding_light.hpp"

inline std::unordered_map <channel_snowflake, guild_snowflake> jtc_vcs;
inline std::unordered_map <guild_snowflake, int8_t> jtc_vc_amount;
inline std::recursive_mutex jtc_mutex;

#endif
