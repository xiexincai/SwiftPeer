#pragma once

#include "p2p/core/export.h"
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace p2p_core {

using Byte = std::uint8_t;
using Bytes = std::vector<Byte>;

struct Endpoint {
  std::string host; // IP地址或主机名
  std::uint16_t port{0};
};

struct PieceId {
  // 向后兼容字段（可选）
  std::string streamId; // 逻辑流标识符（在内容寻址中可选）
  std::uint64_t pieceIndex{0};
  // 内容寻址：权威ID是sha256(data)的十六进制表示
  std::string hashHex; // 存在时用作密钥并在网络上传输
};

struct PieceData {
  PieceId id;
  Bytes data;
  // 通过SHA-256保证完整性：十六进制可选；如果为空则在发送时计算
  std::string sha256Hex;
};

struct StatsSnapshot {
  std::uint64_t bytesUploaded{0};
  std::uint64_t bytesDownloaded{0};
  std::uint32_t connectedPeers{0};
};

} // namespace p2p_core


