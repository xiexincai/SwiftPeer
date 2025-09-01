#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <p2p/api/c_api.h>

// 分片接收回调函数
static void on_piece_received(const p2p_piece_t* piece, void* user) {
    printf("收到分片: stream_id=%s, piece_index=%lu, size=%zu\n", 
           piece->id.stream_id, piece->id.piece_index, piece->size);
    
    // 这里可以处理接收到的分片数据
    // 例如：保存到文件、传递给播放器等
    
    // 打印前几个字节作为示例
    printf("数据预览: ");
    for (size_t i = 0; i < piece->size && i < 16; i++) {
        printf("%02x ", piece->data[i]);
    }
    printf("\n");
}

// 错误处理函数
static void check_error(int result, const char* operation) {
    if (result != 0) {
        fprintf(stderr, "错误: %s 失败，错误码: %d\n", operation, result);
        exit(1);
    }
}

int main() {
    printf("P2P Core 基本 C API 示例\n");
    printf("========================\n\n");
    
    // 1. 创建配置
    p2p_config_t config = {0};
    config.listen_port = 0;           // 自动选择端口
    config.max_upload_bps = 1024000;  // 1 MB/s 上传限制
    config.max_download_bps = 0;      // 不限制下载
    config.max_burst_bytes = 65536;   // 64KB 突发
    config.max_peers = 20;            // 最大20个对等节点
    config.node_id = "example_node_001";
    
    printf("配置信息:\n");
    printf("- 监听端口: 自动\n");
    printf("- 上传限制: %.1f KB/s\n", config.max_upload_bps / 1024.0);
    printf("- 下载限制: 无限制\n");
    printf("- 突发大小: %lu bytes\n", (unsigned long)config.max_burst_bytes);
    printf("- 最大对等节点: %u\n", config.max_peers);
    printf("- 节点ID: %s\n\n", config.node_id);
    
    // 2. 创建P2P实例
    printf("创建P2P实例...\n");
    p2p_handle_t handle = p2p_create(&config);
    if (!handle) {
        fprintf(stderr, "错误: 无法创建P2P实例\n");
        return 1;
    }
    printf("P2P实例创建成功\n\n");
    
    // 3. 设置分片接收回调
    printf("设置分片接收回调...\n");
    p2p_set_piece_received_callback(handle, on_piece_received, NULL);
    
    // 4. 启动P2P引擎
    printf("启动P2P引擎...\n");
    int result = p2p_start(handle);
    check_error(result, "启动P2P引擎");
    printf("P2P引擎启动成功\n\n");
    
    // 5. 添加种子节点
    printf("添加种子节点...\n");
    p2p_endpoint_t seed1 = {"127.0.0.1", 5000};
    p2p_endpoint_t seed2 = {"192.168.1.100", 5000};
    
    p2p_add_seed(handle, &seed1);
    p2p_add_seed(handle, &seed2);
    printf("种子节点添加完成:\n");
    printf("- %s:%u\n", seed1.host, seed1.port);
    printf("- %s:%u\n\n", seed2.host, seed2.port);
    
    // 6. 发布一些测试分片
    printf("发布测试分片...\n");
    const unsigned char test_data1[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    const unsigned char test_data2[] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    
    p2p_piece_t piece1 = {{"test_stream", 1}, test_data1, sizeof(test_data1)};
    p2p_piece_t piece2 = {{"test_stream", 2}, test_data2, sizeof(test_data2)};
    
    p2p_publish_piece(handle, &piece1);
    p2p_publish_piece(handle, &piece2);
    printf("测试分片发布完成:\n");
    printf("- stream: %s, index: %lu, size: %zu\n", 
           piece1.id.stream_id, piece1.id.piece_index, piece1.size);
    printf("- stream: %s, index: %lu, size: %zu\n\n", 
           piece2.id.stream_id, piece2.id.piece_index, piece2.size);
    
    // 7. 请求一些分片
    printf("请求分片...\n");
    p2p_piece_id_t request1 = {"test_stream", 3};
    p2p_piece_id_t request2 = {"test_stream", 4};
    
    p2p_request_piece(handle, &request1);
    p2p_request_piece(handle, &request2);
    printf("分片请求已发送:\n");
    printf("- stream: %s, index: %lu\n", request1.stream_id, request1.piece_index);
    printf("- stream: %s, index: %lu\n\n", request2.stream_id, request2.piece_index);
    
    // 8. 运行一段时间，收集统计信息
    printf("运行中... (等待10秒)\n");
    for (int i = 0; i < 10; i++) {
    #ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
        printf(".");
        fflush(stdout);
        
        // 每2秒显示一次统计信息
        if ((i + 1) % 2 == 0) {
            p2p_stats_t stats = p2p_stats(handle);
            printf("\n当前统计:\n");
            printf("- 上传字节: %lu\n", (unsigned long)stats.bytes_uploaded);
            printf("- 下载字节: %lu\n", (unsigned long)stats.bytes_downloaded);
            printf("- 连接的对等节点: %u\n", stats.connected_peers);
        }
    }
    printf("\n\n");
    
    // 9. 显示最终统计信息
    printf("最终统计信息:\n");
    p2p_stats_t final_stats = p2p_stats(handle);
    printf("- 总上传字节: %lu (%.2f KB)\n", 
           (unsigned long)final_stats.bytes_uploaded,
           final_stats.bytes_uploaded / 1024.0);
    printf("- 总下载字节: %lu (%.2f KB)\n", 
           (unsigned long)final_stats.bytes_downloaded,
           final_stats.bytes_downloaded / 1024.0);
    printf("- 连接的对等节点: %u\n", final_stats.connected_peers);
    
    // 10. 清理资源
    printf("\n清理资源...\n");
    p2p_stop(handle);
    p2p_destroy(handle);
    printf("P2P实例已销毁\n");
    
    printf("\n示例运行完成！\n");
    return 0;
}
