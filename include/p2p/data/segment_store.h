#pragma once

#include "p2p/core/types.h"
#include <unordered_map>
#include <mutex>
#include <vector>
#include <optional>
#include <chrono>

namespace p2p_core {

class InMemorySegmentStore {
public:
    InMemorySegmentStore();
    ~InMemorySegmentStore() = default;
    
    // 存储数据块
    void put(const PieceData& piece);
    
    // 获取数据块
    std::optional<PieceData> get(const PieceId& id) const;
    
    // 检查数据块是否存在
    bool has(const PieceId& id) const;
    
    // 获取所有可用的数据块索引
    std::vector<uint64_t> available_piece_indices(const std::string& stream_id) const;
    
    // 获取存储统计信息
    size_t total_pieces() const;
    size_t total_bytes() const;
    
    // 清理过期数据
    void cleanup_expired(const std::chrono::seconds& max_age);

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, PieceData> pieces_; // 键：stream_id:piece_index
    std::unordered_map<std::string, std::vector<uint64_t>> stream_indices_; // stream_id -> 数据块索引
    
    std::string make_key(const PieceId& id) const;
};

} // namespace p2p_core