#include "p2p/transport/transport.h"
#include "p2p/data/protocol.h"
#include "p2p/utils/logging.h"
#include "p2p/core/net_init.h"

#include <vector>
#include <thread>
#include <mutex>
#include <unordered_set>

#if defined(_WIN32)
#  include <winsock2.h>
#  include <ws2tcpip.h>
using socklen_t = int;
#else
#  include <arpa/inet.h>
#  include <fcntl.h>
#  include <netdb.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

namespace p2p_core {

class TcpConnection : public IConnection, public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(int s, Endpoint remote, FrameHandler onFrame, ConnHandler onClose)
    : sock_(s), remote_(std::move(remote)), onFrame_(std::move(onFrame)), onClose_(std::move(onClose)) {}
  ~TcpConnection() override { close(); }

  void start() {
    if (!started_) {
      started_ = true;
      reader_ = std::thread([self = shared_from_this()] { self->reader_loop(); });
    }
  }

  bool send(const std::vector<Byte>& data) override {
    std::lock_guard<std::mutex> lock(sendMutex_);
    if (sock_ < 0) return false;
    const char* p = reinterpret_cast<const char*>(data.data());
    size_t left = data.size();
    while (left > 0) {
      int n = ::send(sock_, p, static_cast<int>(left), 0);
      if (n <= 0) return false;
      p += n; left -= static_cast<size_t>(n);
    }
    return true;
  }

  void close() override {
    int s = -1;
    {
      std::lock_guard<std::mutex> lock(sendMutex_);
      s = sock_;
      sock_ = -1;
    }
    if (s >= 0) {
#if !defined(_WIN32)
      ::shutdown(s, SHUT_RDWR);
      ::close(s);
#else
      ::shutdown(s, SD_BOTH);
      ::closesocket(s);
#endif
    }
    if (reader_.joinable()) reader_.join();
  }

  Endpoint remote_endpoint() const override { return remote_; }

private:
  void reader_loop() {
    std::vector<Byte> buffer;
    buffer.reserve(4096);
    Frame f;
    while (true) {
      char tmp[2048];
      int n = ::recv(sock_, tmp, sizeof(tmp), 0);
      if (n <= 0) break;
      buffer.insert(buffer.end(), tmp, tmp + n);
      while (try_decode_frame(buffer, f)) {
        onFrame_(shared_from_this(), f);
      }
    }
    onClose_(shared_from_this());
  }

  int sock_;
  Endpoint remote_;
  FrameHandler onFrame_;
  ConnHandler onClose_;
  std::thread reader_;
  std::mutex sendMutex_;
  bool started_{false};
};

class TcpTransport : public ITcpTransport {
public:
  TcpTransport() : net_() {}
  ~TcpTransport() override { stop(); }

  bool listen(std::uint16_t port) override {
    listener_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listener_ < 0) return false;
    int yes = 1;
    setsockopt(listener_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes));
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(port);
    if (::bind(listener_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) return false;
    if (::listen(listener_, 128) != 0) return false;

    socklen_t len = sizeof(addr);
    if (::getsockname(listener_, reinterpret_cast<sockaddr*>(&addr), &len) == 0) boundPort_ = ntohs(addr.sin_port);
    acceptThread_ = std::thread([this]{ accept_loop(); });
    return true;
  }

  std::uint16_t bound_port() override { return boundPort_; }

  void set_handlers(ConnHandler on_accept, FrameHandler on_frame, ConnHandler on_close) override {
    onAccept_ = std::move(on_accept);
    onFrame_ = std::move(on_frame);
    onClose_ = std::move(on_close);
  }

  ConnectionPtr connect(const Endpoint& ep, int /*timeoutMs*/) override {
    addrinfo hints{}; hints.ai_socktype = SOCK_STREAM; hints.ai_family = AF_UNSPEC;
    addrinfo* res = nullptr;
    char portStr[16]; std::snprintf(portStr, sizeof(portStr), "%u", ep.port);
    if (getaddrinfo(ep.host.c_str(), portStr, &hints, &res) != 0) return nullptr;
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
    if (sock < 0) return nullptr;
    auto conn = std::make_shared<TcpConnection>(sock, ep, onFrame_, onClose_);
    if (onAccept_) onAccept_(conn);
    conn->start();
    return conn;
  }

  void stop() override {
    int s = listener_;
    listener_ = -1;
    if (s >= 0) {
#if !defined(_WIN32)
      ::shutdown(s, SHUT_RDWR);
      ::close(s);
#else
      ::shutdown(s, SD_BOTH);
      ::closesocket(s);
#endif
    }
    if (acceptThread_.joinable()) acceptThread_.join();
  }

private:
  void accept_loop() {
    while (listener_ >= 0) {
      sockaddr_in addr{}; socklen_t len = sizeof(addr);
      int s = ::accept(listener_, reinterpret_cast<sockaddr*>(&addr), &len);
      if (s < 0) break;
      char buf[64];
      const char* ip = inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf));
      Endpoint ep{ ip ? std::string(ip) : std::string("0.0.0.0"), ntohs(addr.sin_port) };
      auto conn = std::make_shared<TcpConnection>(s, ep, onFrame_, onClose_);
      if (onAccept_) onAccept_(conn);
      conn->start();
    }
  }

  int listener_{-1};
  std::uint16_t boundPort_{0};
  std::thread acceptThread_;
  ConnHandler onAccept_;
  FrameHandler onFrame_;
  ConnHandler onClose_;
  NetStackInit net_;
};

std::unique_ptr<ITcpTransport> make_tcp_transport() {
  return std::make_unique<TcpTransport>();
}

// These functions are already defined in other files, removing duplicate definitions here

} // namespace p2p_core


