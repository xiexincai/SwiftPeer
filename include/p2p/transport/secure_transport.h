#pragma once

#include <memory>
#include <string>
#include "transport.h"

namespace p2p_core {

// 使用OpenSSL的TLS over TCP；如果未提供证书则生成自签名证书
std::unique_ptr<ITcpTransport> make_tls_transport(const std::string& certPemPath,
                                                  const std::string& keyPemPath);

} // namespace p2p_core


