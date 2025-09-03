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

// 最小ICE：收集主机和STUN反射候选；执行基本连通性检查。
class MinimalIce {
public:
  explicit MinimalIce(const P2PConfig& cfg);

  std::vector<IceCandidate> gather_candidates();
  // 如果连通性检查成功则返回选定的对
  std::optional<IcePair> connectivity_check(const std::vector<IceCandidate>& local,
                                            const std::vector<IceCandidate>& remote,
                                            int timeoutMs = 1000);

  // 将候选序列化为JSON字符串
  static std::string to_json(const std::vector<IceCandidate>& cands);
  static std::vector<IceCandidate> from_json(const std::string& json);

private:
  P2PConfig cfg_;
};

// TURN中继最小钩子（完整RFC5766的占位符）：
struct TurnAllocation {
  std::string relayedIp;
  std::uint16_t relayedPort{0};
  std::string nonce;
  std::string realm;
};

class TurnClient {
public:
  explicit TurnClient(const P2PConfig& cfg);
  // 分配中继地址；成功时返回分配信息
  std::optional<TurnAllocation> allocate(int timeoutMs = 1500);
  bool refresh(const TurnAllocation& alloc);
private:
  P2PConfig cfg_;
};

} // namespace p2p_core


