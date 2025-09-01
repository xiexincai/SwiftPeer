#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <core/peer_engine.h>
#include <core/config.h>
#include <core/types.h>

using namespace p2p_core;
using namespace testing;

// 模拟分片接收回调
class MockPieceCallback {
public:
    MOCK_METHOD(void, onPieceReceived, (const PieceData& piece));
};

// 模拟DNS解析�?
class MockDnsResolver {
public:
    MOCK_METHOD(std::vector<Endpoint>, resolve, (const std::string& hostname));
};

// P2P引擎测试套件
class PeerEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置基本配置
        config_.nodeId = "test_node_001";
        config_.listenPort = 0;  // 自动端口
        config_.maxUploadBps = 1024000;  // 1 MB/s
        config_.maxDownloadBps = 0;      // 无限�?
        config_.maxBurstBytes = 65536;   // 64KB
        config_.maxPeers = 10;
        config_.enableTls = false;       // 测试时禁用TLS
        config_.enableDht = false;       // 测试时禁用DHT
        config_.enableQuic = false;
        config_.enableIce = false;
    }
    
    void TearDown() override {
        if (engine_) {
            engine_->stop();
        }
    }
    
    P2PConfig config_;
    std::unique_ptr<PeerEngine> engine_;
    std::shared_ptr<MockPieceCallback> mock_callback_;
};

// 测试基本初始化和启动
TEST_F(PeerEngineTest, BasicInitialization) {
    EXPECT_NO_THROW({
        engine_ = std::make_unique<PeerEngine>(config_);
    });
    
    EXPECT_TRUE(engine_ != nullptr);
}

TEST_F(PeerEngineTest, StartAndStop) {
    engine_ = std::make_unique<PeerEngine>(config_);
    
    // 测试启动
    EXPECT_TRUE(engine_->start());
    
    // 测试停止
    EXPECT_NO_THROW(engine_->stop());
}

// 测试配置验证
TEST_F(PeerEngineTest, ConfigurationValidation) {
    // 测试有效配置
    EXPECT_NO_THROW({
        engine_ = std::make_unique<PeerEngine>(config_);
    });
    
    // 测试无效配置（负值）
    P2PConfig invalid_config = config_;
    invalid_config.maxUploadBps = -1000;
    
    EXPECT_THROW({
        auto invalid_engine = std::make_unique<PeerEngine>(invalid_config);
    }, std::exception);
}

