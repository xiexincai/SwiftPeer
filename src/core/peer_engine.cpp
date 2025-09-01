#include "p2p/core/peer_engine.h"
#include "p2p/data/protocol.h"
#include "p2p/utils/logging.h"
#include "p2p/data/rate_limiter.h"
#include "p2p/data/segment_store.h"
#include "p2p/discovery/dns_resolver.h"
#include "p2p/transport/transport.h"
#include "p2p/security/crypto.h"

#include <sstream>
#include <random>

namespace p2p_core {

static std::string random_id() {
  static const char* hex = "0123456789abcdef";
  std::random_device rd; std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, 15);
  std::string s(20, '0');
  for (auto& c : s) c = hex[dist(gen)];
  return s;
}

PeerEngine::PeerEngine(P2PConfig config)
  : config_(std::move(config)),
    tcp_(make_tcp_transport()),
    upLimiter_(std::make_unique<TokenBucketLimiter>(config_.maxUploadBps, config_.maxBurstBytes)),
    downLimiter_(std::make_unique<TokenBucketLimiter>(config_.maxDownloadBps, config_.maxBurstBytes)),
    store_(std::make_unique<InMemorySegmentStore>()),
    tracker_(std::make_unique<TrackerClient>("")),
    resolver_(std::make_unique<DnsResolver>()) {
  if (config_.nodeId.empty()) config_.nodeId = random_id();
}

PeerEngine::~PeerEngine() { stop(); }

bool PeerEngine::start() {
  if (running_.exchange(true)) return true;
  // choose transport
  if (config_.enableQuic) {
    extern std::unique_ptr<ITcpTransport> make_quic_transport(const std::string& alpn);
    tlsTcp_ = make_quic_transport("p2p/1");
  }
  if (!tlsTcp_ && config_.enableTls) {
    tlsTcp_ = make_tls_transport(config_.certPemPath, config_.keyPemPath);
  }
  ITcpTransport* tp = tlsTcp_ ? tlsTcp_.get() : tcp_.get();
  tp->set_handlers(
    [this](const ConnectionPtr& c){ on_connection(c); },
    [this](const ConnectionPtr& c, const Frame& f){ on_frame(c, f); },
    [this](const ConnectionPtr& c){ on_close(c); }
  );
  if (!tp->listen(config_.listenPort)) {
    running_ = false;
    return false;
  }
  config_.listenPort = tp->bound_port();
  if (config_.enableDht) {
    dht_.start(config_.dhtPort);
    if (!config_.seedPeers.empty()) {
      for (const auto& ep : config_.seedPeers) dht_.add_bootstrap(ep);
    }
  }
  ioThread_ = std::thread([this]{ io_thread_main(); });
  return true;
}

void PeerEngine::stop() {
  if (!running_.exchange(false)) return;
  if (tlsTcp_) tlsTcp_->stop();
  if (tcp_) tcp_->stop();
  dht_.stop();
  if (ioThread_.joinable()) ioThread_.join();
}

void PeerEngine::publish_piece(const PieceData& piece) {
  if (store_) {
    store_->put(piece);
  }
}

void PeerEngine::request_piece(const PieceId& id) {
  std::string key = make_key(id);
  std::lock_guard<std::mutex> lock(mutex_);
  pendingRequests_.insert(key);
  for (auto& kv : connectionsByKey_) {
    send_request(kv.second, id);
  }
}

void PeerEngine::add_seed_peer(const Endpoint& ep) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::ostringstream oss; oss << ep.host << ":" << ep.port;
  if (connectionsByKey_.find(oss.str()) == connectionsByKey_.end()) {
    ITcpTransport* tp = tlsTcp_ ? tlsTcp_.get() : tcp_.get();
    auto c = tp->connect(ep, 1000);
    if (c) {
      connectionsByKey_[oss.str()] = c;
      send_announce(c);
    }
  }
}

StatsSnapshot PeerEngine::stats() const {
  StatsSnapshot s; s.bytesDownloaded = bytesDown_.load(); s.bytesUploaded = bytesUp_.load();
  std::lock_guard<std::mutex> lock(mutex_);
  s.connectedPeers = static_cast<std::uint32_t>(connectionsByKey_.size());
  return s;
}

