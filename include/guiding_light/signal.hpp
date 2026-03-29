#ifndef SIGNAL_H
#define SIGNAL_H

#include <condition_variable>
#include <mutex>

inline std::atomic <int> last_signal;
inline std::condition_variable_any signal_cv;
inline std::recursive_mutex signal_mutex;

void handle_signal(int code);

#endif
