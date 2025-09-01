#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "p2p/core/types.h"

namespace p2p_core {

enum class MsgType : std::uint8_t {
  Handshake = 0x01,
  Announce  = 0x02,
  Request   = 0x03,
  Piece     = 0x04,
  KeepAlive = 0x05
};

struct Frame {
  MsgType type;
  std::vector<Byte> payload;
};

// Encode: 4-byte big-endian length (1 + payload), 1-byte type, payload
std::vector<Byte> encode_frame(const Frame& f);
// Returns empty optional if incomplete
bool try_decode_frame(std::vector<Byte>& buffer, Frame& out);

// Piece payload format (text header + binary body):
// If content addressing enabled:
//   hashHex \n [optional: ecdsa_der_sig_hex]\n raw-bytes
// Else:
//   streamId \n pieceIndex \n sha256hex \n [optional: ecdsa_der_sig_hex]\n raw-bytes

} // namespace p2p_core


