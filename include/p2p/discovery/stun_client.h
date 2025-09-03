#pragma once

#include <string>
#include <optional>
#include "p2p/core/types.h"

namespace p2p_core {

// RFC 5389最小STUN绑定发现
class StunClient {
public:
  // 服务器格式为host:port，如果未提供端口则默认为3478
  std::optional<Endpoint> discover_mapped_address(const std::string& server, int timeoutMs = 1500) const;
};

} // namespace p2p_core
