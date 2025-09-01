#pragma once

#include <p2p/core/types.h>
#include <p2p/core/export.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <unordered_map>

namespace p2p_core {

// 前向声明
class EnhancedDHT;
class TopologyOptimizer;
class NetworkPartitionDetector;

/**
 * DHT节点信息 - 增强�?
 */
struct DHTNode {
    std::string node_id;                   // 节点ID�?60位）
    std::string address;                    // IP地址
    uint16_t port;                         // 端口
    uint16_t dht_port;                     // DHT专用端口
    
    // 网络质量指标
    double latency_ms;                     // 延迟（毫秒）
    double bandwidth_mbps;                 // 带宽（Mbps�?
    double packet_loss_rate;               // 丢包�?
    double jitter_ms;                      // 抖动
    
    // 地理位置信息
    double latitude;                       // 纬度
    double longitude;                      // 经度
    std::string country_code;              // 国家代码
    std::string isp;                       // 网络服务�?
    
    // 节点状�?
    bool is_online;                        // 是否在线
    bool is_stable;                        // 是否稳定
    uint32_t uptime_seconds;               // 在线时长
    uint32_t last_seen_seconds;            // 最后活跃时�?
    
    // 性能指标
    uint32_t successful_queries;           // 成功查询次数
    uint32_t failed_queries;               // 失败查询次数
    uint32_t response_time_avg_ms;         // 平均响应时间
    uint32_t last_query_time;              // 最后查询时�?
    
    // 网络拓扑信息
    uint32_t network_id;                   // 网络ID
    uint32_t subnet_mask;                  // 子网掩码
    std::string network_type;              // 网络类型（家�?企业/移动�?
    
    DHTNode();
    double get_quality_score() const;
    double get_reliability_score() const;
    bool is_expired(std::chrono::seconds max_age) const;
    bool is_in_same_network(const DHTNode& other) const;
    double get_distance(const DHTNode& other) const;
};

/**
 * DHT路由表桶 - 优化�?
 */
struct DHTBucket {
    std::vector<DHTNode> nodes;            // 节点列表
    std::vector<DHTNode> replacement_cache; // 替换缓存
    uint32_t last_updated;                 // 最后更新时�?
    uint32_t query_count;                  // 查询次数
    uint32_t success_count;                // 成功次数
    
    // 桶状�?
    bool is_stale;                         // 是否过期
    bool needs_refresh;                    // 是否需要刷�?
    uint32_t refresh_interval;             // 刷新间隔
    
    DHTBucket();
    void add_node(const DHTNode& node);
    void remove_node(const std::string& node_id);
    bool contains_node(const std::string& node_id) const;
    void update_node_status(const std::string& node_id, bool is_online);
    void sort_by_quality();
    void cleanup_expired_nodes(std::chrono::seconds max_age);
};

/**
 * DHT查询结果
 */
struct DHTQueryResult {
    bool success;                          // 是否成功
    std::vector<DHTNode> nodes;           // 找到的节�?
    std::vector<std::string> values;      // 找到的�?
    double query_time_ms;                 // 查询耗时
    uint32_t hops;                        // 跳数
    std::string error_message;            // 错误信息
    
    DHTQueryResult();
};

/**
 * 网络分区信息
 */
struct NetworkPartition {
    uint32_t partition_id;                 // 分区ID
    std::vector<DHTNode> nodes;           // 分区内节�?
    std::vector<std::string> gateways;    // 网关节点
    double connectivity_score;             // 连通性评�?
    bool is_stable;                        // 是否稳定
    std::chrono::steady_clock::time_point created_time;
    
    NetworkPartition();
    size_t get_node_count() const;
    double get_average_latency() const;
    bool contains_node(const std::string& node_id) const;
};

/**
 * DHT配置 - 增强�?
 */
struct DHTConfig {
    // 基本配置
    uint16_t port;                         // DHT端口
    uint16_t bootstrap_port;               // 引导端口
    uint32_t max_nodes_per_bucket;        // 每桶最大节点数
    uint32_t max_buckets;                 // 最大桶�?
    
    // 网络优化配置
    bool enable_topology_optimization;     // 启用拓扑优化
    bool enable_geographic_clustering;     // 启用地理聚类
    bool enable_network_awareness;         // 启用网络感知
    bool enable_partition_detection;       // 启用分区检�?
    
    // 性能配置
    uint32_t query_timeout_ms;             // 查询超时时间
    uint32_t refresh_interval_ms;          // 刷新间隔
    uint32_t max_query_hops;               // 最大查询跳�?
    uint32_t parallel_query_count;         // 并行查询数量
    
    // 质量阈�?
    double min_node_quality_score;         // 最小节点质量评�?
    double max_node_latency_ms;            // 最大节点延�?
    double max_node_packet_loss_rate;      // 最大节点丢包率
    
    // 高级配置
    bool enable_machine_learning;          // 启用机器学习
    bool enable_adaptive_routing;          // 启用自适应路由
    bool enable_load_balancing;            // 启用负载均衡
    
    DHTConfig();
};

/**
 * 增强的DHT实现 - 基于最新学术研�?
 */
class P2P_API EnhancedDHT {
public:
    explicit EnhancedDHT(const DHTConfig& config);
    ~EnhancedDHT();
    
