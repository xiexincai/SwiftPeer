#pragma once

#include <p2p/core/types.h>
#include <p2p/core/export.h>
#include <memory>
#include <vector>
#include <functional>
#include <chrono>
#include <atomic>

namespace p2p_core {

// 前向声明
class NATTraversalEngine;
class ConnectionQualityMonitor;

/**
 * NAT类型枚举 - 基于RFC 3489和最新研究
 */
enum class NATType {
    UNKNOWN = 0,           // 未知类型
    OPEN_INTERNET = 1,     // 公网直连
    FULL_CONE = 2,         // 全锥型NAT
    RESTRICTED_CONE = 3,   // 受限锥型NAT
    PORT_RESTRICTED_CONE = 4, // 端口受限锥型NAT
    SYMMETRIC = 5,         // 对称型NAT
    UDP_BLOCKED = 6        // UDP被阻塞
};

/**
 * 候选地址类型 - 基于ICE框架
 */
enum class CandidateType {
    HOST = 0,              // 本地地址
    SERVER_REFLEXIVE = 1,  // STUN反射地址
    PEER_REFLEXIVE = 2,    // 对等反射地址
    RELAYED = 3            // TURN中继地址
};

/**
 * 候选地址信息
 */
struct CandidateAddress {
    std::string address;           // IP地址
    uint16_t port;                 // 端口
    CandidateType type;            // 候选类型
    NATType nat_type;              // 对应的NAT类型
    uint32_t priority;             // 优先级（0-65535）
    std::string foundation;        // 基础标识符
    std::string component_id;      // 组件ID
    std::string username;          // 用户名（TURN）
    std::string password;          // 密码（TURN）
    
    // 连接质量指标
    double latency_ms;             // 延迟（毫秒）
    double bandwidth_mbps;         // 带宽（Mbps）
    double packet_loss_rate;       // 丢包率
    uint32_t connection_attempts;  // 连接尝试次数
    uint32_t successful_connections; // 成功连接次数
    
    // 时间戳
    std::chrono::steady_clock::time_point last_checked;
    std::chrono::steady_clock::time_point last_successful;
    
    CandidateAddress();
    double get_success_rate() const;
    double get_quality_score() const;
    bool is_expired(std::chrono::seconds max_age) const;
};

/**
 * 连接质量指标
 */
struct ConnectionQuality {
    double latency_ms;             // 延迟
    double bandwidth_mbps;         // 带宽
    double packet_loss_rate;       // 丢包率
    double jitter_ms;              // 抖动
    uint32_t rtt_samples;          // RTT样本数
    std::chrono::steady_clock::time_point timestamp;
    
    ConnectionQuality();
    double get_overall_score() const;
    bool is_good() const;
    bool is_poor() const;
};

/**
 * NAT穿透配置
 */
struct NATTraversalConfig {
    // 基本配置
    uint32_t connection_timeout_ms;        // 连接超时时间
    uint32_t max_retry_attempts;           // 最大重试次数
    uint32_t concurrent_connections;       // 并发连接数
    uint32_t heartbeat_interval_ms;        // 心跳间隔
    
    // 高级配置
    bool enable_parallel_connections;      // 启用并行连接
    bool enable_adaptive_timeout;          // 启用自适应超时
    bool enable_quality_monitoring;        // 启用质量监控
    bool enable_machine_learning;          // 启用机器学习优化
    
    // STUN/TURN配置
    std::vector<std::string> stun_servers; // STUN服务器列表
    std::vector<std::string> turn_servers; // TURN服务器列表
    std::string turn_username;             // TURN用户名
    std::string turn_password;             // TURN密码
    
    // 质量阈值
    double min_bandwidth_mbps;             // 最小带宽要求
    double max_latency_ms;                 // 最大延迟要求
    double max_packet_loss_rate;           // 最大丢包率
    
    NATTraversalConfig();
};

/**
 * NAT穿透结果
 */
struct NATTraversalResult {
    bool success;                          // 是否成功
    NATType detected_nat_type;             // 检测到的NAT类型
    std::vector<CandidateAddress> candidates; // 候选地址列表
    std::string selected_address;          // 选中的地址
    uint16_t selected_port;                // 选中的端口
    double connection_time_ms;             // 连接建立时间
    ConnectionQuality quality;             // 连接质量
    std::string error_message;             // 错误信息
    
