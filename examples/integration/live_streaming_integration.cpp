#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <string>
#include <memory>

#include "p2p/core/peer_engine.h"
#include "p2p/core/types.h"
#include "p2p/transport/transport.h"

using namespace p2p_core;

// 媒体段结构
struct MediaSegment {
    std::string streamId;
    uint64_t segmentIndex;
    std::string url;
    bool isP2P;
    std::chrono::steady_clock::time_point requestTime;
    std::chrono::steady_clock::time_point receiveTime;
};

// 段请求标识
struct SegmentRequest {
    std::string streamId;
    uint64_t segmentIndex;
};

// 统计信息
struct AcceleratorStats {
    uint64_t totalSegments = 0;
    uint64_t p2pSegments = 0;
    uint64_t cdnSegments = 0;
    uint64_t totalBytes = 0;
    double avgLatencyMs = 0.0;
};

// 直播流加速器
class LiveStreamingAccelerator {
public:
    LiveStreamingAccelerator() : running_(false) {}
    
    ~LiveStreamingAccelerator() {
        stop();
    }
    
    bool initialize(const P2PConfig& config) {
        try {
            p2p_engine_ = std::make_unique<PeerEngine>(config);
            
            // 设置段接收回调
            p2p_engine_->set_piece_received_callback(
                [this](const PieceData& piece) {
                    handle_p2p_segment(piece);
                }
            );
            
            // 设置自定义DNS解析器（用于CDN优化）
            p2p_engine_->set_custom_resolver(
                [this](const std::string& hostname) -> std::string {
                    auto endpoints = optimize_cdn_resolution(hostname);
                    if (!endpoints.empty()) {
                        return endpoints[0].host + ":" + std::to_string(endpoints[0].port);
                    }
                    return hostname; // 回退到原始主机名
                }
            );
            
            std::cout << "Live streaming accelerator initialized successfully" << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize accelerator: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool start() {
        if (!p2p_engine_) {
            std::cerr << "Accelerator not initialized" << std::endl;
            return false;
        }
        
        std::cout << "Starting live streaming accelerator..." << std::endl;
        
        if (!p2p_engine_->start()) {
            std::cerr << "Failed to start P2P engine" << std::endl;
            return false;
        }
        
        running_ = true;
        
        // Start P2P processing thread
        p2p_thread_ = std::thread([this]() {
            p2p_processing_loop();
        });
        
        // Start CDN fallback thread
        cdn_thread_ = std::thread([this]() {
            cdn_fallback_loop();
        });
        
        std::cout << "Live streaming accelerator started successfully" << std::endl;
        return true;
    }
    
    void stop() {
        if (!running_) return;
        
        std::cout << "Stopping live streaming accelerator..." << std::endl;
        running_ = false;
        
        segment_cv_.notify_all();
        
        if (p2p_thread_.joinable()) {
            p2p_thread_.join();
        }
        
        if (cdn_thread_.joinable()) {
            cdn_thread_.join();
        }
        
        if (p2p_engine_) {
            p2p_engine_->stop();
        }
        
        std::cout << "Live streaming accelerator stopped" << std::endl;
    }
    
    // Request media segment (racing mode: P2P + CDN)
    void requestSegment(const std::string& streamId, uint64_t segmentIndex, const std::string& cdnUrl) {
        if (!running_) return;
        
        std::cout << "Requesting segment: " << streamId << ":" << segmentIndex << std::endl;
        
        // Create segment record
        MediaSegment segment;
        segment.streamId = streamId;
        segment.segmentIndex = segmentIndex;
        segment.url = cdnUrl;
        segment.isP2P = false;
        segment.requestTime = std::chrono::steady_clock::now();
        
        // Add to pending queue
        {
            std::lock_guard<std::mutex> lock(segments_mutex_);
            pending_segments_[streamId][segmentIndex] = segment;
            segment_queue_.push({streamId, segmentIndex});
        }
        
        segment_cv_.notify_one();
        
        // Simultaneously initiate P2P request
        PieceId pieceId;
        pieceId.streamId = streamId;
        pieceId.pieceIndex = segmentIndex;
        p2p_engine_->request_piece(pieceId);
        
        stats_.totalSegments++;
        
        std::cout << "Segment request sent (P2P + CDN racing)" << std::endl;
    }
    
    // Publish media segment (when segment is obtained from CDN)
    void publishSegment(const std::string& streamId, uint64_t segmentIndex, const std::vector<uint8_t>& data) {
        if (!running_) return;
        
        std::cout << "Publishing segment: " << streamId << ":" << segmentIndex << std::endl;
        
        // Create piece data
        PieceData piece;
        piece.id.streamId = streamId;
        piece.id.pieceIndex = segmentIndex;
        piece.data = data;
        
        // Publish to P2P network
        p2p_engine_->publish_piece(piece);
        
        // Update statistics
        stats_.totalBytes += data.size();
        
        std::cout << "Segment published to P2P network" << std::endl;
    }
    
    // Get statistics
    AcceleratorStats getStats() const {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        return stats_;
    }
    
private:
    void handle_p2p_segment(const PieceData& piece) {
        std::cout << "Received P2P segment: " << piece.id.streamId << ":" << piece.id.pieceIndex << std::endl;
        
        // Check if we're still waiting for this segment
        std::lock_guard<std::mutex> lock(segments_mutex_);
        auto streamIt = pending_segments_.find(piece.id.streamId);
        if (streamIt != pending_segments_.end()) {
            auto segmentIt = streamIt->second.find(piece.id.pieceIndex);
            if (segmentIt != streamIt->second.end()) {
                // Mark as received via P2P
                segmentIt->second.isP2P = true;
                segmentIt->second.receiveTime = std::chrono::steady_clock::now();
                
                // Update statistics
                {
                    std::lock_guard<std::mutex> statsLock(stats_mutex_);
                    stats_.p2pSegments++;
                    
                    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
                        segmentIt->second.receiveTime - segmentIt->second.requestTime
                    ).count();
                    
                    // Update average latency
                    if (stats_.p2pSegments == 1) {
                        stats_.avgLatencyMs = latency;
                    } else {
                        stats_.avgLatencyMs = (stats_.avgLatencyMs * (stats_.p2pSegments - 1) + latency) / stats_.p2pSegments;
                    }
                }
                
                std::cout << "Segment completed via P2P" << std::endl;
            }
        }
    }
    
    void p2p_processing_loop() {
        while (running_) {
            // Process P2P-related tasks
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    void cdn_fallback_loop() {
        while (running_) {
            std::unique_lock<std::mutex> lock(segments_mutex_);
            segment_cv_.wait(lock, [this] { return !running_ || !segment_queue_.empty(); });
            
            if (!running_) break;
            
            while (!segment_queue_.empty()) {
                auto request = segment_queue_.front();
                segment_queue_.pop();
                lock.unlock();
                
                // Simulate CDN fallback processing
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                lock.lock();
            }
        }
    }
    
    std::vector<Endpoint> optimize_cdn_resolution(const std::string& hostname) {
        // Simple CDN optimization logic
        std::vector<Endpoint> endpoints;
        
        // Add some example CDN endpoints
        endpoints.push_back({"cdn1.example.com", 443});
        endpoints.push_back({"cdn2.example.com", 443});
        endpoints.push_back({"cdn3.example.com", 443});
        
        return endpoints;
    }
    
    std::unique_ptr<PeerEngine> p2p_engine_;
    std::thread p2p_thread_;
    std::thread cdn_thread_;
    std::atomic<bool> running_;
    
    std::mutex segments_mutex_;
    std::unordered_map<std::string, std::unordered_map<uint64_t, MediaSegment>> pending_segments_;
    std::queue<SegmentRequest> segment_queue_;
    std::condition_variable segment_cv_;
    
    mutable std::mutex stats_mutex_;
    AcceleratorStats stats_;
};

// Main function demonstrating the integration
int main() {
    std::cout << "Live Streaming Integration Example" << std::endl;
    std::cout << "=================================" << std::endl;
    
    try {
        // Create P2P configuration
        P2PConfig config;
        config.nodeId = "live-accelerator-001";
        config.listenPort = 0; // Auto-assign port
        config.maxUploadBps = 1024 * 1024; // 1 Mbps
        config.maxDownloadBps = 1024 * 1024; // 1 Mbps
        config.maxBurstBytes = 65536;
        config.maxPeers = 20;
        
        // Create and initialize accelerator
        LiveStreamingAccelerator accelerator;
        
        if (!accelerator.initialize(config)) {
            std::cerr << "Failed to initialize accelerator" << std::endl;
            return 1;
        }
        
        // Start the accelerator
        if (!accelerator.start()) {
            std::cerr << "Failed to start accelerator" << std::endl;
            return 1;
        }
        
        // Simulate some segment requests
        std::cout << "\nSimulating segment requests..." << std::endl;
        
        for (int i = 0; i < 5; ++i) {
            std::string streamId = "live-stream-001";
            uint64_t segmentIndex = i;
            std::string cdnUrl = "https://cdn.example.com/stream/" + std::to_string(i) + ".m4s";
            
            accelerator.requestSegment(streamId, segmentIndex, cdnUrl);
            
            // Simulate publishing some segments
            std::vector<uint8_t> segmentData(1024 * 1024, 0x42); // 1MB dummy data
            accelerator.publishSegment(streamId, segmentIndex, segmentData);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Wait a bit for processing
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Get and display statistics
        auto stats = accelerator.getStats();
        std::cout << "\nAccelerator Statistics:" << std::endl;
        std::cout << "Total segments: " << stats.totalSegments << std::endl;
        std::cout << "P2P segments: " << stats.p2pSegments << std::endl;
        std::cout << "CDN segments: " << stats.cdnSegments << std::endl;
        std::cout << "Total bytes: " << stats.totalBytes << std::endl;
        std::cout << "Average latency: " << stats.avgLatencyMs << " ms" << std::endl;
        
        // Stop the accelerator
        accelerator.stop();
        
        std::cout << "\nIntegration example completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
