#pragma once

#include <memory>
#include <string>
#include "transport.h"

namespace p2p_core {

// QUIC传输提供可靠的有序流，与ITcpTransport语义兼容。
// 需要外部QUIC栈（例如MsQuic、quiche）。如果不可用则工厂返回nullptr。
std::unique_ptr<ITcpTransport> make_quic_transport(const std::string& alpn = "p2p/1");

} // namespace p2p_core