void PeerEngine::set_piece_received_callback(PieceReceivedCallback cb) { onPiece_ = std::move(cb); }
void PeerEngine::set_custom_resolver(DnsResolveCallback cb) { 
  if (resolver_) {
    resolver_->set_custom_resolver(std::move(cb)); 
  }
}

void PeerEngine::io_thread_main() {
  connect_seed_peers();
  announce_to_trackers();
  // main loop placeholder for timers
  while (running_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}

void PeerEngine::connect_seed_peers() {
  for (const auto& ep : config_.seedPeers) {
    add_seed_peer(ep);
  }
}

void PeerEngine::announce_to_trackers() {
  for (const auto& url : config_.trackerUrls) {
    TrackerClient client(url);
    auto peers = client.announce(config_.nodeId, "default", config_.listenPort, 2000, config_.trackerSharedSecret);
    for (const auto& ep : peers) add_seed_peer(ep);
  }
  if (config_.enableDht) {
    dht_.announce_stream("default", Endpoint{"0.0.0.0", config_.listenPort});
    auto peers = dht_.get_stream_peers("default");
    for (const auto& ep : peers) add_seed_peer(ep);
  }
}

void PeerEngine::on_connection(const ConnectionPtr& c) {
  send_announce(c);
}

void PeerEngine::on_frame(const ConnectionPtr& c, const Frame& f) {
  switch (f.type) {
    case MsgType::Announce: {
      // peer sends list of available pieces (streamId,pieceIndex list)
      // format: streamId\ncount\nindex... (decimal) simple text
      std::string s(reinterpret_cast<const char*>(f.payload.data()), f.payload.size());
      // For simplicity, ignore; could be used to request missing pieces
      (void)s;
      break;
    }
    case MsgType::Request: {
      // payload: either hashHex or streamId\nindex
      std::string s(reinterpret_cast<const char*>(f.payload.data()), f.payload.size());
      auto pos = s.find('\n');
      PieceId id;
      if (pos == std::string::npos) {
        id.hashHex = s; // content addressing request
      } else {
        id.streamId = s.substr(0, pos);
        id.pieceIndex = static_cast<std::uint64_t>(std::stoull(s.substr(pos + 1)));
      }
      PieceData piece;
      auto piece_opt = store_->get(id);
      if (piece_opt) {
        piece = *piece_opt;
        send_piece(c, piece);
      }
      break;
    }
    case MsgType::Piece: {
      // payload:
      // - content addressing: hash\n[optional sig]\nraw
      // - legacy: streamId\nindex\nsha256\n[optional sig]\nraw
      size_t first = 0; size_t pos = 0; int stage = 0;
      std::string streamId;
      std::uint64_t index = 0;
      std::string hex;
      std::string sigHex;
      for (size_t i = 0; i < f.payload.size(); ++i) {
        if (f.payload[i] == '\n') {
          if (stage == 0) {
            // first line: may be streamId or hashHex
            std::string line(reinterpret_cast<const char*>(f.payload.data() + first), i - first);
            if (line.size() == 64) { // likely sha256 hex
              hex = line; stage = 3; first = i + 1; // next could be optional sig
            } else {
              streamId = line; stage = 1; first = i + 1;
            }
          } else if (stage == 1) {
            std::string idxStr(reinterpret_cast<const char*>(f.payload.data() + first), i - first);
            index = static_cast<std::uint64_t>(std::stoull(idxStr));
            stage = 2; first = i + 1;
          } else if (stage == 2) {
            hex.assign(reinterpret_cast<const char*>(f.payload.data() + first), i - first);
            first = i + 1; stage = 3;
          } else if (stage == 3) {
            // optional sig
            std::string maybeSig(reinterpret_cast<const char*>(f.payload.data() + first), i - first);
            if (!maybeSig.empty() && maybeSig.size() > 10) { sigHex = maybeSig; first = i + 1; }
            pos = first = i + 1; stage = 4; break;
          }
        }
      }
      if (stage >= 3) {
        PieceData pd; pd.id = PieceId{streamId, index};
        if (!hex.empty() && streamId.empty() && index == 0) pd.id.hashHex = hex;
        pd.data.assign(f.payload.begin() + static_cast<long>(pos), f.payload.end());
        // verify sha256
        auto hx = sha256_hex(pd.data);
        if (hx != hex) { log_message(LogLevel::Warn, "SHA256 mismatch, dropping piece"); break; }
        // optional signature verify
        if (config_.enablePublisherSignature && !config_.publisherPubKeyPem.empty() && !sigHex.empty()) {
          auto hash = sha256_bytes(pd.data);
          if (!ecdsa_p256_verify_der_hex(config_.publisherPubKeyPem, hash, sigHex)) {
            log_message(LogLevel::Warn, "ECDSA verify failed, dropping piece");
            break;
          }
        }
        if (store_) {
          store_->put(pd);
        }
        bytesDown_ += pd.data.size();
        if (onPiece_) onPiece_(pd);
      }
      break;
    }
    case MsgType::KeepAlive:
    case MsgType::Handshake:
    default:
      break;
  }
}

void PeerEngine::on_close(const ConnectionPtr& c) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::ostringstream oss; oss << c->remote_endpoint().host << ":" << c->remote_endpoint().port;
  connectionsByKey_.erase(oss.str());
}

