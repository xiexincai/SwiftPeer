#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <chrono>
#include <cstdint>
#include <thread>
#include <atomic>
#include <array>
#include <list>
#include "p2p/core/types.h"

namespace p2p_core {

using NodeId = std::array<std::uint8_t, 20>; // 160位

struct DhtNodeInfo {
  NodeId id;
  Endpoint ep;
  std::chrono::steady_clock::time_point lastSeen;
};

class KBucket {
public:
  explicit KBucket(std::size_t k) : k_(k) {}
  void touch(const DhtNodeInfo& n);
  std::vector<DhtNodeInfo> nodes() const;
private:
  std::size_t k_;
  std::list<DhtNodeInfo> list_;
};

class KademliaDht {
public:
  KademliaDht(NodeId selfId, std::size_t k = 20);
  void add_bootstrap(const Endpoint& ep);
  void store(const std::string& key, const Endpoint& value, int ttlSec);
  std::vector<Endpoint> find_value(const std::string& key, int timeoutMs = 1000);
  void refresh();
private:
  NodeId self_;
  std::vector<KBucket> buckets_;
  std::vector<Endpoint> bootstraps_;
};

} // namespace p2p_core


