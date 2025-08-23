#ifndef NW_DPP_H
#define NW_DPP_H

#if defined __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wall"
	#pragma clang diagnostic ignored "-Wextra"
	#pragma clang diagnostic ignored "-Wpedantic"
	#include <dpp/dpp.h>
	#pragma clang diagnostic pop
#elif defined __GNUC__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wall"
	#pragma GCC diagnostic ignored "-Wextra"
	#pragma GCC diagnostic ignored "-Wpedantic"
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	#include <dpp/dpp.h>
	#pragma GCC diagnostic pop
#endif

#endif
