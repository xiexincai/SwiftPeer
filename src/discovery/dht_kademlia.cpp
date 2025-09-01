#include "p2p/discovery/dht_kademlia.h"

namespace p2p_core {

void KBucket::touch(const DhtNodeInfo& n) {
  for (auto it = list_.begin(); it != list_.end(); ++it) {
    if (it->ep.host == n.ep.host && it->ep.port == n.ep.port) { it->lastSeen = n.lastSeen; list_.splice(list_.begin(), list_, it); return; }
  }
  if (list_.size() >= k_) list_.pop_back();
  list_.push_front(n);
}

std::vector<DhtNodeInfo> KBucket::nodes() const { return std::vector<DhtNodeInfo>(list_.begin(), list_.end()); }

KademliaDht::KademliaDht(NodeId selfId, std::size_t k) : self_(selfId), buckets_(160, KBucket(k)) {}

void KademliaDht::add_bootstrap(const Endpoint& ep) { bootstraps_.push_back(ep); }

void KademliaDht::store(const std::string& /*key*/, const Endpoint& /*value*/, int /*ttlSec*/) {}

std::vector<Endpoint> KademliaDht::find_value(const std::string& /*key*/, int /*timeoutMs*/) { return {}; }

void KademliaDht::refresh() {}

} // namespace p2p_core


