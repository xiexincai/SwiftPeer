#pragma once

#include <functional>
#include <string>

namespace p2p_core {

enum class LogLevel {
  Trace = 0,
  Debug,
  Info,
  Warn,
  Error
};

using LogCallback = std::function<void(LogLevel level, const std::string& message)>;

void set_global_logger(LogCallback cb);
void log_message(LogLevel lvl, const std::string& msg);

} // namespace p2p_core


