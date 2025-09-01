#include "p2p/transport/ice.h"

namespace p2p_core {

TurnClient::TurnClient(const P2PConfig& cfg) : cfg_(cfg) {}

std::optional<TurnAllocation> TurnClient::allocate(int /*timeoutMs*/) {
  // Placeholder: implement RFC 5766 Allocate flow (STUN over UDP/TCP/TLS), long-term creds
  return std::nullopt;
}

bool TurnClient::refresh(const TurnAllocation& /*alloc*/) {
  // Placeholder: send Refresh requests
  return false;
}

} // namespace p2p_core


