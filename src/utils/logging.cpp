#include "p2p/utils/logging.h"
#include <mutex>
#include <iostream>

namespace p2p_core {

static std::mutex g_mutex;
static LogCallback g_logger;

void set_global_logger(LogCallback cb) {
  std::lock_guard<std::mutex> lock(g_mutex);
  g_logger = std::move(cb);
}

void log_message(LogLevel lvl, const std::string& msg) {
  std::lock_guard<std::mutex> lock(g_mutex);
  if (g_logger) {
    g_logger(lvl, msg);
  } else {
    std::cerr << "[p2p] " << static_cast<int>(lvl) << ": " << msg << std::endl;
  }
}

} // namespace p2p_core


