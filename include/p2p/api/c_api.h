#pragma once

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
#include "p2p/core/export.h"
#else
#include <stdint.h>
#include <stddef.h>
#define P2P_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* p2p_handle_t;

typedef struct p2p_endpoint_t {
  const char* host;
  uint16_t port;
} p2p_endpoint_t;

typedef struct p2p_config_t {
  const char* node_id; // 可为空
  uint16_t listen_port; // 0 -> 自动选择
  double max_upload_bps; // 0 -> 无限制
  double max_download_bps; // 0 -> 无限制
  double max_burst_bytes; // 默认65536
  uint32_t max_peers; // 默认50
} p2p_config_t;

typedef struct p2p_piece_id_t {
  const char* stream_id;
  uint64_t piece_index;
} p2p_piece_id_t;

typedef struct p2p_piece_t {
  p2p_piece_id_t id;
  const unsigned char* data;
  size_t size;
} p2p_piece_t;

typedef struct p2p_stats_t {
  uint64_t bytes_uploaded;
  uint64_t bytes_downloaded;
  uint32_t connected_peers;
} p2p_stats_t;

typedef void (*p2p_piece_received_cb)(const p2p_piece_t* piece, void* user);

P2P_API p2p_handle_t p2p_create(const p2p_config_t* cfg);
P2P_API void p2p_destroy(p2p_handle_t h);
P2P_API int p2p_start(p2p_handle_t h);
P2P_API void p2p_stop(p2p_handle_t h);

P2P_API void p2p_add_seed(p2p_handle_t h, const p2p_endpoint_t* ep);
P2P_API void p2p_publish_piece(p2p_handle_t h, const p2p_piece_t* piece);
P2P_API void p2p_request_piece(p2p_handle_t h, const p2p_piece_id_t* id);
P2P_API void p2p_set_piece_received_callback(p2p_handle_t h, p2p_piece_received_cb cb, void* user);
P2P_API p2p_stats_t p2p_stats(p2p_handle_t h);

#ifdef __cplusplus
}
#endif
