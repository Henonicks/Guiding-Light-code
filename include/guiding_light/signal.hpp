#ifndef SIGNAL_H
#define SIGNAL_H

#include <condition_variable>
#include <mutex>

inline std::atomic <int> last_signal;

void handle_signal(int code);

#endif