    NATTraversalResult();
};

/**
 * 连接质量监控器
 */
class P2P_API ConnectionQualityMonitor {
public:
    explicit ConnectionQualityMonitor(const NATTraversalConfig& config);
    ~ConnectionQualityMonitor();
    
    // 质量监控
    void start_monitoring(const std::string& address, uint16_t port);
    void stop_monitoring(const std::string& address, uint16_t port);
    ConnectionQuality get_quality(const std::string& address, uint16_t port) const;
    
    // 质量评估
    double evaluate_connection_quality(const CandidateAddress& candidate) const;
    bool should_retry_connection(const CandidateAddress& candidate) const;
    uint32_t get_optimal_retry_delay(const CandidateAddress& candidate) const;
    
    // 统计信息
    double get_average_success_rate() const;
    double get_average_connection_time() const;
    uint32_t get_total_connections() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * 增强的NAT穿透引擎 - 基于最新学术研究
 */
class P2P_API NATTraversalEngine {
public:
    explicit NATTraversalEngine(const NATTraversalConfig& config);
    ~NATTraversalEngine();
    
    // 基本操作
    bool initialize();
    void shutdown();
    bool is_initialized() const;
    
    // NAT类型检测
    NATType detect_nat_type();
    NATType detect_nat_type_async(std::function<void(NATType)> callback);
    
    // 候选地址收集
    std::vector<CandidateAddress> collect_candidates();
    void collect_candidates_async(std::function<void(std::vector<CandidateAddress>)> callback);
    
    // 连接建立
    NATTraversalResult establish_connection(const std::vector<CandidateAddress>& candidates);
    void establish_connection_async(const std::vector<CandidateAddress>& candidates,
                                  std::function<void(NATTraversalResult)> callback);
    
    // 连接质量监控
    void start_quality_monitoring(const std::string& address, uint16_t port);
    void stop_quality_monitoring(const std::string& address, uint16_t port);
    ConnectionQuality get_connection_quality(const std::string& address, uint16_t port) const;
    
    // 智能重试
    bool retry_connection(const std::string& address, uint16_t port);
    void retry_connection_async(const std::string& address, uint16_t port,
                               std::function<void(bool)> callback);
    
    // 配置管理
    void update_config(const NATTraversalConfig& config);
    NATTraversalConfig get_config() const;
    
    // 统计信息
    double get_success_rate() const;
    double get_average_connection_time() const;
    uint32_t get_total_attempts() const;
    uint32_t get_successful_connections() const;
    
    // 高级功能
    void enable_machine_learning_optimization(bool enable);
    void set_custom_quality_evaluator(std::function<double(const CandidateAddress&)> evaluator);
    void set_connection_quality_thresholds(double min_bandwidth, double max_latency, double max_packet_loss);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * 智能连接管理器
 */
class P2P_API SmartConnectionManager {
public:
    explicit SmartConnectionManager(const NATTraversalConfig& config);
    ~SmartConnectionManager();
    
    // 连接管理
    bool establish_optimal_connection(const std::vector<CandidateAddress>& candidates);
    void maintain_connection_pool(size_t min_connections, size_t max_connections);
    void optimize_connection_parameters();
    
    // 负载均衡
    std::string select_best_candidate(const std::vector<CandidateAddress>& candidates) const;
    void distribute_connections(const std::vector<std::string>& addresses);
    
    // 故障转移
    bool switch_to_backup_connection(const std::string& failed_address);
    std::vector<std::string> get_backup_addresses() const;
    
    // 性能优化
    void prewarm_connections(const std::vector<std::string>& addresses);
    void cleanup_idle_connections();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * 机器学习优化器
 */
class P2P_API MLOptimizer {
public:
    explicit MLOptimizer(const NATTraversalConfig& config);
    ~MLOptimizer();
    
    // 模型训练
    void train_connection_success_model(const std::vector<NATTraversalResult>& historical_data);
    void train_quality_prediction_model(const std::vector<ConnectionQuality>& quality_data);
    
    // 预测
    double predict_connection_success_rate(const CandidateAddress& candidate) const;
    double predict_connection_quality(const CandidateAddress& candidate) const;
    uint32_t predict_optimal_timeout(const CandidateAddress& candidate) const;
    
    // 模型更新
    void update_model_with_result(const NATTraversalResult& result);
    void update_model_with_quality(const std::string& address, const ConnectionQuality& quality);
    
    // 模型管理
    void save_model(const std::string& filepath) const;
    bool load_model(const std::string& filepath);
    void reset_model();
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace p2p_core
