#pragma once

#include <memory>
#include <string>
#include "transport.h"

namespace p2p_core {

// TLS over TCP using OpenSSL; generates self-signed cert if none provided
std::unique_ptr<ITcpTransport> make_tls_transport(const std::string& certPemPath,
                                                  const std::string& keyPemPath);

} // namespace p2p_core


