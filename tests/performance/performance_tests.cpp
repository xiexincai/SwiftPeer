#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <random>
#include <iomanip>
#include <sstream>
#include <core/peer_engine.h>
#include <core/config.h>
#include <core/types.h>

using namespace p2p_core;

// 性能测试结果结构
struct PerformanceResult {
    std::string test_name;
    double duration_ms;
    uint64_t pieces_processed;
    double pieces_per_second;
    double throughput_mbps;
    uint64_t memory_usage_kb;
    std::map<std::string, double> additional_metrics;
};

// 性能测试基类
class PerformanceTest {
protected:
    P2PConfig config_;
    std::unique_ptr<PeerEngine> engine_;
    std::vector<PerformanceResult> results_;
    
    // 统计信息
    std::atomic<uint64_t> total_pieces_processed_{0};
    std::atomic<uint64_t> total_bytes_processed_{0};
    std::atomic<uint64_t> total_errors_{0};
    
    // 同步
    std::mutex results_mutex_;
    std::condition_variable test_complete_cv_;
    std::atomic<bool> test_running_{false};

public:
    PerformanceTest() {
        setup_default_config();
    }
    
    virtual ~PerformanceTest() = default;
    
    virtual bool initialize() {
        try {
            engine_ = std::make_unique<PeerEngine>(config_);
            engine_->set_piece_received_callback(
                [this](const PieceData& piece) {
                    on_piece_received(piece);
                }
            );
            return true;
        } catch (const std::exception& e) {
            std::cerr << "初始化失�? " << e.what() << std::endl;
            return false;
        }
    }
    
    virtual void cleanup() {
        if (engine_) {
            engine_->stop();
        }
    }
    
    // 运行所有性能测试
    void run_all_tests() {
        std::cout << "开始性能测试套件..." << std::endl;
        std::cout << "==================" << std::endl << std::endl;
        
        if (!initialize()) {
            std::cerr << "测试初始化失�? << std::endl;
            return;
        }
        
        // 运行各种测试
        run_throughput_test();
        run_latency_test();
        run_concurrent_test();
        run_memory_test();
        run_stress_test();
        run_scalability_test();
        
        // 显示结果
        print_results();
        
        cleanup();
    }
    
    // 获取内存使用量（近似值）
    uint64_t get_memory_usage_kb() {
        // 这是一个简化的内存使用估算
        // 在实际实现中，可以使用更精确的方�?
        return total_pieces_processed_ * 2; // 每分片约2KB开销
    }

protected:
    void setup_default_config() {
        config_.nodeId = "perf_test_node";
        config_.listenPort = 0;
        config_.maxUploadBps = 0;        // 无限�?
        config_.maxDownloadBps = 0;      // 无限�?
        config_.maxBurstBytes = 1048576; // 1MB
        config_.maxPeers = 100;
        config_.enableTls = false;       // 测试时禁用TLS
        config_.enableDht = false;       // 测试时禁用DHT
        config_.enableQuic = false;
        config_.enableIce = false;
    }
    
    virtual void on_piece_received(const PieceData& piece) {
        total_pieces_processed_++;
        total_bytes_processed_ += piece.data.size();
    }
    
    void add_result(const PerformanceResult& result) {
        std::lock_guard<std::mutex> lock(results_mutex_);
        results_.push_back(result);
    }
    
    // 吞吐量测�?
    void run_throughput_test() {
        std::cout << "运行吞吐量测�?.." << std::endl;
        
        const std::vector<int> piece_sizes = {512, 1024, 4096, 16384}; // 不同分片大小
        const int pieces_per_size = 1000;
        
        for (int piece_size : piece_sizes) {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // 启动引擎
            engine_->start();
            
            // 批量发布分片
            for (int i = 0; i < pieces_per_size; ++i) {
                PieceData piece;
                piece.id.streamId = "throughput_test";
                piece.id.pieceIndex = i;
                piece.data = std::vector<uint8_t>(piece_size, i % 256);
                
                engine_->publish_piece(piece);
            }
            
            // 等待处理完成
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            // 计算指标
            double pieces_per_second = pieces_per_size / (duration.count() / 1000.0);
            double throughput_mbps = (pieces_per_size * piece_size * 8.0) / (duration.count() / 1000.0) / 1000000.0;
            
            PerformanceResult result;
            result.test_name = "吞吐量测�?(" + std::to_string(piece_size) + " bytes)";
            result.duration_ms = duration.count();
            result.pieces_processed = pieces_per_size;
            result.pieces_per_second = pieces_per_second;
            result.throughput_mbps = throughput_mbps;
            result.memory_usage_kb = get_memory_usage_kb();
            
            add_result(result);
            
            std::cout << "  分片大小: " << piece_size << " bytes" << std::endl;
            std::cout << "  吞吐�? " << std::fixed << std::setprecision(2) << throughput_mbps << " Mbps" << std::endl;
            std::cout << "  分片/�? " << std::fixed << std::setprecision(2) << pieces_per_second << std::endl;
            
            engine_->stop();
        }
        
        std::cout << "吞吐量测试完�? << std::endl << std::endl;
    }
    
