#include "p2p/data/segment_store.h"
#include <sstream>
#include <algorithm>

namespace p2p_core {

InMemorySegmentStore::InMemorySegmentStore() = default;

void InMemorySegmentStore::put(const PieceData& piece) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = make_key(piece.id);
    pieces_[key] = piece;
    
    // Update stream index
    auto& indices = stream_indices_[piece.id.streamId];
    if (std::find(indices.begin(), indices.end(), piece.id.pieceIndex) == indices.end()) {
        indices.push_back(piece.id.pieceIndex);
        std::sort(indices.begin(), indices.end());
    }
}

std::optional<PieceData> InMemorySegmentStore::get(const PieceId& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = make_key(id);
    auto it = pieces_.find(key);
    if (it != pieces_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool InMemorySegmentStore::has(const PieceId& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::string key = make_key(id);
    return pieces_.find(key) != pieces_.end();
}

std::vector<uint64_t> InMemorySegmentStore::available_piece_indices(const std::string& stream_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = stream_indices_.find(stream_id);
    if (it != stream_indices_.end()) {
        return it->second;
    }
    return {};
}

size_t InMemorySegmentStore::total_pieces() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pieces_.size();
}

size_t InMemorySegmentStore::total_bytes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total = 0;
    for (const auto& [key, piece] : pieces_) {
        total += piece.data.size();
    }
    return total;
}

void InMemorySegmentStore::cleanup_expired(const std::chrono::seconds& max_age) {
    // Simplified implementation, actual project may need to add timestamp fields
    // Temporarily not implementing cleanup logic here
}

std::string InMemorySegmentStore::make_key(const PieceId& id) const {
    std::ostringstream oss;
    oss << id.streamId << ":" << id.pieceIndex;
    return oss.str();
}

} // namespace p2p_core


