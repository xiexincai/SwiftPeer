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

// 编码：4字节大端长度（1 + 负载），1字节类型，负载
std::vector<Byte> encode_frame(const Frame& f);
// 如果不完整则返回空可选
bool try_decode_frame(std::vector<Byte>& buffer, Frame& out);

// 数据块负载格式（文本头部 + 二进制主体）：
// 如果启用内容寻址：
//   hashHex \n [可选：ecdsa_der_sig_hex]\n raw-bytes
// 否则：
//   streamId \n pieceIndex \n sha256hex \n [可选：ecdsa_der_sig_hex]\n raw-bytes

} // namespace p2p_core


