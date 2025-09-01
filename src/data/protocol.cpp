#include "p2p/data/protocol.h"
#include <cstring>

namespace p2p_core {

static inline void put_u32_be(std::vector<Byte>& out, std::uint32_t v) {
  out.push_back(static_cast<Byte>((v >> 24) & 0xFF));
  out.push_back(static_cast<Byte>((v >> 16) & 0xFF));
  out.push_back(static_cast<Byte>((v >> 8) & 0xFF));
  out.push_back(static_cast<Byte>(v & 0xFF));
}

std::vector<Byte> encode_frame(const Frame& f) {
  std::vector<Byte> out;
  std::uint32_t length = 1u + static_cast<std::uint32_t>(f.payload.size());
  put_u32_be(out, length);
  out.push_back(static_cast<Byte>(f.type));
  out.insert(out.end(), f.payload.begin(), f.payload.end());
  return out;
}

bool try_decode_frame(std::vector<Byte>& buffer, Frame& out) {
  if (buffer.size() < 5) return false;
  std::uint32_t len = (static_cast<std::uint32_t>(buffer[0]) << 24) |
                      (static_cast<std::uint32_t>(buffer[1]) << 16) |
                      (static_cast<std::uint32_t>(buffer[2]) << 8) |
                      (static_cast<std::uint32_t>(buffer[3]));
  if (buffer.size() < 4u + len) return false;
  out.type = static_cast<MsgType>(buffer[4]);
  out.payload.assign(buffer.begin() + 5, buffer.begin() + 4u + len);
  buffer.erase(buffer.begin(), buffer.begin() + 4u + len);
  return true;
}

} // namespace p2p_core


