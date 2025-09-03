#pragma once

#include <chrono>
#include <atomic>
#include <mutex>

namespace p2p_core {

class TokenBucketLimiter {
public:
    explicit TokenBucketLimiter(double rate_bps, double burst_bytes);
    
    // 尝试消耗指定数量的字节，返回实际消耗的字节数
    size_t consume(size_t requested_bytes);
    
    // 获取当前可用的令牌数量
    double available_tokens() const;
    
    // 重置速率限制器
    void reset();

private:
    double rate_bps_;           // 速率（字节/秒）
    double burst_bytes_;        // 突发字节数
    double tokens_;             // 当前可用令牌数
    std::chrono::steady_clock::time_point last_update_; // 最后更新时间
    
    mutable std::mutex mutex_;
    
    void refill_tokens();
};

} // namespace p2p_core