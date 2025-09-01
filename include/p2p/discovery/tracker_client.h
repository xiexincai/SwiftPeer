#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "p2p/core/types.h"

namespace p2p_core {

class TrackerClient {
public:
  // trackerUrl example: http://tracker.example.com:8080/announce
  explicit TrackerClient(std::string trackerUrl);

  // Announce self and obtain peer list for a stream
  // JSON response: { "peers": [ {"host":"a","port":1234}, ... ] }
  // Optionally signs the request with HMAC-SHA256 using sharedSecret if provided
  std::vector<Endpoint> announce(const std::string& nodeId,
                                 const std::string& streamId,
                                 std::uint16_t port,
                                 int timeoutMs = 2000,
                                 const std::string& sharedSecret = std::string()) const;

  // Optional ICE candidate exchange endpoints (if tracker supports)
  // POST /candidates { node_id, stream_id, candidates: [ {protocol, ip, port} ] }
  // GET  /candidates?node_id=..&stream_id=..
  bool post_candidates(const std::string& nodeId,
                       const std::string& streamId,
                       const std::string& jsonBody,
                       int timeoutMs = 2000,
                       const std::string& sharedSecret = std::string()) const;
  std::string get_candidates(const std::string& nodeId,
                             const std::string& streamId,
                             int timeoutMs = 2000,
                             const std::string& sharedSecret = std::string()) const;

private:
  std::string trackerUrl_;
};

} // namespace p2p_core


