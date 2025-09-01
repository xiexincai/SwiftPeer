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
  std::string host; // IP or hostname
  std::uint16_t port{0};
};

struct PieceId {
  // Backward compatibility fields (optional)
  std::string streamId; // logical stream identifier (optional in content addressing)
  std::uint64_t pieceIndex{0};
  // Content addressing: authoritative ID is sha256(data) in hex
  std::string hashHex; // when present, used as key and transmitted over the wire
};

struct PieceData {
  PieceId id;
  Bytes data;
  // integrity via SHA-256: hex optional; if empty, computed on send
  std::string sha256Hex;
};

struct StatsSnapshot {
  std::uint64_t bytesUploaded{0};
  std::uint64_t bytesDownloaded{0};
  std::uint32_t connectedPeers{0};
};

} // namespace p2p_core