    // 延迟测试
    void run_latency_test() {
        std::cout << "运行延迟测试..." << std::endl;
        
        const int num_measurements = 100;
        std::vector<double> latencies;
        
        engine_->start();
        
        for (int i = 0; i < num_measurements; ++i) {
            auto request_time = std::chrono::high_resolution_clock::now();
            
            // 发布分片
            PieceData piece;
            piece.id.streamId = "latency_test";
            piece.id.pieceIndex = i;
            piece.data = std::vector<uint8_t>(1024, i % 256);
            
            engine_->publish_piece(piece);
            
            // 等待接收
            auto start_wait = std::chrono::high_resolution_clock::now();
            while (total_pieces_processed_ <= i) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                
                // 超时保护
                auto now = std::chrono::high_resolution_clock::now();
                if ((now - start_wait) > std::chrono::milliseconds(1000)) {
                    break;
                }
            }
            
            auto receive_time = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(receive_time - request_time);
            
            latencies.push_back(latency.count() / 1000.0); // 转换为毫�?
        }
        
        engine_->stop();
        
        // 计算延迟统计
        double avg_latency = 0;
        double min_latency = std::numeric_limits<double>::max();
        double max_latency = 0;
        
        for (double latency : latencies) {
            avg_latency += latency;
            min_latency = std::min(min_latency, latency);
            max_latency = std::max(max_latency, latency);
        }
        avg_latency /= latencies.size();
        
        PerformanceResult result;
        result.test_name = "延迟测试";
        result.duration_ms = 0;
        result.pieces_processed = num_measurements;
        result.pieces_per_second = 0;
        result.throughput_mbps = 0;
        result.memory_usage_kb = get_memory_usage_kb();
        result.additional_metrics["avg_latency_ms"] = avg_latency;
        result.additional_metrics["min_latency_ms"] = min_latency;
        result.additional_metrics["max_latency_ms"] = max_latency;
        
        add_result(result);
        
        std::cout << "  平均延迟: " << std::fixed << std::setprecision(2) << avg_latency << " ms" << std::endl;
        std::cout << "  最小延�? " << std::fixed << std::setprecision(2) << min_latency << " ms" << std::endl;
        std::cout << "  最大延�? " << std::fixed << std::setprecision(2) << max_latency << " ms" << std::endl;
        