    // 基本操作
    bool initialize();
    void shutdown();
    bool is_initialized() const;
    
    // 节点管理
    bool add_node(const DHTNode& node);
    bool remove_node(const std::string& node_id);
    bool update_node(const DHTNode& node);
    DHTNode get_node(const std::string& node_id) const;
    std::vector<DHTNode> get_all_nodes() const;
    
    // DHT操作
    bool put(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool remove(const std::string& key);
    
    // 节点发现
    std::vector<DHTNode> find_node(const std::string& target_id);
    std::vector<DHTNode> find_node_async(const std::string& target_id,
                                        std::function<void(std::vector<DHTNode>)> callback);
    
    // 网络优化
    void optimize_topology();
    void optimize_routing_table();
    void cleanup_stale_nodes();
    void refresh_routing_table();
    
    // 分区管理
    std::vector<NetworkPartition> detect_partitions();
    bool merge_partitions(uint32_t partition1_id, uint32_t partition2_id);
    bool repair_partition(uint32_t partition_id);
    
    // 性能监控
    double get_query_success_rate() const;
    double get_average_query_time() const;
    uint32_t get_total_queries() const;
    uint32_t get_active_nodes() const;
    
    // 配置管理
    void update_config(const DHTConfig& config);
    DHTConfig get_config() const;
    
    // 高级功能
    void enable_machine_learning_optimization(bool enable);
    void set_custom_node_evaluator(std::function<double(const DHTNode&)> evaluator);
    void set_network_awareness_thresholds(double max_latency, double max_packet_loss);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * 拓扑优化�?
 */
class P2P_API TopologyOptimizer {
public:
    explicit TopologyOptimizer(const DHTConfig& config);
    ~TopologyOptimizer();
    
    // 拓扑优化
    void optimize_node_placement(std::vector<DHTNode>& nodes);
    void optimize_routing_paths(const std::vector<DHTNode>& nodes);
    void optimize_network_clustering(std::vector<DHTNode>& nodes);
    
    // 网络感知
    void analyze_network_topology(const std::vector<DHTNode>& nodes);
    void detect_network_bottlenecks(const std::vector<DHTNode>& nodes);
    void optimize_cross_network_connections(const std::vector<DHTNode>& nodes);
    
    // 地理优化
    void optimize_geographic_distribution(std::vector<DHTNode>& nodes);
    void create_geographic_clusters(const std::vector<DHTNode>& nodes);
    void optimize_inter_cluster_connections(const std::vector<DHTNode>& nodes);
    
    // 性能评估
    double evaluate_topology_quality(const std::vector<DHTNode>& nodes) const;
    double evaluate_routing_efficiency(const std::vector<DHTNode>& nodes) const;
    double evaluate_network_resilience(const std::vector<DHTNode>& nodes) const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * 网络分区检测器
 */
class P2P_API NetworkPartitionDetector {
public:
    explicit NetworkPartitionDetector(const DHTConfig& config);
    ~NetworkPartitionDetector();
    
    // 分区检�?
    std::vector<NetworkPartition> detect_partitions(const std::vector<DHTNode>& nodes);
    bool is_partitioned(const std::vector<DHTNode>& nodes) const;
    uint32_t get_partition_count(const std::vector<DHTNode>& nodes) const;
    
    // 分区分析
    void analyze_partition_causes(const std::vector<NetworkPartition>& partitions);
    void identify_gateway_nodes(const std::vector<NetworkPartition>& partitions);
    void calculate_partition_metrics(const std::vector<NetworkPartition>& partitions);
    
    // 分区修复
    bool repair_partition(NetworkPartition& partition, const std::vector<DHTNode>& all_nodes);
    bool merge_partitions(NetworkPartition& partition1, NetworkPartition& partition2);
    std::vector<DHTNode> find_bridge_nodes(const NetworkPartition& partition1, 
                                          const NetworkPartition& partition2);
    
    // 预防措施
    void identify_partition_risks(const std::vector<DHTNode>& nodes);
    void suggest_preventive_measures(const std::vector<DHTNode>& nodes);
    void monitor_partition_trends(const std::vector<NetworkPartition>& partitions);
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * 自适应路由优化�?
 */
class P2P_API AdaptiveRoutingOptimizer {
public:
    explicit AdaptiveRoutingOptimizer(const DHTConfig& config);
    ~AdaptiveRoutingOptimizer();
    
    // 路由优化
    void optimize_routing_table(std::vector<DHTBucket>& buckets);
    void optimize_query_routing(const std::string& target_id, std::vector<DHTNode>& candidates);
    void adapt_to_network_changes(const std::vector<DHTNode>& nodes);
    
    // 负载均衡
    void balance_query_load(const std::vector<DHTNode>& nodes);
    void distribute_storage_load(const std::vector<DHTNode>& nodes);
    void optimize_network_traffic(const std::vector<DHTNode>& nodes);
    
    // 自适应调整
    void adjust_routing_parameters(const std::vector<DHTNode>& nodes);
    void optimize_timeout_values(const std::vector<DHTNode>& nodes);
    void adjust_retry_strategies(const std::vector<DHTNode>& nodes);
    
    // 性能监控
    double get_routing_efficiency() const;
    double get_load_balance_score() const;
    double get_network_utilization() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace p2p_core
