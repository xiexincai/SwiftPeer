#include "p2p/transport/secure_transport.h"
#include "p2p/transport/transport.h"
#include "p2p/utils/logging.h"
#include "p2p/core/net_init.h"

// 暂时禁用OpenSSL支持
// #include <openssl/ssl.h>
// #include <openssl/err.h>

namespace p2p_core {

// 暂时禁用OpenSSL实现
std::unique_ptr<ITcpTransport> make_tls_transport(const std::string& cert_path, const std::string& key_path) {
    // TODO: 实现TLS传输
    return nullptr;
}

} // namespace p2p_core


