#include "p2p/discovery/dht.h"
#include "p2p/utils/logging.h"

#include <cstring>
#include <algorithm>
#include <thread>
#include <sstream>

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

namespace {
enum class DhtMsg : uint8_t { FIND=1, ANNOUNCE=2, REPLY=3 };
}

SimpleDhtNode::SimpleDhtNode() {}
SimpleDhtNode::~SimpleDhtNode() { stop(); }

bool SimpleDhtNode::start(std::uint16_t port) {
  sock_ = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_ < 0) return false;
  sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port);
  if (::bind(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) return false;
  socklen_t len = sizeof(addr);
  if (::getsockname(sock_, reinterpret_cast<sockaddr*>(&addr), &len) == 0) port_ = ntohs(addr.sin_port);
  running_ = true; ioThread_ = std::thread([this]{ io_loop(); });
  return true;
}

void SimpleDhtNode::stop() {
  if (!running_.exchange(false)) return;
#if !defined(_WIN32)
  ::close(sock_);
#else
  ::closesocket(sock_);
#endif
  if (ioThread_.joinable()) ioThread_.join();
}

void SimpleDhtNode::add_bootstrap(const Endpoint& ep) { bootstraps_.push_back(ep); }

void SimpleDhtNode::announce_stream(const std::string& streamId, const Endpoint& self) {
  localStore_[streamId].push_back(self);
  for (const auto& b : bootstraps_) send_announce(b, streamId, self);
}

std::vector<Endpoint> SimpleDhtNode::get_stream_peers(const std::string& streamId, int /*timeoutMs*/) {
  std::vector<Endpoint> out = localStore_[streamId];
  for (const auto& b : bootstraps_) send_find(b, streamId);
  // naive: return current local view
  return out;
}

void SimpleDhtNode::io_loop() {
  unsigned char buf[1024];
  while (running_) {
    sockaddr_in from{}; socklen_t flen = sizeof(from);
    int n = recvfrom(sock_, reinterpret_cast<char*>(buf), sizeof(buf), 0,
                     reinterpret_cast<sockaddr*>(&from), &flen);
    if (n <= 0) continue;
    std::string s(reinterpret_cast<char*>(buf), reinterpret_cast<char*>(buf)+n);
    if (s.size() < 2) continue;
    DhtMsg type = static_cast<DhtMsg>(buf[0]);
    if (type == DhtMsg::FIND) {
      std::string streamId = s.substr(1);
      auto it = localStore_.find(streamId);
      std::string reply; reply.push_back(static_cast<char>(DhtMsg::REPLY));
      if (it != localStore_.end()) {
        for (const auto& ep : it->second) {
          reply.append(ep.host); reply.push_back(':'); reply.append(std::to_string(ep.port)); reply.push_back('\n');
        }
      }
      sendto(sock_, reply.data(), static_cast<int>(reply.size()), 0, reinterpret_cast<sockaddr*>(&from), sizeof(from));
    } else if (type == DhtMsg::ANNOUNCE) {
      // payload: streamId\n host:port\n
      std::string rest = s.substr(1);
      auto nl = rest.find('\n'); if (nl == std::string::npos) continue;
      std::string streamId = rest.substr(0, nl);
      std::string hp = rest.substr(nl+1);
      auto colon = hp.find(':'); if (colon == std::string::npos) continue;
      Endpoint ep{hp.substr(0, colon), static_cast<std::uint16_t>(std::stoi(hp.substr(colon+1)))};
      localStore_[streamId].push_back(ep);
    } else if (type == DhtMsg::REPLY) {
      // naive: parse and merge
      std::istringstream iss(s.substr(1));
      std::string line; while (std::getline(iss, line)) {
        if (line.empty()) continue; auto c = line.find(':'); if (c==std::string::npos) continue;
        bootstraps_.push_back(Endpoint{line.substr(0,c), static_cast<std::uint16_t>(std::stoi(line.substr(c+1)))});
      }
    }
  }
}

void SimpleDhtNode::send_find(const Endpoint& ep, const std::string& streamId) {
  sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_port=htons(ep.port); addr.sin_addr.s_addr=inet_addr(ep.host.c_str());
  std::string msg; msg.push_back(static_cast<char>(DhtMsg::FIND)); msg.append(streamId);
  sendto(sock_, msg.data(), static_cast<int>(msg.size()), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
}

void SimpleDhtNode::send_announce(const Endpoint& ep, const std::string& streamId, const Endpoint& self) {
  sockaddr_in addr{}; addr.sin_family=AF_INET; addr.sin_port=htons(ep.port); addr.sin_addr.s_addr=inet_addr(ep.host.c_str());
  std::string msg; msg.push_back(static_cast<char>(DhtMsg::ANNOUNCE));
  msg.append(streamId); msg.push_back('\n'); msg.append(self.host); msg.push_back(':'); msg.append(std::to_string(self.port));
  sendto(sock_, msg.data(), static_cast<int>(msg.size()), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
}

} // namespace p2p_core


