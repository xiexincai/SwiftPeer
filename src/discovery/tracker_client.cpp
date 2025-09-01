#include "p2p/discovery/tracker_client.h"
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <algorithm>
#include "p2p/security/crypto.h"

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

static bool parse_http_url(const std::string& url, std::string& host, uint16_t& port, std::string& path) {
  // very small parser for http://host[:port]/path
  const std::string prefix = "http://";
  if (url.rfind(prefix, 0) != 0) return false;
  std::string rest = url.substr(prefix.size());
  auto slash = rest.find('/');
  std::string authority = slash == std::string::npos ? rest : rest.substr(0, slash);
  path = slash == std::string::npos ? "/" : rest.substr(slash);
  auto colon = authority.find(':');
  if (colon == std::string::npos) {
    host = authority;
    port = 80;
  } else {
    host = authority.substr(0, colon);
    port = static_cast<uint16_t>(std::stoi(authority.substr(colon + 1)));
  }
  return true;
}

TrackerClient::TrackerClient(std::string trackerUrl) : trackerUrl_(std::move(trackerUrl)) {}

std::vector<Endpoint> TrackerClient::announce(const std::string& nodeId,
                                              const std::string& streamId,
                                              std::uint16_t port,
                                              int timeoutMs,
                                              const std::string& sharedSecret) const {
  std::vector<Endpoint> peers;
  std::string host, path; uint16_t httpPort;
  if (!parse_http_url(trackerUrl_, host, httpPort, path)) return peers;

  addrinfo hints{}; hints.ai_socktype = SOCK_STREAM; hints.ai_family = AF_UNSPEC;
  addrinfo* res = nullptr;
  char portStr[16]; std::snprintf(portStr, sizeof(portStr), "%u", httpPort);
  if (getaddrinfo(host.c_str(), portStr, &hints, &res) != 0) return peers;

  int sock = -1; addrinfo* cur = res;
  for (; cur; cur = cur->ai_next) {
    sock = ::socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
    if (sock < 0) continue;
    if (::connect(sock, cur->ai_addr, static_cast<int>(cur->ai_addrlen)) == 0) break;
#if !defined(_WIN32)
    close(sock);
#else
    closesocket(sock);
#endif
    sock = -1;
  }
  freeaddrinfo(res);
  if (sock < 0) return peers;

  // Build query
  std::ostringstream query;
  query << "node_id=" << nodeId << "&stream_id=" << streamId << "&port=" << port;
  std::string queryStr = query.str();
  std::string signature;
  if (!sharedSecret.empty()) signature = hmac_sha256_hex(sharedSecret, queryStr);

  std::ostringstream req;
  req << "GET " << path << "?" << queryStr << " HTTP/1.1\r\nHost: " << host
      << "\r\nConnection: close\r\n";
  if (!signature.empty()) req << "X-Signature: " << signature << "\r\n";
  req << "\r\n";
  auto s = req.str();
  send(sock, s.c_str(), static_cast<int>(s.size()), 0);

  std::string resp;
  char buf[1024];
  int n;
  while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
    resp.append(buf, buf + n);
  }
#if !defined(_WIN32)
  close(sock);
#else
  closesocket(sock);
#endif

  auto posBody = resp.find("\r\n\r\n");
  if (posBody == std::string::npos) return peers;
  std::string body = resp.substr(posBody + 4);
  // naive JSON parsing for {"peers":[{"host":"x","port":y}, ...]}
  auto arrPos = body.find("["), arrEnd = body.find("]", arrPos);
  if (arrPos == std::string::npos || arrEnd == std::string::npos || arrEnd <= arrPos) return peers;
  std::string arr = body.substr(arrPos + 1, arrEnd - arrPos - 1);
  std::istringstream ais(arr);
  std::string item;
  while (std::getline(ais, item, '}')) {
    auto hpos = item.find("\"host\"");
    auto ppos = item.find("\"port\"");
    if (hpos == std::string::npos || ppos == std::string::npos) continue;
    auto q1 = item.find('"', hpos + 6); q1 = item.find('"', q1 + 1);
    auto q2 = item.find('"', q1 + 1); auto q3 = item.find('"', q2 + 1);
    if (q2 == std::string::npos || q3 == std::string::npos) continue;
    std::string hostStr = item.substr(q2 + 1, q3 - q2 - 1);
    auto colon = item.find(':', ppos);
    if (colon == std::string::npos) continue;
    int portVal = std::stoi(item.substr(colon + 1));
    peers.push_back(Endpoint{hostStr, static_cast<std::uint16_t>(portVal)});
  }
  return peers;
}

