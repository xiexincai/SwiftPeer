#include "p2p/data/rate_limiter.h"
#include <algorithm>
#include <chrono>

namespace p2p_core {

TokenBucketLimiter::TokenBucketLimiter(double rate_bps, double burst_bytes)
    : rate_bps_(rate_bps)
    , burst_bytes_(burst_bytes)
    , tokens_(burst_bytes)
    , last_update_(std::chrono::steady_clock::now()) {
}

size_t TokenBucketLimiter::consume(size_t requested_bytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    refill_tokens();
    
    if (tokens_ >= static_cast<double>(requested_bytes)) {
        tokens_ -= static_cast<double>(requested_bytes);
        return requested_bytes;
    } else {
        size_t consumed = static_cast<size_t>(tokens_);
        tokens_ = 0.0;
        return consumed;
    }
}

double TokenBucketLimiter::available_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    const_cast<TokenBucketLimiter*>(this)->refill_tokens();
    return tokens_;
}

void TokenBucketLimiter::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_ = burst_bytes_;
    last_update_ = std::chrono::steady_clock::now();
}

void TokenBucketLimiter::refill_tokens() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double>(now - last_update_).count();
    
    if (elapsed > 0.0) {
        double new_tokens = rate_bps_ * elapsed;
        tokens_ = std::min(tokens_ + new_tokens, burst_bytes_);
        last_update_ = now;
    }
}

} // namespace p2p_core


