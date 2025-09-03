#pragma once

#include "p2p/core/export.h"
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include "p2p/core/types.h"

namespace p2p_core {

struct P2PConfig {
  // 身份和网络配置
  std::string nodeId;                  // 稳定ID；如果为空则自动生成
  std::uint16_t listenPort{0};         // 0 -> 自动选择
  std::vector<Endpoint> seedPeers;     // 引导节点

  // 节点发现
  std::vector<std::string> trackerUrls; // 可选的HTTP跟踪器
  std::vector<std::string> stunServers; // 例如："stun.l.google.com:19302"
  std::string trackerSharedSecret;      // 可选的HMAC-SHA256签名密钥

  // 速率限制（字节/秒）
  double maxUploadBps{0};   // 0 -> 无限制
  double maxDownloadBps{0}; // 0 -> 无限制
  double maxBurstBytes{65536};

  // 限制
  std::uint32_t maxPeers{50};

  // DNS
  // 如果为true且未设置自定义解析器，则使用系统解析器（getaddrinfo）
  bool useSystemDns{true};

  // 安全/传输
  bool enableTls{true};                 // TLS over TCP
  bool enableQuic{false};               // QUIC（如果支持）
  std::string certPemPath;              // 可选；如果为空则自动生成自签名证书
  std::string keyPemPath;               // 可选；如果为空则自动生成

  // 内容寻址/签名
  bool enableContentAddressing{false};   // 如果为true，pieceId为sha256(data)
  bool enablePublisherSignature{false};  // 如果为true，包含ECDSA签名
  std::string publisherPrivKeyPemPath;   // 用于签名（PEM格式）
  std::string publisherPubKeyPem;        // 用于验证（PEM格式）

  // DHT
  bool enableDht{true};
  std::uint16_t dhtPort{0};             // DHT的UDP端口（0 -> 自动选择）
  std::vector<Endpoint> dhtBootstraps;  // 初始已知DHT节点

  // ICE/TURN
  bool enableIce{false};
  std::string turnUrl;                  // 例如：turn:turn.example.com:3478?transport=udp
  std::string turnUsername;
  std::string turnPassword;
  int iceCheckTimeoutMs{1500};
  int iceKeepaliveMs{15000};
};

} // namespace p2p_core
