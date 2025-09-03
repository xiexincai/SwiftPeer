#pragma once

namespace p2p_core {

// 确保在需要它的平台上初始化套接字栈（例如Windows）
struct NetStackInit {
  NetStackInit();
  ~NetStackInit();
};

} // namespace p2p_core


