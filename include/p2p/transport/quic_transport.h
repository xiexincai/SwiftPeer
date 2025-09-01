#pragma once

#include <memory>
#include <string>
#include "transport.h"

namespace p2p_core {

// QUIC transport providing reliable ordered streams compatible with ITcpTransport semantics.
// Requires an external QUIC stack (e.g., MsQuic, quiche). Factory returns nullptr if not available.
std::unique_ptr<ITcpTransport> make_quic_transport(const std::string& alpn = "p2p/1");

} // namespace p2p_core


