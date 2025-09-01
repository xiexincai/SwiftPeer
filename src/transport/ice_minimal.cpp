#include "p2p/transport/ice.h"
#include "p2p/discovery/stun_client.h"

#include <sstream>

namespace p2p_core {

MinimalIce::MinimalIce(const P2PConfig& cfg) : cfg_(cfg) {}

std::vector<IceCandidate> MinimalIce::gather_candidates() {
  std::vector<IceCandidate> out;
  // host
  out.push_back(IceCandidate{"udp", "0.0.0.0", cfg_.listenPort, IceCandidateType::Host});
  // reflexive via STUN
  if (!cfg_.stunServers.empty()) {
    StunClient client;
    for (const auto& s : cfg_.stunServers) {
      auto ep = client.discover_mapped_address(s);
      if (ep) { out.push_back(IceCandidate{"udp", ep->host, ep->port, IceCandidateType::ServerReflexive}); break; }
    }
  }
  return out;
}

std::optional<IcePair> MinimalIce::connectivity_check(const std::vector<IceCandidate>& local,
                                                      const std::vector<IceCandidate>& remote,
                                                      int /*timeoutMs*/) {
  // Placeholder: in a full ICE, we would try UDP STUN binding checks. Here, just pair first s-rflx if exists
  IceCandidate l = local.empty() ? IceCandidate{} : local.front();
  IceCandidate r = remote.empty() ? IceCandidate{} : remote.front();
  return IcePair{l, r};
}

static std::string cand_to_json(const IceCandidate& c) {
  std::ostringstream oss;
  oss << "{\"protocol\":\"" << c.protocol << "\",\"ip\":\"" << c.ip
      << "\",\"port\":" << c.port << ",\"type\":" << static_cast<int>(c.type) << "}";
  return oss.str();
}

std::string MinimalIce::to_json(const std::vector<IceCandidate>& cands) {
  std::ostringstream oss; oss << "{\"candidates\":[";
  for (size_t i=0;i<cands.size();++i) { if (i) oss << ","; oss << cand_to_json(cands[i]); }
  oss << "]}"; return oss.str();
}

std::vector<IceCandidate> MinimalIce::from_json(const std::string& json) {
  std::vector<IceCandidate> out;
  auto arrPos = json.find("["); auto arrEnd = json.find("]", arrPos);
  if (arrPos==std::string::npos||arrEnd==std::string::npos||arrEnd<=arrPos) return out;
  std::string arr = json.substr(arrPos+1, arrEnd-arrPos-1);
  std::istringstream ais(arr); std::string item;
  while (std::getline(ais, item, '}')) {
    auto ppos = item.find("\"protocol\""); auto ippos = item.find("\"ip\""); auto tpos = item.find("\"type\"");
    if (ppos==std::string::npos||ippos==std::string::npos) continue;
    auto q1 = item.find('"', ppos+9); q1 = item.find('"', q1+1); auto q2 = item.find('"', q1+1); auto q3 = item.find('"', q2+1);
    std::string proto = (q2!=std::string::npos && q3!=std::string::npos) ? item.substr(q2+1, q3-q2-1) : "udp";
    auto i1 = item.find('"', ippos+4); i1 = item.find('"', i1+1); auto i2 = item.find('"', i1+1); auto i3 = item.find('"', i2+1);
    std::string ip = (i2!=std::string::npos && i3!=std::string::npos) ? item.substr(i2+1, i3-i2-1) : "0.0.0.0";
    auto p = item.find("\"port\""); auto c = item.find(':', p); int port = c!=std::string::npos ? std::stoi(item.substr(c+1)) : 0;
    int t = 0; if (tpos!=std::string::npos) { auto c2=item.find(':', tpos); if (c2!=std::string::npos) t = std::stoi(item.substr(c2+1)); }
    out.push_back(IceCandidate{proto, ip, static_cast<std::uint16_t>(port), static_cast<IceCandidateType>(t)});
  }
  return out;
}

} // namespace p2p_core



