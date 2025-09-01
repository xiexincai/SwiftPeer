#pragma once

#include <string>
#include <optional>
#include "p2p/core/types.h"

namespace p2p_core {

// RFC 5389 minimal STUN Binding discovery
class StunClient {
public:
  // server in form host:port, default port 3478 if no port provided
  std::optional<Endpoint> discover_mapped_address(const std::string& server, int timeoutMs = 1500) const;
};

} // namespace p2p_core
