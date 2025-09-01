#include "p2p/api/c_api.h"
#include "p2p/core/peer_engine.h"
#include "p2p/core/types.h"

#include <memory>
#include <vector>
#include <string>

using namespace p2p_core;

struct HandleWrapper {
  std::unique_ptr<PeerEngine> engine;
  p2p_piece_received_cb cb{nullptr};
  void* user{nullptr};
};

static void bridge_piece_cb(HandleWrapper* h, const PieceData& pd) {
  if (!h || !h->cb) return;
  p2p_piece_t cpiece;
  cpiece.id.stream_id = pd.id.streamId.c_str();
  cpiece.id.piece_index = pd.id.pieceIndex;
  cpiece.data = pd.data.data();
  cpiece.size = pd.data.size();
  h->cb(&cpiece, h->user);
}

extern "C" {

P2P_API p2p_handle_t p2p_create(const p2p_config_t* cfg) {
  P2PConfig c;
  if (cfg) {
    if (cfg->node_id) c.nodeId = cfg->node_id;
    c.listenPort = cfg->listen_port;
    c.maxUploadBps = cfg->max_upload_bps;
    c.maxDownloadBps = cfg->max_download_bps;
    c.maxBurstBytes = cfg->max_burst_bytes > 0 ? cfg->max_burst_bytes : 65536;
    c.maxPeers = cfg->max_peers > 0 ? cfg->max_peers : 50;
  }
  auto h = new HandleWrapper();
  h->engine = std::make_unique<PeerEngine>(std::move(c));
  h->engine->set_piece_received_callback([h](const PieceData& pd){ bridge_piece_cb(h, pd); });
  return reinterpret_cast<p2p_handle_t>(h);
}

P2P_API void p2p_destroy(p2p_handle_t handle) {
  if (!handle) return;
  auto h = reinterpret_cast<HandleWrapper*>(handle);
  delete h;
}

P2P_API int p2p_start(p2p_handle_t handle) {
  if (!handle) return 0;
  auto h = reinterpret_cast<HandleWrapper*>(handle);
  return h->engine->start() ? 1 : 0;
}

P2P_API void p2p_stop(p2p_handle_t handle) {
  if (!handle) return;
  auto h = reinterpret_cast<HandleWrapper*>(handle);
  h->engine->stop();
}

P2P_API void p2p_add_seed(p2p_handle_t handle, const p2p_endpoint_t* ep) {
  if (!handle || !ep || !ep->host) return;
  auto h = reinterpret_cast<HandleWrapper*>(handle);
  h->engine->add_seed_peer(Endpoint{std::string{ep->host}, ep->port});
}

P2P_API void p2p_publish_piece(p2p_handle_t handle, const p2p_piece_t* piece) {
  if (!handle || !piece || !piece->id.stream_id || !piece->data) return;
  auto h = reinterpret_cast<HandleWrapper*>(handle);
  PieceData pd; pd.id.streamId = piece->id.stream_id; pd.id.pieceIndex = piece->id.piece_index;
  pd.data.assign(piece->data, piece->data + piece->size);
  h->engine->publish_piece(pd);
}

P2P_API void p2p_request_piece(p2p_handle_t handle, const p2p_piece_id_t* id) {
  if (!handle || !id || !id->stream_id) return;
  auto h = reinterpret_cast<HandleWrapper*>(handle);
  h->engine->request_piece(PieceId{std::string{id->stream_id}, id->piece_index});
}

P2P_API void p2p_set_piece_received_callback(p2p_handle_t handle, p2p_piece_received_cb cb, void* user) {
  if (!handle) return;
  auto h = reinterpret_cast<HandleWrapper*>(handle);
  h->cb = cb; h->user = user;
}

P2P_API p2p_stats_t p2p_stats(p2p_handle_t handle) {
  p2p_stats_t s{0,0,0};
  if (!handle) return s;
  auto h = reinterpret_cast<HandleWrapper*>(handle);
  auto ss = h->engine->stats();
  s.bytes_downloaded = ss.bytesDownloaded;
  s.bytes_uploaded = ss.bytesUploaded;
  s.connected_peers = ss.connectedPeers;
  return s;
}

}


