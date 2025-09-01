#pragma once

#include "p2p/core/export.h"
#include "p2p/core/config.h"
#include "p2p/core/types.h"
#include "p2p/transport/transport.h"
#include "p2p/data/protocol.h"
#include "p2p/data/segment_store.h"
#include "p2p/data/rate_limiter.h"
#include "p2p/discovery/dht.h"
#include "p2p/discovery/tracker_client.h"
#include "p2p/discovery/dns_resolver.h"
#include "p2p/security/crypto.h"

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace p2p_core {

// 前向声明
class ITcpTransport;

// 回调类型定义
using PieceReceivedCallback = std::function<void(const PieceData&)>;  // 数据块接收回调
using DnsResolveCallback = std::function<std::string(const std::string&)>;  // DNS解析回调

/**
 * @brief P2P网络引擎主类
 * 
 * 这是SwiftPeer库的核心类，负责管理P2P网络连接、数据传输、节点发现等所有功能。
 * 提供完整的P2P网络通信解决方案，支持TCP、TLS、QUIC等多种传输协议。
 */
class P2P_CORE_API PeerEngine {
public:
    /**
     * @brief 构造函数
     * @param config P2P配置参数
     * 
     * 根据配置参数初始化P2P引擎，包括传输层、速率限制器、数据存储等组件
     */
    explicit PeerEngine(P2PConfig config);
    
    /**
     * @brief 析构函数
     * 
     * 自动停止引擎并清理所有资源
     */
    ~PeerEngine();

    // 核心生命周期管理
    /**
     * @brief 启动P2P引擎
     * @return 启动成功返回true，失败返回false
     * 
     * 启动传输层监听、初始化DHT网络、启动IO线程等
     */
    bool start();
    
    /**
     * @brief 停止P2P引擎
     * 
     * 停止所有传输层、关闭DHT网络、等待IO线程结束
     */
    void stop();

    // 数据块管理
    /**
     * @brief 发布数据块
     * @param piece 要发布的数据块
     * 
     * 将数据块存储到本地存储中，供其他节点下载
     */
    void publish_piece(const PieceData& piece);
    
    /**
     * @brief 请求数据块
     * @param id 要请求的数据块ID
     * 
     * 向所有连接的节点发送数据块请求
     */
    void request_piece(const PieceId& id);

    // 节点管理
    /**
     * @brief 添加种子节点
     * @param ep 种子节点的端点信息
     * 
     * 添加用于网络引导的种子节点，建立初始连接
     */
    void add_seed_peer(const Endpoint& ep);

    // 回调设置
    /**
     * @brief 设置数据块接收回调
     * @param cb 接收回调函数
     * 
     * 当接收到数据块时，会调用此回调函数
     */
    void set_piece_received_callback(PieceReceivedCallback cb);
    
    /**
     * @brief 设置自定义DNS解析器
     * @param cb 自定义解析回调函数
     * 
     * 允许应用程序提供自定义的DNS解析逻辑
     */
    void set_custom_resolver(DnsResolveCallback cb);

    // 统计信息
    /**
     * @brief 获取统计快照
     * @return 包含上传/下载字节数、连接节点数等统计信息
     * 
     * 返回当前引擎运行状态的统计信息
     */
    StatsSnapshot stats() const;

private:
    // 配置参数
    P2PConfig config_;  // P2P配置参数

    // 传输层组件
    std::unique_ptr<ITcpTransport> tcp_;      // TCP传输层
    std::unique_ptr<ITcpTransport> tlsTcp_;  // TLS加密传输层

    // 速率限制器
    std::unique_ptr<TokenBucketLimiter> upLimiter_;    // 上传速率限制器
    std::unique_ptr<TokenBucketLimiter> downLimiter_;  // 下载速率限制器

    // 数据存储
    std::unique_ptr<InMemorySegmentStore> store_;  // 内存数据块存储

    // 节点发现组件
    SimpleDhtNode dht_;                           // DHT网络节点
    std::unique_ptr<TrackerClient> tracker_;      // Tracker客户端

    // DNS解析
    std::unique_ptr<DnsResolver> resolver_;       // DNS解析器

    // 回调函数
    PieceReceivedCallback onPiece_;               // 数据块接收回调

    // 运行状态
    std::atomic<bool> running_{false};            // 引擎运行状态标志
    std::thread ioThread_;                        // IO处理线程

    // 统计信息
    std::atomic<std::uint64_t> bytesUp_{0};      // 已上传字节数
    std::atomic<std::uint64_t> bytesDown_{0};    // 已下载字节数

    // 连接管理
    mutable std::mutex mutex_;                    // 连接管理互斥锁
    std::unordered_map<std::string, ConnectionPtr> connectionsByKey_;  // 按键值管理的连接
    std::unordered_set<std::string> pendingRequests_;                 // 待处理的请求

    // 内部方法
    /**
     * @brief IO线程主函数
     * 
     * 处理网络IO事件、连接管理、数据转发等
     */
    void io_thread_main();
    
    /**
     * @brief 连接种子节点
     * 
     * 建立与种子节点的初始连接
     */
    void connect_seed_peers();
    
    /**
     * @brief 向Tracker服务器通告
     * 
     * 向Tracker服务器通告本节点信息
     */
    void announce_to_trackers();
    
    /**
     * @brief 处理新连接
     * @param c 新建立的连接
     * 
     * 当有新连接建立时调用，进行握手和初始化
     */
    void on_connection(const ConnectionPtr& c);
    
    /**
     * @brief 处理接收到的数据帧
     * @param c 连接对象
     * @param f 接收到的数据帧
     * 
     * 解析和处理接收到的网络数据帧
     */
    void on_frame(const ConnectionPtr& c, const Frame& f);
    
    /**
     * @brief 处理连接关闭
     * @param c 要关闭的连接
     * 
     * 清理连接资源，更新连接状态
     */
    void on_close(const ConnectionPtr& c);
    
    /**
     * @brief 发送通告消息
     * @param c 目标连接
     * 
     * 向指定连接发送节点通告信息
     */
    void send_announce(const ConnectionPtr& c);
    void send_request(const ConnectionPtr& c, const PieceId& id);
    void send_piece(const ConnectionPtr& c, const PieceData& piece);
    std::string make_key(const PieceId& id) const;
};

} // namespace p2p_core