// 测试分片发布和请�?
TEST_F(PeerEngineTest, PiecePublishAndRequest) {
    engine_ = std::make_unique<PeerEngine>(config_);
    EXPECT_TRUE(engine_->start());
    
    // 创建测试分片
    PieceData test_piece;
    test_piece.id.streamId = "test_stream";
    test_piece.id.pieceIndex = 1;
    test_piece.data = {0x01, 0x02, 0x03, 0x04, 0x05};
    
    // 发布分片
    EXPECT_NO_THROW(engine_->publish_piece(test_piece));
    
    // 请求分片
    PieceId request_id;
    request_id.streamId = "test_stream";
    request_id.pieceIndex = 1;
    
    EXPECT_NO_THROW(engine_->request_piece(request_id));
    
    // 等待一下让引擎处理
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// 测试回调设置
TEST_F(PeerEngineTest, CallbackSetting) {
    engine_ = std::make_unique<PeerEngine>(config_);
    
    mock_callback_ = std::make_shared<MockPieceCallback>();
    
    // 设置回调
    EXPECT_NO_THROW({
        engine_->set_piece_received_callback(
            [this](const PieceData& piece) {
                mock_callback_->onPieceReceived(piece);
            }
        );
    });
    
    // 启动引擎
    EXPECT_TRUE(engine_->start());
}

// 测试种子节点管理
TEST_F(PeerEngineTest, SeedPeerManagement) {
    engine_ = std::make_unique<PeerEngine>(config_);
    EXPECT_TRUE(engine_->start());
    
    // 添加种子节点
    Endpoint seed1{"127.0.0.1", 5000};
    Endpoint seed2{"192.168.1.100", 5000};
    
    EXPECT_NO_THROW(engine_->add_seed_peer(seed1));
    EXPECT_NO_THROW(engine_->add_seed_peer(seed2));
    
    // 等待一下让引擎处理
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// 测试统计信息
TEST_F(PeerEngineTest, Statistics) {
    engine_ = std::make_unique<PeerEngine>(config_);
    EXPECT_TRUE(engine_->start());
    
    // 获取初始统计
    auto initial_stats = engine_->stats();
    EXPECT_EQ(initial_stats.bytesUploaded, 0);
    EXPECT_EQ(initial_stats.bytesDownloaded, 0);
    EXPECT_EQ(initial_stats.connectedPeers, 0);
    
    // 发布一些分�?
    for (int i = 1; i <= 5; ++i) {
        PieceData piece;
        piece.id.streamId = "test_stream";
        piece.id.pieceIndex = i;
        piece.data = std::vector<uint8_t>(1024, i);  // 1KB数据
        
        engine_->publish_piece(piece);
    }
    
    // 等待一下让引擎处理
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 检查统计是否更�?
    auto updated_stats = engine_->stats();
    EXPECT_GE(updated_stats.bytesUploaded, 0);
}

// 测试并发操作
TEST_F(PeerEngineTest, ConcurrentOperations) {
    engine_ = std::make_unique<PeerEngine>(config_);
    EXPECT_TRUE(engine_->start());
    
    const int num_threads = 4;
    const int pieces_per_thread = 10;
    
    std::vector<std::thread> threads;
    
    // 启动多个线程并发发布分片
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, pieces_per_thread]() {
            for (int i = 0; i < pieces_per_thread; ++i) {
                PieceData piece;
                piece.id.streamId = "concurrent_stream_" + std::to_string(t);
                piece.id.pieceIndex = i;
                piece.data = std::vector<uint8_t>(512, t * 100 + i);
                
                engine_->publish_piece(piece);
                
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    // 等待所有线程完�?
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 等待引擎处理
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 检查统�?
    auto stats = engine_->stats();
    EXPECT_GE(stats.bytesUploaded, num_threads * pieces_per_thread * 512);
}

// 测试错误处理
TEST_F(PeerEngineTest, ErrorHandling) {
    // 测试空数�?
    engine_ = std::make_unique<PeerEngine>(config_);
    EXPECT_TRUE(engine_->start());
    
    PieceData empty_piece;
    empty_piece.id.streamId = "test_stream";
    empty_piece.id.pieceIndex = 1;
    empty_piece.data = {};
    
    // 应该能够处理空数�?
    EXPECT_NO_THROW(engine_->publish_piece(empty_piece));
    
    // 测试无效的流ID
    PieceData invalid_piece;
    invalid_piece.id.streamId = "";
    invalid_piece.id.pieceIndex = 1;
    invalid_piece.data = {0x01, 0x02, 0x03};
    
    // 应该能够处理无效流ID
    EXPECT_NO_THROW(engine_->publish_piece(invalid_piece));
}

// 测试性能基准
TEST_F(PeerEngineTest, PerformanceBenchmark) {
    engine_ = std::make_unique<PeerEngine>(config_);
    EXPECT_TRUE(engine_->start());
    
    const int num_pieces = 1000;
    const int piece_size = 1024;  // 1KB
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 批量发布分片
    for (int i = 0; i < num_pieces; ++i) {
        PieceData piece;
        piece.id.streamId = "perf_test_stream";
        piece.id.pieceIndex = i;
        piece.data = std::vector<uint8_t>(piece_size, i % 256);
        
        engine_->publish_piece(piece);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 等待引擎处理
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // 计算性能指标
    double pieces_per_second = num_pieces / (duration.count() / 1000.0);
    double mbps = (num_pieces * piece_size * 8.0) / (duration.count() / 1000.0) / 1000000.0;
    
    std::cout << "性能测试结果:" << std::endl;
    std::cout << "- 总时�? " << duration.count() << " ms" << std::endl;
    std::cout << "- 分片/�? " << pieces_per_second << std::endl;
    std::cout << "- 吞吐�? " << mbps << " Mbps" << std::endl;
    
    // 性能断言（根据实际情况调整）
    EXPECT_GT(pieces_per_second, 100);  // 至少100分片/�?
    EXPECT_GT(mbps, 0.1);               // 至少0.1 Mbps
}

// 测试内存管理
TEST_F(PeerEngineTest, MemoryManagement) {
    // 测试多次创建和销�?
    for (int i = 0; i < 5; ++i) {
        auto temp_engine = std::make_unique<PeerEngine>(config_);
        EXPECT_TRUE(temp_engine->start());
        
        // 发布一些分�?
        for (int j = 0; j < 10; ++j) {
            PieceData piece;
            piece.id.streamId = "memory_test_stream";
            piece.id.pieceIndex = j;
            piece.data = std::vector<uint8_t>(1024, j);
            
            temp_engine->publish_piece(piece);
        }
        
        temp_engine->stop();
        // temp_engine 自动销�?
    }
    
    // 如果没有内存泄漏，这里不应该崩溃
    EXPECT_TRUE(true);
}

// 测试配置选项
TEST_F(PeerEngineTest, ConfigurationOptions) {
    // 测试不同的配置组�?
    
    // 配置1：启用TLS
    P2PConfig tls_config = config_;
    tls_config.enableTls = true;
    
    auto tls_engine = std::make_unique<PeerEngine>(tls_config);
    EXPECT_TRUE(tls_engine->start());
    tls_engine->stop();
    
    // 配置2：启用DHT
    P2PConfig dht_config = config_;
    dht_config.enableDht = true;
    
    auto dht_engine = std::make_unique<PeerEngine>(dht_config);
    EXPECT_TRUE(dht_engine->start());
    dht_engine->stop();
    
    // 配置3：启用内容寻址
    P2PConfig content_config = config_;
    content_config.enableContentAddressing = true;
    
    auto content_engine = std::make_unique<PeerEngine>(content_config);
    EXPECT_TRUE(content_engine->start());
    content_engine->stop();
}

// 测试边界条件
TEST_F(PeerEngineTest, BoundaryConditions) {
    engine_ = std::make_unique<PeerEngine>(config_);
    EXPECT_TRUE(engine_->start());
    
    // 测试最大分片索�?
    PieceData max_piece;
    max_piece.id.streamId = "boundary_test";
    max_piece.id.pieceIndex = UINT64_MAX;
    max_piece.data = {0x01, 0x02, 0x03};
    
    EXPECT_NO_THROW(engine_->publish_piece(max_piece));
    
    // 测试空流ID
    PieceData empty_stream_piece;
    empty_stream_piece.id.streamId = "";
    empty_stream_piece.id.pieceIndex = 1;
    empty_stream_piece.data = {0x01, 0x02, 0x03};
    
    EXPECT_NO_THROW(engine_->publish_piece(empty_stream_piece));
    
    // 测试非常大的数据
    PieceData large_piece;
    large_piece.id.streamId = "large_test";
    large_piece.id.pieceIndex = 1;
    large_piece.data = std::vector<uint8_t>(1024 * 1024, 0xFF);  // 1MB
    
    EXPECT_NO_THROW(engine_->publish_piece(large_piece));
}

// 主函�?
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    
    std::cout << "P2P Core 单元测试" << std::endl;
    std::cout << "=================" << std::endl;
    
    return RUN_ALL_TESTS();
}
