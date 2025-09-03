#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "p2p/core/types.h"

namespace p2p_core {

class TrackerClient {
public:
  // trackerUrl示例：http://tracker.example.com:8080/announce
  explicit TrackerClient(std::string trackerUrl);

  // 通告自身并获取流的对等节点列表
  // JSON响应：{ "peers": [ {"host":"a","port":1234}, ... ] }
  // 如果提供了sharedSecret，可选择使用HMAC-SHA256签名请求
  std::vector<Endpoint> announce(const std::string& nodeId,
                                 const std::string& streamId,
                                 std::uint16_t port,
                                 int timeoutMs = 2000,
                                 const std::string& sharedSecret = std::string()) const;

  // 可选的ICE候选交换端点（如果跟踪器支持）
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