        std::cout << "延迟测试完成" << std::endl << std::endl;
    }
    
    // 并发测试
    void run_concurrent_test() {
        std::cout << "运行并发测试..." << std::endl;
        
        const std::vector<int> thread_counts = {1, 2, 4, 8, 16};
        const int pieces_per_thread = 100;
        
        for (int thread_count : thread_counts) {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            engine_->start();
            
            std::vector<std::thread> threads;
            std::atomic<int> pieces_published{0};
            
            // 启动多个线程并发发布分片
            for (int t = 0; t < thread_count; ++t) {
                threads.emplace_back([this, t, pieces_per_thread, &pieces_published]() {
                    for (int i = 0; i < pieces_per_thread; ++i) {
                        PieceData piece;
                        piece.id.streamId = "concurrent_test_" + std::to_string(t);
                        piece.id.pieceIndex = i;
                        piece.data = std::vector<uint8_t>(1024, t * 100 + i);
                        
                        engine_->publish_piece(piece);
                        pieces_published++;
                        
                        std::this_thread::sleep_for(std::chrono::microseconds(100));
                    }
                });
            }
            
            // 等待所有线程完�?
            for (auto& thread : threads) {
                thread.join();
            }
            
            // 等待引擎处理
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            // 计算指标
            double pieces_per_second = pieces_published / (duration.count() / 1000.0);
            double throughput_mbps = (pieces_published * 1024 * 8.0) / (duration.count() / 1000.0) / 1000000.0;
            
            PerformanceResult result;
            result.test_name = "并发测试 (" + std::to_string(thread_count) + " 线程)";
            result.duration_ms = duration.count();
            result.pieces_processed = pieces_published;
            result.pieces_per_second = pieces_per_second;
            result.throughput_mbps = throughput_mbps;
            result.memory_usage_kb = get_memory_usage_kb();
            result.additional_metrics["thread_count"] = thread_count;
            
            add_result(result);
            
            std::cout << "  线程�? " << thread_count << std::endl;
            std::cout << "  吞吐�? " << std::fixed << std::setprecision(2) << throughput_mbps << " Mbps" << std::endl;
            std::cout << "  分片/�? " << std::fixed << std::setprecision(2) << pieces_per_second << std::endl;
            
            engine_->stop();
        }
        
        std::cout << "并发测试完成" << std::endl << std::endl;
    }
    
    // 内存测试
    void run_memory_test() {
        std::cout << "运行内存测试..." << std::endl;
        
        const int num_iterations = 5;
        const int pieces_per_iteration = 1000;
        
        std::vector<uint64_t> memory_usage;
        
        for (int iter = 0; iter < num_iterations; ++iter) {
            engine_->start();
            
            // 发布分片
            for (int i = 0; i < pieces_per_iteration; ++i) {
                PieceData piece;
                piece.id.streamId = "memory_test";
                piece.id.pieceIndex = iter * pieces_per_iteration + i;
                piece.data = std::vector<uint8_t>(1024, i % 256);
                
                engine_->publish_piece(piece);
            }
            
            // 等待处理
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            memory_usage.push_back(get_memory_usage_kb());
            
            engine_->stop();
        }
        
        // 计算内存统计
        double avg_memory = 0;
        for (uint64_t mem : memory_usage) {
            avg_memory += mem;
        }
        avg_memory /= memory_usage.size();
        
        PerformanceResult result;
        result.test_name = "内存测试";
        result.duration_ms = 0;
        result.pieces_processed = num_iterations * pieces_per_iteration;
        result.pieces_per_second = 0;
        result.throughput_mbps = 0;
        result.memory_usage_kb = static_cast<uint64_t>(avg_memory);
        result.additional_metrics["avg_memory_kb"] = avg_memory;
        
        add_result(result);
        
        std::cout << "  平均内存使用: " << std::fixed << std::setprecision(2) << avg_memory << " KB" << std::endl;
        
        std::cout << "内存测试完成" << std::endl << std::endl;
    }
    
    // 压力测试
    void run_stress_test() {
        std::cout << "运行压力测试..." << std::endl;
        
        const int stress_duration_seconds = 30;
        const int pieces_per_second = 1000;
        
        engine_->start();
        
        auto start_time = std::chrono::high_resolution_clock::now();
        auto end_time = start_time + std::chrono::seconds(stress_duration_seconds);
        
        int piece_counter = 0;
        
        while (std::chrono::high_resolution_clock::now() < end_time) {
            // 发布分片
            for (int i = 0; i < pieces_per_second / 10; ++i) { // �?00ms发布100个分�?
                PieceData piece;
                piece.id.streamId = "stress_test";
                piece.id.pieceIndex = piece_counter++;
                piece.data = std::vector<uint8_t>(1024, piece_counter % 256);
                
                engine_->publish_piece(piece);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        auto actual_end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(actual_end_time - start_time);
        
        // 等待处理完成
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        
        // 计算指标
        double actual_pieces_per_second = piece_counter / (duration.count() / 1000.0);
        double throughput_mbps = (piece_counter * 1024 * 8.0) / (duration.count() / 1000.0) / 1000000.0;
        
        PerformanceResult result;
        result.test_name = "压力测试 (" + std::to_string(stress_duration_seconds) + " �?";
        result.duration_ms = duration.count();
        result.pieces_processed = piece_counter;
        result.pieces_per_second = actual_pieces_per_second;
        result.throughput_mbps = throughput_mbps;
        result.memory_usage_kb = get_memory_usage_kb();
        result.additional_metrics["target_pieces_per_second"] = pieces_per_second;
        result.additional_metrics["actual_pieces_per_second"] = actual_pieces_per_second;
        
        add_result(result);
        
        std::cout << "  目标分片/�? " << pieces_per_second << std::endl;
        std::cout << "  实际分片/�? " << std::fixed << std::setprecision(2) << actual_pieces_per_second << std::endl;
        std::cout << "  吞吐�? " << std::fixed << std::setprecision(2) << throughput_mbps << " Mbps" << std::endl;
        
        engine_->stop();
        
        std::cout << "压力测试完成" << std::endl << std::endl;
    }
    
    // 可扩展性测�?
    void run_scalability_test() {
        std::cout << "运行可扩展性测�?.." << std::endl;
        
        const std::vector<int> peer_counts = {10, 25, 50, 100, 200};
        const int pieces_per_peer = 50;
        
        for (int peer_count : peer_counts) {
            // 调整配置
            config_.maxPeers = peer_count;
            
            // 重新创建引擎
            engine_.reset();
            engine_ = std::make_unique<PeerEngine>(config_);
            engine_->set_piece_received_callback(
                [this](const PieceData& piece) {
                    on_piece_received(piece);
                }
            );
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            engine_->start();
            
            // 模拟多个对等节点
            for (int p = 0; p < peer_count; ++p) {
                for (int i = 0; i < pieces_per_peer; ++i) {
                    PieceData piece;
                    piece.id.streamId = "scalability_test_" + std::to_string(p);
                    piece.id.pieceIndex = i;
                    piece.data = std::vector<uint8_t>(1024, p * 100 + i);
                    
                    engine_->publish_piece(piece);
                }
            }
            
            // 等待处理
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            // 计算指标
            int total_pieces = peer_count * pieces_per_peer;
            double pieces_per_second = total_pieces / (duration.count() / 1000.0);
            double throughput_mbps = (total_pieces * 1024 * 8.0) / (duration.count() / 1000.0) / 1000000.0;
            
            PerformanceResult result;
            result.test_name = "可扩展性测�?(" + std::to_string(peer_count) + " 对等节点)";
            result.duration_ms = duration.count();
            result.pieces_processed = total_pieces;
            result.pieces_per_second = pieces_per_second;
            result.throughput_mbps = throughput_mbps;
            result.memory_usage_kb = get_memory_usage_kb();
            result.additional_metrics["peer_count"] = peer_count;
            result.additional_metrics["pieces_per_peer"] = pieces_per_peer;
            
            add_result(result);
            
            std::cout << "  对等节点�? " << peer_count << std::endl;
            std::cout << "  吞吐�? " << std::fixed << std::setprecision(2) << throughput_mbps << " Mbps" << std::endl;
            std::cout << "  分片/�? " << std::fixed << std::setprecision(2) << pieces_per_second << std::endl;
            
            engine_->stop();
        }
        
        std::cout << "可扩展性测试完�? << std::endl << std::endl;
    }
    
    // 打印所有测试结�?
    void print_results() {
        std::cout << "\n性能测试结果汇�? << std::endl;
        std::cout << "==================" << std::endl;
        
        std::cout << std::left << std::setw(40) << "测试名称" 
                  << std::setw(15) << "分片/�? 
                  << std::setw(15) << "吞吐�?Mbps)" 
                  << std::setw(15) << "内存(KB)" 
                  << std::setw(15) << "时间(ms)" << std::endl;
        
        std::cout << std::string(100, '-') << std::endl;
        
        for (const auto& result : results_) {
            std::cout << std::left << std::setw(40) << result.test_name
                      << std::setw(15) << std::fixed << std::setprecision(2) << result.pieces_per_second
                      << std::setw(15) << std::fixed << std::setprecision(2) << result.throughput_mbps
                      << std::setw(15) << result.memory_usage_kb
                      << std::setw(15) << result.duration_ms << std::endl;
        }
        
        std::cout << std::endl;
        
        // 计算总体统计
        double total_throughput = 0;
        double total_pieces_per_second = 0;
        int valid_results = 0;
        
        for (const auto& result : results_) {
            if (result.throughput_mbps > 0) {
                total_throughput += result.throughput_mbps;
                total_pieces_per_second += result.pieces_per_second;
                valid_results++;
            }
        }
        
        if (valid_results > 0) {
            std::cout << "总体统计:" << std::endl;
            std::cout << "- 平均吞吐�? " << std::fixed << std::setprecision(2) 
                      << (total_throughput / valid_results) << " Mbps" << std::endl;
            std::cout << "- 平均分片/�? " << std::fixed << std::setprecision(2) 
                      << (total_pieces_per_second / valid_results) << std::endl;
            std::cout << "- 总测试数: " << valid_results << std::endl;
        }
    }
};

// 主函�?
int main() {
    std::cout << "P2P Core 性能测试套件" << std::endl;
    std::cout << "=====================" << std::endl << std::endl;
    
    try {
        PerformanceTest test;
        test.run_all_tests();
    } catch (const std::exception& e) {
        std::cerr << "性能测试失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
