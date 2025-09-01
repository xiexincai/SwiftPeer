#pragma once

#include "p2p/core/export.h"
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include "p2p/core/types.h"

namespace p2p_core {

struct P2PConfig {
  // Identity and networking
  std::string nodeId;                  // stable ID; if empty auto-generate
  std::uint16_t listenPort{0};         // 0 -> auto
  std::vector<Endpoint> seedPeers;     // bootstrap peers

  // Discovery
  std::vector<std::string> trackerUrls; // optional HTTP trackers
  std::vector<std::string> stunServers; // e.g., "stun.l.google.com:19302"
  std::string trackerSharedSecret;      // optional HMAC-SHA256 signer for tracker requests

  // Rate limits (bytes/sec)
  double maxUploadBps{0};   // 0 -> unlimited
  double maxDownloadBps{0}; // 0 -> unlimited
  double maxBurstBytes{65536};

  // Limits
  std::uint32_t maxPeers{50};

  // DNS
  // If true and custom resolver is unset, use system resolver (getaddrinfo)
  bool useSystemDns{true};

  // Security / transport
  bool enableTls{true};                 // TLS over TCP
  bool enableQuic{false};               // QUIC (if supported)
  std::string certPemPath;              // optional; if empty auto-generate self-signed
  std::string keyPemPath;               // optional; if empty auto-generate

  // Content addressing / signing
  bool enableContentAddressing{false};   // if true, pieceId is sha256(data)
  bool enablePublisherSignature{false};  // if true, include ECDSA signature
  std::string publisherPrivKeyPemPath;   // for signing (PEM)
  std::string publisherPubKeyPem;        // for verification (PEM)

  // DHT
  bool enableDht{true};
  std::uint16_t dhtPort{0};             // UDP port for DHT (0 -> auto)
  std::vector<Endpoint> dhtBootstraps;  // initial known DHT nodes

  // ICE/TURN
  bool enableIce{false};
  std::string turnUrl;                  // e.g., turn:turn.example.com:3478?transport=udp
  std::string turnUsername;
  std::string turnPassword;
  int iceCheckTimeoutMs{1500};
  int iceKeepaliveMs{15000};
};

} // namespace p2p_core
