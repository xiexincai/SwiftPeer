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

// 用于流对等节点发现的简化DHT（基于UDP）
class SimpleDhtNode {
public:
  SimpleDhtNode();
  ~SimpleDhtNode();

  bool start(std::uint16_t port /*0->自动*/);
  void stop();
  std::uint16_t bound_port() const { return port_; }

  void add_bootstrap(const Endpoint& ep);

  // 通告我们在(host,port)有streamId
  void announce_stream(const std::string& streamId, const Endpoint& self);

  // 查询streamId的对等节点
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


