#pragma once

#include <chrono>
#include <atomic>
#include <mutex>

namespace p2p_core {

class TokenBucketLimiter {
public:
    explicit TokenBucketLimiter(double rate_bps, double burst_bytes);
    
    // 尝试消费指定字节数，返回实际消费的字节数
    size_t consume(size_t requested_bytes);
    
    // 获取当前可用令牌数
    double available_tokens() const;
    
    // 重置限流器
    void reset();

private:
    double rate_bps_;           // 速率（字节/秒）
    double burst_bytes_;        // 突发字节数
    double tokens_;             // 当前可用令牌数
    std::chrono::steady_clock::time_point last_update_; // 上次更新时间
    
    mutable std::mutex mutex_;
    
    void refill_tokens();
};

} // namespace p2p_core


