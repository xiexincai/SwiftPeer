#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>
#include <chrono>
#include <thread>
#include <atomic>
#include <unordered_map>
#include "p2p/core/types.h"

namespace p2p_core {

// Simplified DHT for stream peer discovery (UDP based)
class SimpleDhtNode {
public:
  SimpleDhtNode();
  ~SimpleDhtNode();

  bool start(std::uint16_t port /*0->auto*/);
  void stop();
  std::uint16_t bound_port() const { return port_; }

  void add_bootstrap(const Endpoint& ep);

  // Announce that we have streamId at (host,port)
  void announce_stream(const std::string& streamId, const Endpoint& self);

  // Query peers for a streamId
  std::vector<Endpoint> get_stream_peers(const std::string& streamId, int timeoutMs = 1000);

private:
  void io_loop();
  void send_find(const Endpoint& ep, const std::string& streamId);
  void send_announce(const Endpoint& ep, const std::string& streamId, const Endpoint& self);

  int sock_{-1};
  std::uint16_t port_{0};
  std::thread ioThread_;
  std::atomic<bool> running_{false};
  std::vector<Endpoint> bootstraps_;
  std::unordered_map<std::string, std::vector<Endpoint>> localStore_;
};

} // namespace p2p_core


