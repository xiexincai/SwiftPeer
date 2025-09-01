#include "p2p/core/net_init.h"

#if defined(_WIN32)
#  include <winsock2.h>
#endif

namespace p2p_core {

NetStackInit::NetStackInit() {
#if defined(_WIN32)
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

NetStackInit::~NetStackInit() {
#if defined(_WIN32)
  WSACleanup();
#endif
}

} // namespace p2p_core