void PeerEngine::send_announce(const ConnectionPtr& c) {
  if (!store_) return;
  auto avail = store_->available_piece_indices("default");
  std::ostringstream oss; oss << "default\n" << avail.size();
  for (auto idx : avail) oss << "\n" << idx;
  Frame f{MsgType::Announce, {}};
  auto s = oss.str();
  f.payload.assign(s.begin(), s.end());
  auto bytes = encode_frame(f);
  if (upLimiter_ && upLimiter_->consume(bytes.size()) > 0) {
    if (c->send(bytes)) bytesUp_ += bytes.size();
  }
}

void PeerEngine::send_request(const ConnectionPtr& c, const PieceId& id) {
  std::ostringstream oss;
  if (config_.enableContentAddressing && !id.hashHex.empty()) {
    oss << id.hashHex;
  } else {
    oss << id.streamId << "\n" << id.pieceIndex;
  }
  Frame f{MsgType::Request, {}};
  auto s = oss.str(); f.payload.assign(s.begin(), s.end());
  auto bytes = encode_frame(f);
  if (upLimiter_ && upLimiter_->consume(bytes.size()) > 0) {
    if (c->send(bytes)) bytesUp_ += bytes.size();
  }
}

void PeerEngine::send_piece(const ConnectionPtr& c, const PieceData& piece) {
  auto hx = sha256_hex(piece.data);
  std::string sigHex;
  if (config_.enablePublisherSignature && !config_.publisherPrivKeyPemPath.empty()) {
    auto hash = sha256_bytes(piece.data);
    sigHex = ecdsa_p256_sign_der_hex(config_.publisherPrivKeyPemPath, hash);
  }
  std::ostringstream oss;
  if (config_.enableContentAddressing) {
    oss << hx << "\n";
  } else {
    oss << piece.id.streamId << "\n" << piece.id.pieceIndex << "\n" << hx << "\n";
  }
  if (!sigHex.empty()) oss << sigHex << "\n";
  std::vector<Byte> payload;
  auto header = oss.str();
  payload.insert(payload.end(), header.begin(), header.end());
  payload.insert(payload.end(), piece.data.begin(), piece.data.end());
  Frame f{MsgType::Piece, std::move(payload)};
  auto bytes = encode_frame(f);
  if (upLimiter_ && upLimiter_->consume(bytes.size()) > 0) {
    if (c->send(bytes)) bytesUp_ += bytes.size();
  }
}

std::string PeerEngine::make_key(const PieceId& id) const {
  std::ostringstream oss; oss << id.streamId << ":" << id.pieceIndex; return oss.str();
}

} // namespace p2p_core


