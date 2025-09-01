#include "p2p/discovery/stun_client.h"
#include "p2p/discovery/dns_resolver.h"
#include "p2p/utils/logging.h"
#include <cstring>
#include <optional>
#include <random>

#if defined(_WIN32)
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <sys/socket.h>
#  include <unistd.h>
#endif

namespace p2p_core {

// Very small STUN Binding Request per RFC 5389
// Returns mapped address if present; we keep it minimal for bootstrap
std::optional<Endpoint> StunClient::discover_mapped_address(const std::string& server, int timeoutMs) const {
  std::string host = server;
  uint16_t port = 3478;
  auto pos = server.find(':');
  if (pos != std::string::npos) {
    host = server.substr(0, pos);
    port = static_cast<uint16_t>(std::stoi(server.substr(pos + 1)));
  }

  DnsResolver resolver;
  auto ip = resolver.resolve(host);
  if (ip.empty()) return std::nullopt;

  int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) return std::nullopt;

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip.c_str());

#if defined(_WIN32)
  DWORD tv = timeoutMs;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&tv), sizeof(tv));
#else
  timeval tv{ timeoutMs / 1000, (timeoutMs % 1000) * 1000 };
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

  unsigned char req[20] = {0};
  // STUN header
  req[0] = 0x00; req[1] = 0x01; // Binding Request
  req[2] = 0x00; req[3] = 0x00; // length
  req[4] = 0x21; req[5] = 0x12; req[6] = 0xA4; req[7] = 0x42; // magic
  std::random_device rd; std::mt19937 gen(rd());
  for (int i = 0; i < 12; ++i) req[8 + i] = static_cast<unsigned char>(gen() & 0xFF);

  if (sendto(sock, reinterpret_cast<const char*>(req), sizeof(req), 0,
             reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
#if !defined(_WIN32)
    close(sock);
#else
    closesocket(sock);
#endif
    return std::nullopt;
  }

  unsigned char resp[512];
  sockaddr_in from{}; socklen_t fromlen = sizeof(from);
  int n = recvfrom(sock, reinterpret_cast<char*>(resp), sizeof(resp), 0,
                   reinterpret_cast<sockaddr*>(&from), &fromlen);
#if !defined(_WIN32)
  close(sock);
#else
  closesocket(sock);
#endif
  if (n < 0) return std::nullopt;

  // Minimal parse: look for XOR-MAPPED-ADDRESS (0x0020)
  if (n < 20) return std::nullopt;
  if (!(resp[0] == 0x01 && resp[1] == 0x01)) return std::nullopt; // Binding Success
  int len = (resp[2] << 8) | resp[3];
  int posAttr = 20;
  while (posAttr + 4 <= 20 + len) {
    uint16_t at = (resp[posAttr] << 8) | resp[posAttr + 1];
    uint16_t alen = (resp[posAttr + 2] << 8) | resp[posAttr + 3];
    posAttr += 4;
    if (at == 0x0020 && alen >= 8) {
      uint8_t family = resp[posAttr + 1];
      uint16_t xport = (resp[posAttr + 2] << 8) | resp[posAttr + 3];
      uint32_t xaddr = (resp[posAttr + 4] << 24) | (resp[posAttr + 5] << 16) |
                       (resp[posAttr + 6] << 8) | (resp[posAttr + 7]);
      uint32_t magic = 0x2112A442;
      uint16_t portOut = static_cast<uint16_t>(xport ^ (magic >> 16));
      uint32_t addrOut = xaddr ^ magic;
      char ipbuf[64];
      std::snprintf(ipbuf, sizeof(ipbuf), "%u.%u.%u.%u",
                    (addrOut >> 24) & 0xFF, (addrOut >> 16) & 0xFF,
                    (addrOut >> 8) & 0xFF, addrOut & 0xFF);
      return Endpoint{ipbuf, portOut};
    }
    posAttr += alen;
    posAttr = (posAttr + 3) & ~3; // 4-byte align
  }
  return std::nullopt;
}

} // namespace p2p_core


