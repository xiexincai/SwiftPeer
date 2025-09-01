#pragma once

namespace p2p_core {

// Ensure socket stack is initialized on platforms that require it (e.g., Windows)
struct NetStackInit {
  NetStackInit();
  ~NetStackInit();
};

} // namespace p2p_core