bool TrackerClient::post_candidates(const std::string& nodeId,
                                    const std::string& streamId,
                                    const std::string& jsonBody,
                                    int /*timeoutMs*/,
                                    const std::string& sharedSecret) const {
  std::string host, path; uint16_t httpPort;
  if (!parse_http_url(trackerUrl_, host, httpPort, path)) return false;
  // post to path + "/candidates"
  std::string postPath = path + "/candidates";

  addrinfo hints{}; hints.ai_socktype = SOCK_STREAM; hints.ai_family = AF_UNSPEC;
  addrinfo* res = nullptr; char portStr[16]; std::snprintf(portStr, sizeof(portStr), "%u", httpPort);
  if (getaddrinfo(host.c_str(), portStr, &hints, &res) != 0) return false;
  int sock = -1; addrinfo* cur = res;
  for (; cur; cur = cur->ai_next) {
    sock = ::socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
    if (sock < 0) continue;
    if (::connect(sock, cur->ai_addr, static_cast<int>(cur->ai_addrlen)) == 0) break;
#if !defined(_WIN32)
    ::close(sock);
#else
    ::closesocket(sock);
#endif
    sock = -1;
  }
  freeaddrinfo(res);
  if (sock < 0) return false;

  std::string body = jsonBody;
  std::ostringstream req;
  req << "POST " << postPath << " HTTP/1.1\r\nHost: " << host
      << "\r\nContent-Type: application/json\r\nContent-Length: " << body.size() << "\r\n";
  std::string toSign = nodeId + "|" + streamId + "|" + body;
  if (!sharedSecret.empty()) req << "X-Signature: " << hmac_sha256_hex(sharedSecret, toSign) << "\r\n";
  req << "Connection: close\r\n\r\n" << body;
  auto s = req.str(); send(sock, s.c_str(), static_cast<int>(s.size()), 0);
  char buf[256]; while (recv(sock, buf, sizeof(buf), 0) > 0) {}
#if !defined(_WIN32)
  ::close(sock);
#else
  ::closesocket(sock);
#endif
  return true;
}

std::string TrackerClient::get_candidates(const std::string& nodeId,
                                          const std::string& streamId,
                                          int /*timeoutMs*/,
                                          const std::string& sharedSecret) const {
  std::string host, path; uint16_t httpPort;
  if (!parse_http_url(trackerUrl_, host, httpPort, path)) return {};
  std::string getPath = path + "/candidates?node_id=" + nodeId + "&stream_id=" + streamId;
  addrinfo hints{}; hints.ai_socktype = SOCK_STREAM; hints.ai_family = AF_UNSPEC;
  addrinfo* res = nullptr; char portStr[16]; std::snprintf(portStr, sizeof(portStr), "%u", httpPort);
  if (getaddrinfo(host.c_str(), portStr, &hints, &res) != 0) return {};
  int sock = -1; addrinfo* cur = res;
  for (; cur; cur = cur->ai_next) {
    sock = ::socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
    if (sock < 0) continue;
    if (::connect(sock, cur->ai_addr, static_cast<int>(cur->ai_addrlen)) == 0) break;
#if !defined(_WIN32)
    ::close(sock);
#else
    ::closesocket(sock);
#endif
    sock = -1;
  }
  freeaddrinfo(res);
  if (sock < 0) return {};
  std::ostringstream req;
  req << "GET " << getPath << " HTTP/1.1\r\nHost: " << host << "\r\nConnection: close\r\n";
  if (!sharedSecret.empty()) {
    std::string toSign = nodeId + "|" + streamId;
    req << "X-Signature: " << hmac_sha256_hex(sharedSecret, toSign) << "\r\n";
  }
  req << "\r\n";
  auto s = req.str(); send(sock, s.c_str(), static_cast<int>(s.size()), 0);
  std::string resp; char buf[1024]; int n; while ((n=recv(sock, buf, sizeof(buf), 0))>0) resp.append(buf, buf+n);
#if !defined(_WIN32)
  ::close(sock);
#else
  ::closesocket(sock);
#endif
  auto pos = resp.find("\r\n\r\n");
  if (pos == std::string::npos) return {};
  return resp.substr(pos+4);
}

} // namespace p2p_core


