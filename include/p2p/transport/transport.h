#pragma once

#include "p2p/core/export.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>
#include <chrono>
#include <optional>
#include "p2p/core/types.h"
#include "p2p/data/protocol.h"

namespace p2p_core {

class P2P_CORE_API IConnection {
public:
  virtual ~IConnection() = default;
  virtual bool send(const std::vector<Byte>& data) = 0;
  virtual void close() = 0;
  virtual Endpoint remote_endpoint() const = 0;
};

using ConnectionPtr = std::shared_ptr<IConnection>;
using FrameHandler = std::function<void(const ConnectionPtr&, const Frame&)>;
using ConnHandler = std::function<void(const ConnectionPtr&)>;

class P2P_CORE_API ITcpTransport {
public:
  virtual ~ITcpTransport() = default;
  virtual bool listen(std::uint16_t port) = 0;
  virtual std::uint16_t bound_port() = 0;
  virtual void set_handlers(ConnHandler on_accept, FrameHandler on_frame, ConnHandler on_close) = 0;
  virtual ConnectionPtr connect(const Endpoint& ep, int timeoutMs) = 0;
  virtual void stop() = 0;
};

P2P_CORE_EXTERN std::unique_ptr<ITcpTransport> make_tcp_transport();
P2P_CORE_EXTERN std::unique_ptr<ITcpTransport> make_tls_transport(const std::string& cert_path, const std::string& key_path);
P2P_CORE_EXTERN std::unique_ptr<ITcpTransport> make_quic_transport(const std::string& alpn);

} // namespace p2p_core


