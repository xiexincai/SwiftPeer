#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <p2p/core/peer_engine.h>
#include <p2p/core/config.h>
#include <p2p/core/types.h>

using namespace p2p_core;

class P2PExample {
private:
    std::unique_ptr<PeerEngine> engine_;
    std::atomic<bool> running_{false};
    std::thread stats_thread_;
    
    // Statistics
    std::atomic<uint64_t> total_pieces_received_{0};
    std::atomic<uint64_t> total_pieces_published_{0};

public:
    P2PExample() = default;
    ~P2PExample() {
        stop();
    }

    bool initialize() {
        std::cout << "Initializing P2P engine..." << std::endl;
        
        // Create configuration
        P2PConfig config;
        config.nodeId = "cpp_example_node_001";
        config.listenPort = 0;  // Auto-select port
        config.maxUploadBps = 2048000;  // 2 MB/s
        config.maxDownloadBps = 0;      // Unlimited
        config.maxBurstBytes = 131072;  // 128KB
        config.maxPeers = 30;
        
        // Add seed peers
        config.seedPeers.push_back({"127.0.0.1", 5000});
        config.seedPeers.push_back({"192.168.1.100", 5000});
        config.seedPeers.push_back({"10.0.0.50", 5000});
        
        // Configure tracker and STUN servers
        config.trackerUrls.push_back("http://tracker.example.com:8080/announce");
        config.stunServers.push_back("stun.l.google.com:19302");
        config.stunServers.push_back("stun1.l.google.com:19302");
        
        // Enable TLS
        config.enableTls = true;
        config.enableQuic = false;
        
        // Enable DHT
        config.enableDht = true;
        config.dhtPort = 0;  // Auto-select port
        
        // Enable content addressing (optional)
        config.enableContentAddressing = false;
        config.enablePublisherSignature = false;
        
        // Enable ICE (optional)
        config.enableIce = false;
        
        print_config(config);
        
        try {
            engine_ = std::make_unique<PeerEngine>(config);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create PeerEngine: " << e.what() << std::endl;
            return false;
        }
        
        // Set piece received callback
        engine_->set_piece_received_callback(
            [this](const PieceData& piece) {
                on_piece_received(piece);
            }
        );
        
        return true;
    }
    
    bool start() {
        if (!engine_) {
            std::cerr << "Engine not initialized" << std::endl;
            return false;
        }
        
        std::cout << "Starting P2P engine..." << std::endl;
        
        if (!engine_->start()) {
            std::cerr << "Failed to start P2P engine" << std::endl;
            return false;
        }
        
        running_ = true;
        std::cout << "P2P engine started successfully" << std::endl;
        
        // Start stats thread
        stats_thread_ = std::thread([this] { stats_loop(); });
        
        return true;
    }
    
    void stop() {
        if (!running_) return;
        
        running_ = false;
        
        if (engine_) {
            engine_->stop();
        }
        
        if (stats_thread_.joinable()) {
            stats_thread_.join();
        }
        
        std::cout << "P2P engine stopped" << std::endl;
    }
    
