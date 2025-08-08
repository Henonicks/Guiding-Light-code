#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "notification_channel.h"
#include "jtc_vc.h"
#include "jtc_defaults.h"
#include "logging.h"
#include "slash_funcs.h"
#include "topgg.h"
#include "ticket.h"
#include "config_values.h"
#include "commands.h"

namespace configuration {
	void read_config();
	void init_logs();
	void pray();
	void write_down_slashcommands();
}

#endif
