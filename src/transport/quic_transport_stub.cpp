#include "p2p/transport/quic_transport.h"

namespace p2p_core {

std::unique_ptr<ITcpTransport> make_quic_transport(const std::string& /*alpn*/) {
  // Stub: integrate MsQuic/quiche and return a transport mapping QUIC streams to Frame IO
  return nullptr;
}

} // namespace p2p_core