    void publish_test_pieces() {
        if (!engine_ || !running_) return;
        
        std::cout << "Publishing test pieces..." << std::endl;
        
        // Create some test data
        std::vector<std::vector<uint8_t>> test_data = {
            {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
            {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12},
            {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A}
        };
        
        for (size_t i = 0; i < test_data.size(); ++i) {
            PieceData piece;
            piece.id.streamId = "test_stream_cpp";
            piece.id.pieceIndex = i + 1;
            piece.data = test_data[i];
            
            engine_->publish_piece(piece);
            total_pieces_published_++;
            
            std::cout << "Published piece: stream=" << piece.id.streamId 
                      << ", index=" << piece.id.pieceIndex 
                      << ", size=" << piece.data.size() << " bytes" << std::endl;
        }
        
        std::cout << "Test pieces published" << std::endl;
    }
    
    void request_test_pieces() {
        if (!engine_ || !running_) return;
        
        std::cout << "Requesting test pieces..." << std::endl;
        
        // Request some pieces
        for (uint64_t i = 5; i <= 10; ++i) {
            PieceId id;
            id.streamId = "test_stream_cpp";
            id.pieceIndex = i;
            
            engine_->request_piece(id);
            std::cout << "Requested piece: stream=" << id.streamId 
                      << ", index=" << id.pieceIndex << std::endl;
        }
        
        std::cout << "Piece requests sent" << std::endl;
    }
    
    void add_seed_peers() {
        if (!engine_ || !running_) return;
        
        std::cout << "Adding seed peers..." << std::endl;
        
        std::vector<Endpoint> additional_seeds = {
            {"172.16.0.100", 5000},
            {"192.168.0.200", 5000},
            {"10.0.1.150", 5000}
        };
        
        for (const auto& seed : additional_seeds) {
            engine_->add_seed_peer(seed);
            std::cout << "Added seed peer: " << seed.host << ":" << seed.port << std::endl;
        }
        
        std::cout << "Seed peers added" << std::endl;
    }
    
    void run_demo() {
        std::cout << "Running P2P demo..." << std::endl;
        
        // Publish some pieces
        publish_test_pieces();
        
        // Wait a bit
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Request some pieces
        request_test_pieces();
        
        // Add more seed peers
        std::this_thread::sleep_for(std::chrono::seconds(2));
        add_seed_peers();
        
        // Run for a while
        std::cout << "Demo running... (waiting 30 seconds)" << std::endl;
        for (int i = 0; i < 30; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if ((i + 1) % 5 == 0) {
                std::cout << "Running time: " << (i + 1) << " seconds" << std::endl;
            }
        }
        
        std::cout << "Demo completed" << std::endl;
    }

private:
    void print_config(const P2PConfig& config) {
        std::cout << "Configuration:" << std::endl;
        std::cout << "- Node ID: " << config.nodeId << std::endl;
        std::cout << "- Listen Port: " << (config.listenPort == 0 ? "auto" : std::to_string(config.listenPort)) << std::endl;
        std::cout << "- Max Upload: " << (config.maxUploadBps / 1024.0) << " KB/s" << std::endl;
        std::cout << "- Max Download: " << (config.maxDownloadBps == 0 ? "unlimited" : std::to_string(config.maxDownloadBps / 1024.0) + " KB/s") << std::endl;
        std::cout << "- Max Burst: " << config.maxBurstBytes << " bytes" << std::endl;
        std::cout << "- Max Peers: " << config.maxPeers << std::endl;
        std::cout << "- TLS Enabled: " << (config.enableTls ? "yes" : "no") << std::endl;
        std::cout << "- QUIC Enabled: " << (config.enableQuic ? "yes" : "no") << std::endl;
        std::cout << "- DHT Enabled: " << (config.enableDht ? "yes" : "no") << std::endl;
        std::cout << "- ICE Enabled: " << (config.enableIce ? "yes" : "no") << std::endl;
        std::cout << std::endl;
    }
    
    void on_piece_received(const PieceData& piece) {
        total_pieces_received_++;
        
        std::cout << "Received piece: stream=" << piece.id.streamId 
                  << ", index=" << piece.id.pieceIndex 
                  << ", size=" << piece.data.size() << " bytes" << std::endl;
        
        // Print first few bytes as preview
        std::cout << "Data preview: ";
        for (size_t i = 0; i < piece.data.size() && i < 16; ++i) {
            printf("%02x ", piece.data[i]);
        }
        std::cout << std::endl;
    }
    
    void stats_loop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            
            if (engine_) {
                auto stats = engine_->stats();
                std::cout << "Stats: " << stats.connectedPeers << " peers, "
                          << "uploaded: " << (stats.bytesUploaded / 1024) << " KB, "
                          << "downloaded: " << (stats.bytesDownloaded / 1024) << " KB, "
                          << "pieces received: " << total_pieces_received_.load() << ", "
                          << "pieces published: " << total_pieces_published_.load() << std::endl;
            }
        }
    }
};

int main() {
    std::cout << "P2P Core C++ API Example" << std::endl;
    std::cout << "=========================" << std::endl << std::endl;
    
    P2PExample example;
    
    if (!example.initialize()) {
        std::cerr << "Failed to initialize example" << std::endl;
        return 1;
    }
    
    if (!example.start()) {
        std::cerr << "Failed to start example" << std::endl;
        return 1;
    }
    
    // Run the demo
    example.run_demo();
    
    // Cleanup
    example.stop();
    
    std::cout << "Example completed successfully" << std::endl;
    return 0;
}

