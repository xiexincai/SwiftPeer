#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>
#include "p2p/core/types.h"
#include "p2p/core/config.h"

namespace p2p_core {

enum class IceCandidateType { Host, ServerReflexive, Relay };

struct IceCandidate {
  std::string protocol; // "udp"
  std::string ip;
  std::uint16_t port{0};
  IceCandidateType type{IceCandidateType::Host};
};

struct IcePair {
  IceCandidate local;
  IceCandidate remote;
};

// Minimal ICE: gather host and STUN reflexive candidates; perform basic connectivity checks.
class MinimalIce {
public:
  explicit MinimalIce(const P2PConfig& cfg);

  std::vector<IceCandidate> gather_candidates();
  // Returns selected pair if connectivity check succeeds
  std::optional<IcePair> connectivity_check(const std::vector<IceCandidate>& local,
                                            const std::vector<IceCandidate>& remote,
                                            int timeoutMs = 1000);

  // Serialize candidates to JSON string
  static std::string to_json(const std::vector<IceCandidate>& cands);
  static std::vector<IceCandidate> from_json(const std::string& json);

private:
  P2PConfig cfg_;
};

// TURN relay minimal hooks (placeholder for full RFC5766):
struct TurnAllocation {
  std::string relayedIp;
  std::uint16_t relayedPort{0};
  std::string nonce;
  std::string realm;
};

class TurnClient {
public:
  explicit TurnClient(const P2PConfig& cfg);
  // Allocate a relay address; returns allocation info on success
  std::optional<TurnAllocation> allocate(int timeoutMs = 1500);
  bool refresh(const TurnAllocation& alloc);
private:
  P2PConfig cfg_;
};

} // namespace p2p_core


